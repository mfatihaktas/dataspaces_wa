#include "ds_drive.h"

#include <string.h>
#include <pthread.h>
#include <cstdarg> //for variable argument lists
#include <sstream>

namespace patch
{
    template <typename T> 
    std::string to_string(const T& n)
    {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
    
    template <typename T>
    struct thread_safe_vector
    {
      private:
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        typename std::vector<T> v;
      public:
        thread_safe_vector() {};
        ~thread_safe_vector() {};
        
        int push_back(T obj)
        {
          // std::cout << "push_back:: locking mutex.\n";
          pthread_mutex_lock(&mutex);
          // std::cout << "push_back:: will push obj.\n";
          v.push_back(obj);
          // std::cout << "push_back:: pushed obj.\n";
          pthread_mutex_unlock(&mutex);
          // std::cout << "push_back:: unlocked mutex.\n";
        };
        
        T pop_back()
        {
          T back_obj;
          // std::cerr << "pop_back:: locking mutex.\n";
          pthread_mutex_lock(&mutex);
          if (v.empty() ) {
            // std::cerr << "pop_back:: cannot pop, vector is empty!\n";
          }
          else {
            // std::cout << "pop_back:: will pop obj.\n";
            back_obj = v.back();
            v.pop_back();
            // std::cout << "pop_back:: popped obj.\n";
          }
          pthread_mutex_unlock(&mutex);
          // std::cerr << "pop_back:: unlocked mutex.\n";
          
          return back_obj;
        };
        
        T& operator[](int k) {
          pthread_mutex_lock(&mutex);
          T& obj = v[k];
          pthread_mutex_unlock(&mutex);
          return obj;
        };
    };
    
    template <typename T>
    void free_all(int num, ...)
    {
      va_list arguments;                     // A place to store the list of arguments
    
      va_start ( arguments, num );           // Initializing arguments to store all values after num
      
      for ( int x = 0; x < num; x++ )        // Loop until all numbers are added
        va_arg ( arguments, T* );
      
      va_end ( arguments );                  // Cleans up the list
    }
}

struct pthread_arg_struct {
  int thread_id;
  std::string base_key;
};

class DSTest {
  public:
    DSTest(int num_dscnodes, int app_id, int num_putget_threads);
    ~DSTest();
    
    int run_multithreaded_put_test(std::string base_key);
    int run_multithreaded_get_test(std::string base_key);
    
    static void* bst_repetitive_put(void* context);
    void* repetitive_put();
    static void* bst_repetitive_get(void* context);
    void* repetitive_get();
  private:
    int num_dscnodes, app_id, num_putget_threads;
    DSDriver ds_driver;
    // Data attributes for putting and getting
    unsigned int ver;
    int ndim;
    uint64_t *gdim_, *lb_, *ub_;
    char* data_;
    // 
    patch::thread_safe_vector<pthread_t*> put_thread_ptr_vector;
    patch::thread_safe_vector<pthread_t*> get_thread_ptr_vector;
    patch::thread_safe_vector<struct pthread_arg_struct> put_thread_arg_struct_vector;
    patch::thread_safe_vector<struct pthread_arg_struct> get_thread_arg_struct_vector;
};
