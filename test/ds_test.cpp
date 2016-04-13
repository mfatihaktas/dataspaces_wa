#include "ds_test.h"

extern "C" void* call_repetitive_put_w_wrap(void* wrap_)
{
  wrap_DSTest* w_(static_cast<wrap_DSTest*>(wrap_) );
  int err;
  return_err_if_ret_cond_flag(w_->ds_test_->repetitive_put(), err, !=, 0, NULL)
  
  delete w_;
  return 0;
}

extern "C" void* call_repetitive_get_w_wrap(void* wrap_)
{
  wrap_DSTest* w_(static_cast<wrap_DSTest*>(wrap_) );
  int err;
  return_err_if_ret_cond_flag(w_->ds_test_->repetitive_get(), err, !=, 0, NULL)
  
  delete w_;
  return 0;
}

#define MSG_SIZE 50
#define MSG "test 1 test 2 test 3 test 4 test 5"
#define RANDOM_INT_RANGE 100

DSTest::DSTest(int num_dscnodes, int app_id, int num_putget_threads)
: num_dscnodes(num_dscnodes), app_id(app_id), num_putget_threads(num_putget_threads),
  ds_driver(num_dscnodes, app_id)
{
  for (int i = 0; i < num_putget_threads; i++) {
    put_thread_ptr_vector.push_back(new pthread_t() );
    get_thread_ptr_vector.push_back(new pthread_t() );
  }
  // 
  log(INFO, "constructed.")
}

DSTest::~DSTest()
{
  for (int i = 0; i < num_putget_threads; i++) {
    free(put_thread_ptr_vector[i] );
    free(get_thread_ptr_vector[i] );
  }
  
  patch_test::free_all<uint64_t*>(3, gdim_, lb_, ub_);
  free(data_);
  // 
  log(INFO, "destructed.")
}

std::string DSTest::to_str()
{
  std::stringstream ss;
  ss << "ndim= " << ndim << "\n"
     << "gdim_= " << patch_test::arr_to_str<>(ndim, gdim_) << "\n"
     << "lb_= " << patch_test::arr_to_str<>(ndim, lb_) << "\n"
     << "ub_= " << patch_test::arr_to_str<>(ndim, ub_) << "\n";
  
  return ss.str();
}

int DSTest::init(uint64_t data_size)
{
  log(INFO, "with data_size= " << data_size)
  ver = 0;
  ndim = 3;
  gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
  uint64_t ub_limit = floor(pow((float)data_size/sizeof(DATA_T), (float)1/ndim) );
  for (int i = 0; i < ndim; i++) {
    gdim_[i] = ub_limit;
    lb_[i] = 0;
    ub_[i] = ub_limit - 1;
  }
  
  if (data_size == 0) {
    // data_ = (DATA_T*)malloc(MSG_SIZE*sizeof(DATA_T) );
    // std::string msg(MSG);
    // int msg_size = msg.size();
    // strcpy(data_, msg.c_str() );
    // data_[msg_size] = '\0';
  }
  else {
    data_ = (DATA_T*)malloc(data_size);
    
    // int chunk_size = 100*1024*1024;
    // for (int i = 0; i < data_size; i++) {
    //   char c = patch_test::to_str<>(i/chunk_size).c_str()[0];
    //   data_[i] = c;
    // }
    
    // for (int i = 0; i < data_size/chunk_size; i++) {
    //   std::string chunk_header = "chunk_" + patch_test::to_str<>(i);
    //   memcpy(data_ + i*chunk_size, chunk_header.c_str(), chunk_header.size() );
    //   data_[i*chunk_size + chunk_header.size() ] = '\0';
    // }
    
    // for (int i = 0; i < data_size; i++)
    //   data_[i] = static_cast<char>(i%26 + 97);
      // data_[i] = static_cast<char>(rand()%26 + 97);
  }
  
  log(INFO, "to_str= \n" << to_str() )
  return 0;
}

// ----------------------------------  Single-threaded  ----------------------------------------- //
int DSTest::exp_put(uint64_t data_size)
{
  int err;
  return_if_err(init(data_size), err)
  std::string key = "thread0";
  
  uint64_t exp_lb_1_[] = {1, 1};
  uint64_t exp_ub_1_[] = {3, 3};
  int data_length = DSDriver::get_data_length(ndim, gdim_, exp_lb_1_, exp_ub_1_);
  DATA_T* data_to_put_ = (DATA_T*)malloc(data_size);
  for (int i = 0; i < data_length; i++)
    data_to_put_[i] = static_cast<DATA_T>(i%26 + 97);
  return_if_err(ds_driver.sync_put(key.c_str(), ver, sizeof(DATA_T), ndim, gdim_, exp_lb_1_, exp_ub_1_, data_to_put_), err)
  log(INFO, "put key= " << key << ", data_size= " << data_size << ", data_= \n" << patch_test::arr_to_str<>(data_size, data_to_put_) )
  free(data_to_put_);
  
  uint64_t exp_lb_2_[] = {1, 4};
  uint64_t exp_ub_2_[] = {3, 5};
  data_length = DSDriver::get_data_length(ndim, gdim_, exp_lb_2_, exp_ub_2_);
  data_to_put_ = (DATA_T*)malloc(data_size);
  for (int i = 0; i < data_length; i++)
    data_to_put_[i] = static_cast<DATA_T>(i%26 + 97);
  return_if_err(ds_driver.sync_put(key.c_str(), ver, sizeof(DATA_T), ndim, gdim_, exp_lb_2_, exp_ub_2_, data_to_put_), err)
  log(INFO, "put key= " << key << ", data_size= " << data_size << ", data_= \n" << patch_test::arr_to_str<>(data_size, data_to_put_) )
  free(data_to_put_);
}

int DSTest::exp_get(uint64_t data_size)
{
  int err;
  return_if_err(init(data_size), err)
  std::string key = "thread0";
  uint64_t exp_lb_[] = {1, 1};
  uint64_t exp_ub_[] = {3, 5};
  
  // data_size = DSDriver::get_data_length(ndim, gdim_, exp_lb_, exp_ub_);
  DATA_T* data_to_get_ = (DATA_T*)malloc(data_size);
  return_if_err(ds_driver.get(key.c_str(), ver, sizeof(DATA_T), ndim, gdim_, exp_lb_, exp_ub_, data_to_get_), err)
  
  log(INFO, "got key= " << key << ", data_size= " << data_size << ", data_= \n" << patch_test::arr_to_str<>(data_size, data_to_get_) )
  
  free(data_to_get_);
  return 0;
}

// -----------------------------------  Multi-threaded  ----------------------------------------- //
#define NUM_REP 5

int DSTest::repetitive_put()
{
  int err;
  struct pthread_arg_struct arg_struct = put_thread_arg_struct_v.pop_back();
  return_if_err(init(arg_struct.data_size), err)
  uint64_t data_size = (arg_struct.data_size == 0) ? MSG_SIZE : arg_struct.data_size;
  
  srand(time(NULL) );
  std::string base_key = arg_struct.base_key + patch_test::to_str(arg_struct.thread_id);
  int counter = 0;
  while (1) {
    // float s_time = abs(3*(float)(rand() % RANDOM_INT_RANGE)/RANDOM_INT_RANGE);
    // sleep(s_time);
    // log(INFO, "slept for " << s_time)
    
    std::string key = base_key + "_" + patch_test::to_str(counter);
    return_if_err(ds_driver.sync_put(key.c_str(), ver, sizeof(DATA_T), ndim, gdim_, lb_, ub_, data_), err)
    log(INFO, "put key= " << key << ", data_size= " << data_size)
    // log(INFO, "put key= " << key << ", data_size= " << data_size << ", data_= \n" << patch_test::arr_to_str<>(data_size, data_) )
    // log(INFO, "put key= " << key << ", writing to file= data_put")
    // return_if_err(patch_test::write_arr_to_file(data_size, data_, "data_put"), err)
    
    if (++counter >= NUM_REP)
      break;
  }
  
  return 0;
}

int DSTest::repetitive_get()
{
  struct pthread_arg_struct arg_struct = get_thread_arg_struct_v.pop_back();
  int err;
  return_if_err(init(arg_struct.data_size), err)
  uint64_t data_size = (arg_struct.data_size == 0) ? MSG_SIZE : arg_struct.data_size;
  
  srand(time(NULL) );
  std::string base_key = arg_struct.base_key + patch_test::to_str(arg_struct.thread_id);
  int counter = 0;
  while (1) {
    // float s_time = abs(3*(float)(rand() % RANDOM_INT_RANGE)/RANDOM_INT_RANGE - (float)(rand() % RANDOM_INT_RANGE)/RANDOM_INT_RANGE);
    // sleep(s_time);
    // log(INFO, "slept for " << s_time)
    
    std::string key = base_key + "_" + patch_test::to_str(counter);
    DATA_T* data_to_get_ = (DATA_T*)malloc(data_size);
    return_if_err(ds_driver.get(key.c_str(), ver, sizeof(DATA_T), ndim, gdim_, lb_, ub_, data_to_get_), err)
    // return_if_err(ds_driver.del(key.c_str(), ver), err)
    if (data_size == MSG_SIZE)
      log(INFO, "got key= " << key << ", data= " << data_to_get_)
    else {
      log(INFO, "got key= " << key << ", data_size= " << float(data_size/1024/1024) << " MB.")
      // log(INFO, "got key= " << key << ", data_size= " << data_size << ", data_= \n" << patch_test::arr_to_str<>(data_size, data_to_get_) )
      // log(INFO, "got key= " << key << ", writing to file= data_got")
      // return_if_err(patch_test::write_arr_to_file(data_size, data_to_get_, "data_got"), err)
    }
    free(data_to_get_);
    
    if (++counter >= NUM_REP)
      break;
  }
  
  return 0;
}
// --------------------------------------------  run_*  ----------------------------------------- //
int DSTest::run_multithreaded_put_test(std::string base_key, uint64_t data_size)
{
  int err;
  for (int i = 0; i < num_putget_threads; i++) {
    pthread_arg_struct arg_struct;
    arg_struct.thread_id = i;
    arg_struct.base_key = base_key;
    arg_struct.data_size = data_size;
    
    put_thread_arg_struct_v.push_back(arg_struct);
    
    // log(INFO, "Enter to continue with run_multithreaded_put_test")
    // std::string temp;
    // getline(std::cin, temp);
    sleep(1); // simultaneous (or close-to-each-other) locking causes some locks to be halting
    
    wrap_DSTest* wrap_ = new wrap_DSTest(this);
    return_if_err(pthread_create(put_thread_ptr_vector[i], NULL, call_repetitive_put_w_wrap, wrap_), err)
  }
}

int DSTest::run_multithreaded_get_test(std::string base_key, uint64_t data_size)
{
  int err;
  for (int i = 0; i < num_putget_threads; i++) {
    pthread_arg_struct arg_struct;
    arg_struct.thread_id = i;
    arg_struct.base_key = base_key;
    arg_struct.data_size = data_size;
    
    get_thread_arg_struct_v.push_back(arg_struct);
    
    wrap_DSTest* wrap_ = new wrap_DSTest(this);
    return_if_err(pthread_create(get_thread_ptr_vector[i], NULL, call_repetitive_get_w_wrap, wrap_), err)
  }
}
