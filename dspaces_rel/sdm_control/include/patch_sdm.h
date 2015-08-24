#ifndef _PATCH_SDM_H_
#define _PATCH_SDM_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdarg> // For variable argument lists
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <glog/logging.h>

namespace patch_sdm {
  template <typename T>
  struct thread_safe_vector {
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
  struct thread_safe_map {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv> map;
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
        map.erase(map.find(k) );
        return 0;
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
      
      int size() { return map.size(); }
  };
  
  template <typename T>
  struct syncer {
    private:
      thread_safe_map<T, boost::shared_ptr<boost::condition_variable> > point_cv_map;
      thread_safe_map<T, boost::shared_ptr<boost::mutex> > point_m_map;
      thread_safe_map<T, int> point_numpeers_map;
    public:
      syncer() { LOG(INFO) << "syncer:: constructed."; }
      ~syncer() { LOG(INFO) << "syncer:: destructed."; }
      
      void close()
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
        boost::shared_ptr<boost::condition_variable> t_cv_( new boost::condition_variable() );
        boost::shared_ptr<boost::mutex> t_m_( new boost::mutex() );
        
        point_cv_map[point] = t_cv_;
        point_m_map[point] = t_m_;
        point_numpeers_map[point] = num_peers;
        
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
        point_numpeers_map.del(point);
        
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
          LOG(ERROR) << "notify:: non-existing point.";
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
      }
      
      bool contains(T point) { return point_cv_map.contains(point); }
  };
  
  #define HASH_PRIME 54059
  #define HASH_PRIME_2 76963
  #define HASH_PRIME_3 86969
  unsigned int hash_str(std::string str);
  
  template <typename T>
  void free_all(int num, ...)
  {
    va_list arguments;                     // A place to store the list of arguments
  
    va_start(arguments, num);           // Initializing arguments to store all values after num
    for (int x = 0; x < num; x++)        // Loop all
      free(va_arg(arguments, T*) );
    va_end(arguments);                  // Cleans up the list
  }
}

#endif //_PATCH_SDM_H_