#ifndef _PATCH_TCP_H_
#define _PATCH_TCP_H_

#include <map>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include <glog/logging.h>

#ifndef _STR_MACROS_
#define _STR_MACROS_
#define str_str_equals(x,y) (strcmp(x.c_str(), y.c_str() ) == 0)
#define str_cstr_equals(x, y) (strcmp(x.c_str(), (const char*)y) == 0)
#define cstr_cstr_equals(x, y) (strcmp((const char*)x, (const char*)y) == 0)
#endif // _STR_MACROS_

#define _DEBUG_
#ifdef _DEBUG_
#define log_(type, msg) \
  LOG(type) << __func__ << ":: " << msg;
  // std::cerr << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
  // std::clog << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;
#else
  #define log_(type, msg) ;
#endif // _DEBUG_

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
  
  #define HASH_PRIME 54059
  #define HASH_PRIME_2 76963
  #define HASH_PRIME_3 86969
  unsigned int hash_str(std::string str);
}

#endif //_PATCH_TCP_H_