#include "ds_client.h"

//************************************  RIManager  ********************************//
RIManager::RIManager(char id, int num_cnodes, int app_id)
: ds_driver_ ( new DSpacesDriver(num_cnodes, app_id) ),
  num_cnodes(num_cnodes),
  app_id(app_id)
{
  //
  LOG(INFO) << "RIManager:: constructed.";
}

RIManager::~RIManager()
{
  //
  LOG(INFO) << "RIManager:: destructed.";
}

void RIManager::handle_ri_req(char* ri_req)
{
  //
  LOG(INFO) << "handle_ri_req:: ri_req= " << ri_req;
}

void RIManager::init_listen_ri_req(int peer_id)
{
  std::string var_name = "ri_req_" + boost::lexical_cast<std::string>(peer_id);
  
  function_cb_on_ri_req fp_handle_ri_req = boost::bind(&RIManager::handle_ri_req, this, _1);
  ds_driver_->reg_cb_on_get(var_name, fp_handle_ri_req);
  ds_driver_->init_riget_thread(var_name, RI_MSG_SIZE);
  
  //
  LOG(INFO) << "init_listen_ri_req:: done for peer_id= " << peer_id;
}

//************************************  TestClient  *******************************//
void debug_print(const char* var_name, unsigned int ver, int datasize, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "debug_print::";
  std::cout << "var_name= " << var_name << "\n"
            << "ver= " << ver << "\n"
            << "datasize= " << datasize << "\n"
            << "ndim= " << ndim << "\n";
  std::cout << "gdim=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << gdim[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "lb=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << lb[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "ub=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << ub[i] << ", ";
  }
  std::cout << "\n";
  
  //
  if (data == NULL){
    return;
  }
  
  std::cout << "data=";
  for (int i=0; i<datasize; i++){
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}

TestClient::TestClient(int num_cnodes, int app_id)
: ds_driver_ ( new DSpacesDriver(num_cnodes, app_id) ),
  num_cnodes(num_cnodes),
  app_id(app_id)
{
  lock_ri_var(app_id);
  //
  LOG(INFO) << "TestClient:: constructed.";
}

TestClient::~TestClient()
{
  //
  LOG(INFO) << "TestClient:: destructed.";
}

float TestClient::get_avg(int size, int* data)
{
  float sum = 0;
  for (int i=0; i<size; i++){
    sum += data[i];
  }
  
  return sum / (float) size;
}

#define TEST_SIZE 5
#define TEST_NDIM 1
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 1
#define TEST_SGDIM 10

void TestClient::put_test()
{
  LOG(INFO) << "put_test:: started.";
  //
  
  //generics
  const char* var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  for (int i=0; i<TEST_DATASIZE; i++){
    data[i] = (i+1);
  }
  
  LOG(INFO) << "put_test:: before put;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg = " << get_avg(TEST_DATASIZE, data) << "\n";
  
  int result = ds_driver_->sync_put(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "put_test:: sync_put returned " << result;
  
  LOG(INFO) << "put_test:: after put;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg= " << get_avg(TEST_DATASIZE, data) << "\n";
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  //
  LOG(INFO) << "put_test:: done.";
}

void TestClient::get_test()
{
  LOG(INFO) << "get_test:: started.";
  //
  
  //generics
  const char* var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  //
  LOG(INFO) << "get_test:: before get;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg= " << get_avg(TEST_DATASIZE, data) << "\n";
  
  int result = ds_driver_->get(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "get_test:: get returned " << result;
  
  LOG(INFO) << "get_test:: after get;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg = " << get_avg(TEST_DATASIZE, data) << "\n";
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  //
  LOG(INFO) << "get_test:: done.";
}

void TestClient::block()
{
  LOG(INFO) << "block:: blocking.";
  ds_driver_->lock_on_write("dummy_lock");
  LOG(INFO) << "block:: done.";
}

void TestClient::unblock()
{
  LOG(INFO) << "unblock:: unblocking.";
  ds_driver_->unlock_on_write("dummy_lock");
  LOG(INFO) << "unblock:: done.";
}

void TestClient::wait()
{
  LOG(INFO) << "wait:: waiting.";
  ds_driver_->lock_on_read("dummy_lock");
  ds_driver_->unlock_on_read("dummy_lock");
  //ds_driver_->lock_on_write("dummy_lock");
  //ds_driver_->unlock_on_write("dummy_lock");
  LOG(INFO) << "wait:: done.";
}

void TestClient::lock_ri_var(int peer_id)
{
  std::string var_name = "ri_req_" + boost::lexical_cast<std::string>(peer_id);
  ds_driver_->lock_on_write(var_name.c_str() );
}

void TestClient::put_ri_msg(std::string ri_msg)
{
  int msg_size = ri_msg.size();
  if (msg_size > RI_MSG_SIZE){
    LOG(ERROR) << "put_ri_msg:: msg_size= " << msg_size << " > RI_MSG_SIZE= " << RI_MSG_SIZE;
    return;
  }
  usleep(2*1000*1000);
  
  std::string var_name = "ri_req_" + boost::lexical_cast<std::string>(app_id);
  //1 dimensional char array
  uint64_t gdim = RI_MSG_SIZE;
  uint64_t lb = 0;
  uint64_t ub = RI_MSG_SIZE-1;
  
  char *data_ = (char*)malloc(RI_MSG_SIZE*sizeof(char));
  strcpy(data_, ri_msg.c_str() );
  for (int i=msg_size; i<RI_MSG_SIZE-msg_size; i++){
    data_[i] = '\0';
  }
  int result = ds_driver_->sync_put_without_lock(var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  LOG(INFO) << "put_ri_msg:: sync_put_without_lock returned " << result;
  
  free(data_);
}