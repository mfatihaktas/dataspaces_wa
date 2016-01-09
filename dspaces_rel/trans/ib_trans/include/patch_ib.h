#ifndef _PATCH_IB_H_
#define _PATCH_IB_H_

#include <cstdio>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <pthread.h>

#include <glog/logging.h>

#define str_str_equals(x,y) (strcmp(x.c_str(), (const char*)y) == 0)
#define str_cstr_equals(x, y) (strcmp(x.c_str(), y) == 0)
#define cstr_cstr_equals(x, y) (strcmp(x, y) == 0)

#define DEBUG_IB
#ifdef DEBUG_IB
#define log_(type, msg) \
  LOG(type) << __func__ << ":: " << msg;
  // std::cerr << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
  // std::clog << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
#else
  #define log_(type, msg) ;
#endif // DEBUG_IB

#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) if (x) {log_(ERROR, #x << "failed!") exit(EXIT_FAILURE); }
#define TEST_Z(x)  if (!(x)) {log_(ERROR, #x << "failed!") exit(EXIT_FAILURE); }

#define return_if_err(x, err) \
  err = x; \
  if (err) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }

#define return_err_if_ret_cond_flag(x, ret, cond, flag, err) \
  ret = x; \
  if (ret cond flag) { \
    log_(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }

#endif // _TEST_MACROS_

namespace patch_ib {
  template <typename T>
  std::string to_str(T in)
  {
    std::stringstream ss;
    ss << in;
    
    return ss.str();
  }
  
  template<typename Tk, typename Tv>
  std::string map_to_str(std::map<Tk, Tv> m)
  {
    std::stringstream ss;
    for (typename std::map<Tk, Tv>::iterator it = m.begin(); it != m.end(); it++)
      ss << "\t" << it->first << " : " << it->second << "\n";
    
    return ss.str();
  }
  
  template <typename T>
  std::string arr_to_str(int size, T* arr_)
  {
    std::stringstream ss;
    for (int i = 0; i < size; i++) {
      ss << arr_[i];
      if (i < size - 1)
        ss << ",";
    }
    
    return ss.str();
  }
  
  template <typename Tk, typename Tv, class Compare = std::less<Tk> >
  struct thread_safe_map {
    private:
      pthread_mutex_t mutex;
      typename std::map<Tk, Tv, Compare> map;
    public:
      thread_safe_map() {
        TEST_NZ(pthread_mutex_init(&mutex, NULL) );
      }
      
      ~thread_safe_map() {
        TEST_NZ(pthread_mutex_destroy(&mutex) );
      }
      
      Tv& operator[](Tk k) {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        Tv& r = map[k];
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return r;
      }
      
      int del(Tk k) {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        map.erase(map.find(k) );
        TEST_NZ(pthread_mutex_unlock(&mutex) );
      }
      
      bool contains(Tk k) {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        bool r = !(map.count(k) == 0);
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return r;
      }
      
      typename std::map<Tk, Tv>::iterator begin() {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        typename std::map<Tk, Tv>::iterator it = map.begin();
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return it;
      }
      
      typename std::map<Tk, Tv>::iterator end() {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        typename std::map<Tk, Tv>::iterator it = map.end();
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return it;
      }
      
      int size() {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        int r = map.size();
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return r;
      }
      
      std::string to_str() {
        std::stringstream ss;
        for (typename std::map<Tk, Tv>::iterator it = map.begin(); it != map.end(); it++)
          ss << "\t" << it->first << " : " << it->second << "\n";
        
        return ss.str();
      }
  };
  
  template <typename T, class Compare = std::less<T> >
  struct syncer {
    protected:
      thread_safe_map<T, pthread_cond_t*, Compare> point_cv_map;
      thread_safe_map<T, pthread_mutex_t*, Compare> point_m_map;
      thread_safe_map<T, int, Compare> point__num_peers_map;
    public:
      syncer() {}
      ~syncer() {}
      
      int close() {
        for (typename std::map<T, pthread_cond_t*>::iterator it = point_cv_map.begin(); it != point_cv_map.end(); it++)
          TEST_NZ(pthread_cond_destroy(it->second) );
        for (typename std::map<T, pthread_mutex_t*>::iterator it = point_m_map.begin(); it != point_m_map.end(); it++)
          TEST_NZ(pthread_mutex_destroy(it->second) );
        // 
        log_(INFO, "closed.")
      }
      
      int add_sync_point(T point, int num_peers) {
        if (point_cv_map.contains(point) ) {
          log_(ERROR, "already added point; point= " << point)
          return 1;
        }
        pthread_cond_t* cv_ = new pthread_cond_t();
        TEST_NZ(pthread_cond_init(cv_, NULL) );
        point_cv_map[point] = cv_;
        
        pthread_mutex_t* m_ = new pthread_mutex_t();
        TEST_NZ(pthread_mutex_init(m_, NULL) );
        point_m_map[point] = m_;
        
        point__num_peers_map[point] = num_peers;
        
        return 0;
      }
      
      int del_sync_point(T point) {
        if (!point_cv_map.contains(point) ) {
          log_(ERROR, "non-existing point= " << point)
          return 1;
        }
        point_cv_map.del(point);
        point_m_map.del(point);
        point__num_peers_map.del(point);
        
        return 0;
      }
      
      int wait(T point) {
        TEST_NZ(pthread_mutex_lock(point_m_map[point] ) );
        TEST_NZ(pthread_cond_wait(point_cv_map[point], point_m_map[point] ) );
        TEST_NZ(pthread_mutex_unlock(point_m_map[point] ) );
        
        return 0;
      }
      
      int notify(T point) {
        if (!point_cv_map.contains(point) ) {
          // log_(ERROR, "notify:: non-existing point.")
          return 1;
        }
        
        int num_peers_to_wait = point__num_peers_map[point];
        --num_peers_to_wait;
        
        if (num_peers_to_wait == 0) {
          TEST_NZ(pthread_mutex_lock(point_m_map[point] ) );
          TEST_NZ(pthread_cond_signal(point_cv_map[point] ) );
          TEST_NZ(pthread_mutex_unlock(point_m_map[point] ) );
          
          return 0;
        }
        point__num_peers_map[point] = num_peers_to_wait;
        
        return 0;
      }
  };
  
  template <typename T>
  class BQueue { //Blocking
    private:
      pthread_mutex_t mutex;
      pthread_cond_t cv;
      std::deque<T> dq;
    public:
      BQueue() {
        TEST_NZ(pthread_mutex_init(&mutex, NULL) );
        TEST_NZ(pthread_cond_init(&cv, NULL) );
      }
      
      std::string to_str() {
        std::stringstream ss;
        ss << "\t ->";
        for (typename std::deque<T>::iterator it = dq.begin(); it != dq.end(); ++it)
          ss << patch_ib::to_str(*it) << ", ";
        ss << "-> \n";
        
        return ss.str();
      };
      
      int size() {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        int size = dq.size();
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return size;
      }
      
      void push(T const& value) {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        dq.push_front(value);
        TEST_NZ(pthread_cond_signal(&cv) );
        TEST_NZ(pthread_mutex_unlock(&mutex) );
      }
      
      T pop() {
        TEST_NZ(pthread_mutex_lock(&mutex) );
        while (dq.empty() )
          TEST_NZ(pthread_cond_wait(&cv, &mutex) );
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        TEST_NZ(pthread_mutex_lock(&mutex) );
        T rc = dq.back();
        dq.pop_back();
        TEST_NZ(pthread_mutex_unlock(&mutex) );
        
        return rc;
      }
  };
};

#endif // _PATCH_IB_H_