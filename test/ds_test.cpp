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
  ndim = 3;
  gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  for (int i = 0; i < ndim; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  data_ = (char*)malloc(MSG_SIZE*sizeof(char) );
  std::string msg(MSG);
  int msg_size = msg.size();
  strcpy(data_, msg.c_str() );
  for (int i = msg_size; i < MSG_SIZE - msg_size; i++) {
    data_[i] = '\0';
  }
  // 
  std::cout << "DSTest:: constructed.\n";
}

DSTest::~DSTest()
{
  for (int i = 0; i < num_putget_threads; i++) {
    free(put_thread_ptr_vector[i]);
    free(get_thread_ptr_vector[i]);
  }
  
  patch::free_all<uint64_t*>(3, gdim_, lb_, ub_);
  free(data_);
  // 
  std::cout << "DSTest:: destructed.\n";
}

void* DSTest::bst_repetitive_put(void* context)
{
  return ((DSTest*)context)->repetitive_put();
}

void* DSTest::repetitive_put()
{
  struct pthread_arg_struct arg_struct = put_thread_arg_struct_vector.pop_back();
  
  std::string key = arg_struct.base_key + patch::to_string(arg_struct.thread_id);
  while (1) {
    srand(time(NULL));
    sleep(rand() % 5 + 1);
    
    if (ds_driver.sync_put(key.c_str(), ver, MSG_SIZE*sizeof(char), ndim, gdim_, lb_, ub_, data_) )
      std::cerr << "repetitive_put:: sync_put failed!\n";
      
    std::cout << "repetitive_put:: put key= " << key << "\n";
  }
}

void* DSTest::bst_repetitive_get(void* context)
{
  return ((DSTest*)context)->repetitive_get();
}

void* DSTest::repetitive_get()
{
  struct pthread_arg_struct arg_struct = get_thread_arg_struct_vector.pop_back();
  
  std::string key = arg_struct.base_key + patch::to_string(arg_struct.thread_id);
  while (1) {
    srand(time(NULL));
    sleep(rand() % 5 + 1);
    
    char *data_to_get_ = (char*)malloc(MSG_SIZE*sizeof(char) );
    while ( ds_driver.get(key.c_str(), ver, MSG_SIZE*sizeof(char), ndim, gdim_, lb_, ub_, data_to_get_) ) {
      std::cerr << "repetitive_get:: ds_driver.get failed!";
      usleep(1);
    }
    std::cout << "repetitive_get:: got key= " << key << ", data= " << data_to_get_ << "\n";
    
    free(data_to_get_);
  }
}
/******************************************  run_*  ***********************************************/
int DSTest::run_multithreaded_put_test(std::string base_key)
{
  for (int i = 0; i < num_putget_threads; i++) {
    pthread_arg_struct arg_struct;
    arg_struct.thread_id = i;
    arg_struct.base_key = base_key;
    
    put_thread_arg_struct_vector.push_back(arg_struct);
    
    int r = pthread_create(put_thread_ptr_vector[i], NULL, &DSTest::bst_repetitive_put, (void*)(this) );
    if (r) {
      std::cerr << "run_multithreaded_put_test:: pthread_create failed with return= " << r << ". Exiting...\n";
      exit(1);
    }
  }
}

int DSTest::run_multithreaded_get_test(std::string base_key)
{
  for (int i = 0; i < num_putget_threads; i++) {
    pthread_arg_struct arg_struct;
    arg_struct.thread_id = i;
    arg_struct.base_key = base_key;
    
    get_thread_arg_struct_vector.push_back(arg_struct);
    
    int r = pthread_create(get_thread_ptr_vector[i], NULL, &DSTest::bst_repetitive_get, (void*)(this) );
    if (r) {
      std::cerr << "run_multithreaded_get_test:: pthread_create failed with return= " << r << ". Exiting...\n";
      exit(1);
    }
  }
}

