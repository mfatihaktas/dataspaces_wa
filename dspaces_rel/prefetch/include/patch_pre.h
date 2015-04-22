#ifndef _PATCH_PRE_H_
#define _PATCH_PRE_H_

#include <vector>
#include <map>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

namespace patch_pre {
  template<typename T>
  std::string vector_to_str(std::vector<T> v)
  {
    std::stringstream ss;
    for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); it++) {
      ss << boost::lexical_cast<std::string>(*it);
      if (it != (v.end() - 1) )
        ss << ", ";
    }
    
    return ss.str();
  };
  
  template <typename T>
  struct thread_safe_vector
  {
    private:
      boost::mutex mutex;
      typename std::vector<T> vector;
      typename std::vector<T>::iterator it;
    public:
      thread_safe_vector() {};
      ~thread_safe_vector() {};
      
      T& operator[](int i) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return vector[i];
      };
      
      void push_back(T e)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        vector.push_back(e);
      }
      
      int del(T e)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        it = std::find(vector.begin(), vector.end(), e);
        vector.erase(it);
        return 0;
      };
      
      bool contains(T e)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return (std::find(vector.begin(), vector.end(), e) != vector.end() );
      };
      
      typename std::vector<T>::iterator begin()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return vector.begin();
      };
      
      typename std::vector<T>::iterator end()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return vector.end();
      };
  };
  
  template <typename Tk, typename Tv>
  struct thread_safe_map
  {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv> map;
      typename std::map<Tk, Tv>::iterator map_it;
    public:
      thread_safe_map() {};
      ~thread_safe_map() {};
      
      Tv& operator[](Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map[k];
      };
      
      int del(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map_it = map.find(k);
        map.erase(map_it);
      };
      
      bool contains(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return !(map.count(k) == 0);
      };
      
      typename std::map<Tk, Tv>::iterator begin() 
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.begin();
      };
      
      typename std::map<Tk, Tv>::iterator end()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.end();
      };
      
      size_t size()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.size();
      };
  };
  
  template <typename T>
  struct syncer 
  {
    private:
      thread_safe_map<T, boost::shared_ptr<boost::condition_variable> > point_cv_map;
      thread_safe_map<T, boost::shared_ptr<boost::mutex> > point_m_map;
      thread_safe_map<T, int> point_numpeers_map;
    public:
      syncer() {LOG(INFO) << "syncer:: constructed."; };
      ~syncer() { LOG(INFO) << "syncer:: destructed."; };
      int add_sync_point(T point, int num_peers)
      {
        if (point_cv_map.contains(point) ) {
          LOG(ERROR) << "add_sync_point:: already added point.";
          return 1;
        }
        boost::shared_ptr<boost::condition_variable> t_cv_( new boost::condition_variable() );
        boost::shared_ptr<boost::mutex> t_m_( new boost::mutex() );
        
        point_cv_map[point] = t_cv_;
        point_m_map[point] = t_m_;
        point_numpeers_map[point] = num_peers;
        
        return 0;
      };
      int del_sync_point(T point)
      {
        if (!point_cv_map.contains(point) ) {
          LOG(ERROR) << "del_sync_point:: non-existing point.";
          return 1;
        }
        point_cv_map.del(point);
        point_m_map.del(point);
        point_numpeers_map.del(point);
        
        return 0;
      };
      int wait(T point)
      {
        boost::mutex::scoped_lock lock(*point_m_map[point]);
        point_cv_map[point]->wait(lock);
        
        return 0;
      };
      int notify(T point)
      {
        if (!point_cv_map.contains(point) ) {
          // LOG(ERROR) << "notify:: non-existing point.";
          return 1;
        }
        
        int num_peers_to_wait = point_numpeers_map[point];
        --num_peers_to_wait;
        
        if (num_peers_to_wait == 0) {
          point_cv_map[point]->notify_one();
          return 0;
        }
        point_numpeers_map[point] = num_peers_to_wait;
        
        return 0;
      };
  };
}

#endif // _PATCH_PRE_H_
