#ifndef _PATCH_TCP_H_
#define _PATCH_TCP_H_

#include <map>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#define str_cstr_equals(x, y) (strcmp(x.c_str(), y) == 0)

namespace patch_tcp {
  template<typename Tk, typename Tv>
  std::string map_to_str(std::map<Tk, Tv> m)
  {
    std::stringstream ss;
    for (typename std::map<Tk, Tv>::iterator it = m.begin(); it != m.end(); it++)
      ss << "\t" << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
    
    return ss.str();
  }
  
  template <typename T>
  std::string arr_to_str(int size, T* arr_)
  {
    std::stringstream ss;
    for (int i = 0; i < size; i++) {
      ss << boost::lexical_cast<std::string>(arr_[i] );
      if (i < size - 1)
        ss << ",";
    }
    
    return ss.str();
  }
  
  template <typename Tk, typename Tv, class Compare = std::less<Tk> >
  struct thread_safe_map {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv, Compare> map;
    public:
      thread_safe_map() {}
      ~thread_safe_map() {}
      
      Tv& operator[](Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map[k];
      }
      
      int del(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map.erase(map.find(k) );
      }
      
      bool contains(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return !(map.count(k) == 0);
      }
      
      typename std::map<Tk, Tv>::iterator begin() 
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.begin();
      }
      
      typename std::map<Tk, Tv>::iterator end()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.end();
      }
      
      size_t size()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.size();
      }
      
      std::string to_str()
      {
        std::stringstream ss;
        for (typename std::map<Tk, Tv>::iterator it = map.begin(); it != map.end(); it++)
          ss << "\t" << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
        
        return ss.str();
      }
  };
  
  template <typename T, class Compare = std::less<T> >
  struct syncer {
    protected:
      thread_safe_map<T, boost::shared_ptr<boost::condition_variable>, Compare> point_cv_map;
      thread_safe_map<T, boost::shared_ptr<boost::mutex>, Compare> point_m_map;
      thread_safe_map<T, int, Compare> point__num_peers_map;
    public:
      syncer() {}
      ~syncer() {}
      
      int close()
      {
        for (typename std::map<T, boost::shared_ptr<boost::condition_variable> >::iterator it = point_cv_map.begin(); it != point_cv_map.end(); it++)
          (it->second).reset();
        for (typename std::map<T, boost::shared_ptr<boost::mutex> >::iterator it = point_m_map.begin(); it != point_m_map.end(); it++)
          (it->second).reset();
        // 
        LOG(INFO) << "closed:: closed.";
      }
      
      int add_sync_point(T point, int num_peers)
      {
        if (point_cv_map.contains(point) ) {
          LOG(ERROR) << "add_sync_point:: already added point.";
          return 1;
        }
        
        point_cv_map[point] = boost::make_shared<boost::condition_variable>();
        point_m_map[point] = boost::make_shared<boost::mutex>();
        point__num_peers_map[point] = num_peers;
        
        return 0;
      }
      
      int del_sync_point(T point)
      {
        if (!point_cv_map.contains(point) ) {
          LOG(ERROR) << "del_sync_point:: non-existing point.";
          return 1;
        }
        point_cv_map.del(point);
        point_m_map.del(point);
        point__num_peers_map.del(point);
        
        return 0;
      }
      
      int wait(T point)
      {
        boost::mutex::scoped_lock lock(*point_m_map[point] );
        point_cv_map[point]->wait(lock);
        
        return 0;
      }
      
      int notify(T point)
      {
        if (!point_cv_map.contains(point) ) {
          // LOG(ERROR) << "notify:: non-existing point.";
          return 1;
        }
        
        int num_peers_to_wait = point__num_peers_map[point];
        --num_peers_to_wait;
        
        if (num_peers_to_wait == 0) {
          point_cv_map[point]->notify_one();
          return 0;
        }
        point__num_peers_map[point] = num_peers_to_wait;
        
        return 0;
      }
  };
}

#endif //_PATCH_TCP_H_