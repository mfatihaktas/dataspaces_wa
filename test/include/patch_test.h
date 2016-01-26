#ifndef _PATCH_TEST_H_
#define _PATCH_TEST_H_

#include <cstdarg> // for variable argument lists
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <map>
#include <pthread.h>

#define str_cstr_equals(x, y) (strcmp(x.c_str(), y) == 0)
#define cstr_cstr_equals(x, y) (strcmp(x, y) == 0)

#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) if (x) {log(ERROR, #x << "failed!") exit(EXIT_FAILURE); }
#define TEST_Z(x)  if (!(x)) {log(ERROR, #x << "failed!") exit(EXIT_FAILURE); }
#endif // _TEST_MACROS_

#define DEBUG_TEST
#ifdef DEBUG_TEST
// static pthread_mutex_t log_m; // = PTHREAD_MUTEX_INITIALIZER;

#define log(type, msg) \
  std::clog << #type " "<< __FILE__ << ":" << __LINE__ << "] " << __func__ << ":: " << msg << std::endl;

  // TEST_NZ(pthread_mutex_lock(&log_m) ) \
  // TEST_NZ(pthread_mutex_unlock(&log_m) )
#else
  #define log(type, msg)
#endif // DEBUG_TEST

#define return_if_err(x, err) \
  err = x; \
  if (err) { \
    log(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }

#define return_err_if_ret_cond_flag(x, ret, cond, flag, err) \
  ret = x; \
  if (ret cond flag) { \
    log(ERROR, __func__ << ":: " #x " failed!") \
    return err; \
  }

namespace patch_test {
    template <typename T> 
    std::string to_str(const T& n) {
      std::ostringstream stm;
      stm << n;
      return stm.str();
    }
    
    template <typename T>
    std::string arr_to_str(int size, const T* arr_) {
      std::stringstream ss;
      for (int i = 0; i < size; i++) {
        ss << arr_[i];
        if (i < size - 1)
          ss << ",";
      }
      
      return ss.str();
    }
    
    template<typename Tk, typename Tv>
    std::string map_to_str(std::map<Tk, Tv> m) {
      std::stringstream ss;
      for (typename std::map<Tk, Tv>::iterator it = m.begin(); it != m.end(); it++)
        ss << "\t" << it->first << " : " << it->second << "\n";
      
      return ss.str();
    }
    
    template <typename T>
    struct thread_safe_vec {
      private:
        pthread_mutex_t mutex;
        typename std::vector<T> v;
      public:
        thread_safe_vec() {
          TEST_NZ(pthread_mutex_init(&mutex, NULL) );
        }
        ~thread_safe_vec() {
          TEST_NZ(pthread_mutex_destroy(&mutex) );
        }
        
        int push_back(T obj) {
          // std::cout << "push_back:: locking mutex.\n";
          TEST_NZ(pthread_mutex_lock(&mutex) );
          // std::cout << "push_back:: will push obj.\n";
          v.push_back(obj);
          // std::cout << "push_back:: pushed obj.\n";
          TEST_NZ(pthread_mutex_unlock(&mutex) );
          // std::cout << "push_back:: unlocked mutex.\n";
        }
        
        T pop_back() {
          T back_obj;
          // std::cerr << "pop_back:: locking mutex.\n";
          TEST_NZ(pthread_mutex_lock(&mutex) );
          if (v.empty() ) {
            // std::cerr << "pop_back:: cannot pop, vector is empty!\n";
          }
          else {
            // std::cout << "pop_back:: will pop obj.\n";
            back_obj = v.back();
            v.pop_back();
            // std::cout << "pop_back:: popped obj.\n";
          }
          TEST_NZ(pthread_mutex_unlock(&mutex) );
          // std::cerr << "pop_back:: unlocked mutex.\n";
          
          return back_obj;
        }
        
        T& operator[](int k) {
          TEST_NZ(pthread_mutex_lock(&mutex) );
          T& obj = v[k];
          TEST_NZ(pthread_mutex_unlock(&mutex) );
          return obj;
        }
    };
    
    template <typename T>
    void free_all(int num, ...) {
      va_list arguments;                     // A place to store the list of arguments
      
      va_start(arguments, num);           // Initializing arguments to store all values after num
      for (int x = 0; x < num; x++)        // Loop until all numbers are added
        va_arg (arguments, T*);
      va_end (arguments);                  // Cleans up the list
    }
    
    template <typename T>
    int write_arr_to_file(uint64_t size, const T* arr_, std::string f_url) {
      std::ofstream f_out(f_url.c_str(), std::ofstream::out);
      if (f_out.is_open() ) {
        for (int i = 0; i < size; i++)
          f_out << arr_[i];
        log(DEBUG, "done; size= " << size)
      }
      else
        return 1;
      return 0;
    }
};

#endif // _PATCH_TEST_H_
