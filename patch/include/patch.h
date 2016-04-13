#ifndef _PATCH_H_
#define _PATCH_H_

#include <arpa/inet.h> // For sockaddr_in_to_str
#include <cstdarg> // For variable argument lists
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include "debug.h"

namespace patch {
  template <typename T>
  void free_all(int num, ...) {
    va_list arguments;                     // A place to store the list of arguments

    va_start(arguments, num);           // Initializing arguments to store all values after num
    for (int x = 0; x < num; x++) {       // Loop all
      T* arg_ = va_arg(arguments, T*);
      free(arg_); arg_ = NULL;
    }
    va_end(arguments);                  // Cleans up the list
  };
  
  #define HASH_PRIME 54059
  #define HASH_PRIME_2 76963
  #define HASH_PRIME_3 86969
  static unsigned int hash_str(std::string str)
  {
    unsigned int h = 31; // Also prime
    const char* s_ = str.c_str();
    while (*s_) {
      h = (h * HASH_PRIME) ^ (s_[0] * HASH_PRIME_2);
      s_++;
    }
    return h; // return h % HASH_PRIME_3;
  }
  
  static std::string sockaddr_in_to_str(struct sockaddr_in addr)
  {
    char lip_[INET_ADDRSTRLEN];
    TEST_Z(inet_ntop(AF_INET, &(addr.sin_addr), lip_, sizeof(lip_) ) )
    
    std::stringstream ss;
    ss << lip_ << ":" << addr.sin_port;
    
    return ss.str();
  }
  
  template <typename T>
  std::string to_str(T in)
  {
    std::stringstream ss;
    ss << in;
    
    return ss.str();
  }
  
  template<typename T>
  std::string vec_to_str(std::vector<T> v) {
    std::stringstream ss;
    for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); it++) {
      ss << *it;
      if (it != (v.end() - 1) )
        ss << ", ";
    }
    
    return ss.str();
  };
  
  template<typename T>
  std::string set_to_str(std::set<T> s) {
    std::stringstream ss;
    for (typename std::set<T>::iterator it = s.begin(); it != s.end(); it++)
      ss << *it << ", ";
    
    return ss.str();
  };
  
  template<typename T>
  std::string deque_to_str(std::deque<T> q) {
    std::stringstream ss;
    for (typename std::deque<T>::iterator it = q.begin(); it != q.end(); it++)
      ss << *it << "\n";
    
    return ss.str();
  };
  
  template<typename T>
  std::string pdeque_to_str(std::deque<T> q) {
    std::stringstream ss;
    for (typename std::deque<T>::iterator it = q.begin(); it != q.end(); it++)
      ss << "<" << it->first << ", " << it->second << ">\n";
    
    return ss.str();
  };
  
  template<typename PAIR_T>
  std::string pvec_to_str(std::vector<PAIR_T> v) {
    std::stringstream ss;
    for (typename std::vector<PAIR_T>::iterator it = v.begin(); it != v.end(); it++)
      ss << "<" << it->first << ", " << it->second << ">\n";
    
    return ss.str();
  };
  
  template<typename Tk, typename Tv>
  std::string map_to_str(std::map<Tk, Tv> m) {
    std::stringstream ss;
    for (typename std::map<Tk, Tv>::iterator it = m.begin(); it != m.end(); it++)
      ss << "\t" << it->first << " : " << it->second << "\n";
    
    return ss.str();
  }
  
  template <typename T>
  std::string arr_to_str(size_t size, T* arr_) {
    std::stringstream ss;
    for (int i = 0; i < size; i++) {
      ss << arr_[i];
      if (i < size - 1)
        ss << ",";
    }
    
    return ss.str();
  }
  
  template <typename T>
  struct thread_safe_vector {
    private:
      boost::mutex mutex;
      typename std::vector<T> v;
    public:
      thread_safe_vector() {}
      ~thread_safe_vector() {}
      
      T& operator[](int i) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return v[i];
      }
      
      void push_back(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        v.push_back(e);
      }
      
      int del(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        v.erase(std::find(v.begin(), v.end(), e) );
        return 0;
      }
      
      int size() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return v.size();
      }
      
      bool contains(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return (std::find(v.begin(), v.end(), e) != v.end() );
      }
      
      typename std::vector<T>::iterator begin() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return v.begin();
      }
      
      typename std::vector<T>::iterator end() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return v.end();
      }
      
      void clear() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        v.clear();
      }
      
      std::string to_str() {
        std::stringstream ss;
        for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); it++) {
          ss << *it;
          if (it != (v.end() - 1) )
            ss << ", ";
        }
        
        return ss.str();
      }
  };
  
  template <typename T>
  struct thread_safe_set {
    private:
      boost::mutex mutex;
      typename std::set<T> s;
    public:
      thread_safe_set() {}
      ~thread_safe_set() {}
      
      void add(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        s.insert(e);
      }
      
      int del(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        s.erase(std::find(s.begin(), s.end(), e) );
        return 0;
      }
      
      int size() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return s.size();
      }
      
      bool contains(T e) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return (std::find(s.begin(), s.end(), e) != s.end() );
      }
      
      typename std::set<T>::iterator begin() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return s.begin();
      }
      
      typename std::set<T>::iterator end() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return s.end();
      }
      
      void clear() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        s.clear();
      }
      
      std::string to_str() {
        std::stringstream ss;
        for (typename std::set<T>::iterator it = s.begin(); it != s.end(); it++) {
          ss << *it;
          if (it != (s.end() - 1) )
            ss << ", ";
        }
        
        return ss.str();
      }
  };
  
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
      
      int del(Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map.erase(map.find(k) );
      }
      
      bool contains(Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return !(map.count(k) == 0);
      }
      
      typename std::map<Tk, Tv>::iterator begin() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.begin();
      }
      
      typename std::map<Tk, Tv>::iterator end() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.end();
      }
      
      size_t size() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.size();
      }
      
      void clear() {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map.clear();
      }
      
      std::string to_str() {
        std::stringstream ss;
        for (typename std::map<Tk, Tv>::iterator it = map.begin(); it != map.end(); it++)
          ss << "\t" << it->first << " : " << it->second << "\n";
        
        return ss.str();
      }
  };
  
  // template <typename Tk, typename Tv, class Compare = std::less<Tk> >
  // struct not_thread_safe_map {
  //   private:
  //     // boost::mutex mutex;
  //     typename std::map<Tk, Tv, Compare> map;
  //   public:
  //     not_thread_safe_map() {}
  //     ~not_thread_safe_map() {}
      
  //     Tv& operator[](Tk k) {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       return map[k];
  //     }
      
  //     int del(Tk k)
  //     {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       map.erase(map.find(k) );
  //     }
      
  //     bool contains(Tk k)
  //     {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       return !(map.count(k) == 0);
  //     }
      
  //     typename std::map<Tk, Tv>::iterator begin() 
  //     {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       return map.begin();
  //     }
      
  //     typename std::map<Tk, Tv>::iterator end()
  //     {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       return map.end();
  //     }
      
  //     size_t size()
  //     {
  //       // boost::lock_guard<boost::mutex> guard(this->mutex);
  //       return map.size();
  //     }
      
  //     std::string to_str()
  //     {
  //       std::stringstream ss;
  //       for (typename std::map<Tk, Tv>::iterator it = map.begin(); it != map.end(); it++)
  //         ss << "\t" << it->first << " : " << it->second << "\n";
        
  //       return ss.str();
  //     }
  // };
  
  template <typename T, class Compare = std::less<T> >
  struct syncer {
    protected:
      thread_safe_map<T, boost::shared_ptr<boost::condition_variable>, Compare> point_cv_map;
      thread_safe_map<T, boost::shared_ptr<boost::mutex>, Compare> point_m_map;
      thread_safe_map<T, int, Compare> point__num_peers_map;
    public:
      syncer() {}
      ~syncer() {}
      
      int close() {
        for (typename std::map<T, boost::shared_ptr<boost::condition_variable> >::iterator it = point_cv_map.begin(); it != point_cv_map.end(); it++)
          (it->second).reset();
        for (typename std::map<T, boost::shared_ptr<boost::mutex> >::iterator it = point_m_map.begin(); it != point_m_map.end(); it++)
          (it->second).reset();
        // 
        log_(INFO, "closed.")
      }
      
      int add_sync_point(T point, int num_peers) {
        if (point_cv_map.contains(point) ) {
          log_(ERROR, "already added point; point.")
          return 1;
        }
        
        point_cv_map[point] = boost::make_shared<boost::condition_variable>();
        point_m_map[point] = boost::make_shared<boost::mutex>();
        point__num_peers_map[point] = num_peers;
        
        return 0;
      }
      
      int del_sync_point(T point) {
        if (!point_cv_map.contains(point) ) {
          log_(ERROR, "non-existing point.")
          return 1;
        }
        point_cv_map.del(point);
        point_m_map.del(point);
        point__num_peers_map.del(point);
        
        return 0;
      }
      
      int wait(T point) {
        boost::mutex::scoped_lock lock(*point_m_map[point] );
        point_cv_map[point]->wait(lock);
        
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
          point_cv_map[point]->notify_one();
          return 0;
        }
        point__num_peers_map[point] = num_peers_to_wait;
        
        return 0;
      }
  };
  
  template <typename T>
  class Queue { // Queue
    private:
      int size;
      
      std::deque<T> q;
    public:
      Queue(int size)
      : size(size)
      {}
      
      T& operator[](int i) { return q[i]; }
      
      typename std::deque<T>::iterator begin() { return q.begin(); }
      
      typename std::deque<T>::iterator end() { return q.end(); }
      
      void clear() { q.clear(); }
      
      void push(T const& value) {
        if (q.size() == size)
          pop();
        q.push_front(value);
      }
      
      T pop() {
        T rc = q.back();
        q.pop_back();
        
        return rc;
      }
      
      std::string to_str() {
        std::stringstream ss;
        ss << "->";
        for (typename std::deque<T>::iterator it = q.begin(); it != q.end(); ++it)
          ss << *it << ", ";
        ss << "-> \n";
        
        return ss.str();
      }
  };
  
  template <typename T>
  class BQueue { // Blocking Queue
    private:
      boost::mutex mutex, mutex_pop;
      boost::condition_variable condition_pop;
      std::deque<T> d_queue;
    public:
      void push(T const& value) {
        // usleep(3*1000*1000);
        {
          boost::lock_guard<boost::mutex> guard(mutex);
          d_queue.push_front(value);
        }
        condition_pop.notify_one();
      }
      
      T pop() {
        while(d_queue.empty() ) {
          boost::mutex::scoped_lock lock(mutex_pop);
          condition_pop.wait(lock);
        }
        
        T rc;
        {
          boost::lock_guard<boost::mutex> guard(mutex);
          rc = d_queue.back();
          d_queue.pop_back();
        }
        return rc;
      }
      
      std::string to_str() {
        std::stringstream ss;
        ss << "\t ->";
        for (typename std::deque<T>::iterator it = d_queue.begin(); it != d_queue.end(); ++it)
          ss << *it << ", ";
        ss << "-> \n";
        
        return ss.str();
      }
      
      void create_timed_push_thread(T value) {
        boost::thread(&BQueue<T>::push, this, value);
      }
  };
}

#endif // _PATCH_H_
