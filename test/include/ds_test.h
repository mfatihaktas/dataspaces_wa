#include <stdlib.h>
#include <math.h>
#include <cstring> // some old compilers require this for strcpy

#include "ds_drive.h"

struct pthread_arg_struct {
  int thread_id;
  std::string base_key;
  uint64_t data_size;
};

typedef int DATA_T;

class DSTest {
  private:
    int num_dscnodes, app_id, num_putget_threads;
    DSDriver ds_driver;
    // Data attributes for putting and getting
    unsigned int ver;
    int ndim;
    uint64_t *gdim_, *lb_, *ub_;
    DATA_T* data_;
    // 
    patch_test::thread_safe_vec<pthread_t*> put_thread_ptr_vector;
    patch_test::thread_safe_vec<pthread_t*> get_thread_ptr_vector;
    patch_test::thread_safe_vec<struct pthread_arg_struct> put_thread_arg_struct_v;
    patch_test::thread_safe_vec<struct pthread_arg_struct> get_thread_arg_struct_v;
  public:
    DSTest(int num_dscnodes, int app_id, int num_putget_threads);
    ~DSTest();
    std::string to_str();
    int init(uint64_t data_size = 0);
    
    int exp_put(uint64_t data_size);
    int exp_get(uint64_t data_size);
    
    int repetitive_put();
    int repetitive_get();
    
    int run_multithreaded_put_test(std::string base_key, uint64_t data_size = 0);
    int run_multithreaded_get_test(std::string base_key, uint64_t data_size = 0);
};

struct wrap_DSTest {
  DSTest* ds_test_;
  
  wrap_DSTest(DSTest* ds_test_)
  : ds_test_(ds_test_)
  {}
};
