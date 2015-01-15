#include "ds_drive.h"

#include <pthread.h>

class DSTest {
  public:
    DSTest(int num_dscnodes, int app_id, int num_putget_threads);
    ~DSTest();
    
    int run_multithreaded_put_test(std::string base_key);
    static void* dummy_test(void* context);
    
    void* repetitive_put(void* thread_id_, void* base_key_);
  private:
    int num_dscnodes, app_id, num_putget_threads;
    DSDriver ds_driver;
    // Data attributes for putting and getting
    unsigned int ver;
    int size, ndim;
    uint64_t *gdim_, *lb_, *ub_;
    void *data_;
    // 
    std::vector<pthread_t*> put_thread_ptr_vector;
    std::vector<pthread_t*> get_thread_ptr_vector;
};