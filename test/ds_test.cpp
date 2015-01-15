#include "ds_test.h"

#define MSG_SIZE 50
#define MSG "test 1 test 2 test 3 test 4 test 5"

DSTest::DSTest(int num_dscnodes, int app_id, int num_putget_threads)
: num_dscnodes(num_dscnodes),
  app_id(app_id),
  num_putget_threads(num_putget_threads),
  ds_driver(num_dscnodes, app_id)
{
  for (int i = 0; i < num_putget_threads; i++) {
    put_thread_ptr_vector.push_back(new pthread_t());
    get_thread_ptr_vector.push_back(new pthread_t());
  }
  // 
  ver = 0;
  size = MSG_SIZE*sizeof(char);
  ndim = 3;
  for (int i = 0; i < ndim; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  char *data_ = (char*)malloc(MSG_SIZE*sizeof(char) );
  std::string msg(MSG);
  int msg_size = msg.size();
  strcpy(data_, msg.c_str() );
  for (int i = msg_size; i < MSG_SIZE - msg_size; i++) {
    data_[i] = '\0';
  }
  //
  std::cout << "DSTest:: constructed.";
}

DSTest::~DSTest()
{
  for (int i = 0; i < num_putget_threads; i++) {
    free(put_thread_ptr_vector[i]);
    free(get_thread_ptr_vector[i]);
  }
  
  free(data_);
  //
  std::cout << "DSTest:: destructed.";
}


int DSTest::run_multithreaded_put_test(std::string base_key)
{
  for (int i = 0; i < num_putget_threads; i++) {
    // int r = pthread_create(put_thread_ptr_vector[i], NULL, &DSTest::repetitive_put, (void*)&i, (void*)&base_key);
    int r = pthread_create(put_thread_ptr_vector[i], NULL, &DSTest::dummy_test, (void*)this);
    if (r) {
      std::cerr << "run_multithreaded_put_test:: r= " << r << ". Exiting...\n";
      exit(1);
    }
  }
}

void* DSTest::dummy_test(void* context)
{
  std::cout << "dummy_test:: hey!\n";
}

void* DSTest::repetitive_put(void* thread_id_, void* base_key_)
{
  int thread_id = *(int*)thread_id_;
  std::string base_key = *(std::string*)base_key_;
  
  std::cout << "repetitive_put:: thread_id= " << thread_id << ", base_key= " << base_key << "\n";
  // std::string key = base_key + std::to_string(thread_id);
  // while (1) {
  //   srand(time(NULL));
  //   sleep(rand() % 5 + 1);
    
  //   if (ds_driver.sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) )
  //     std::cerr << "repetitive_put:: sync_put failed!\n";
  // }
}