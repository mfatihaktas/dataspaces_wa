#include "ds_client.h"

//************************************   RQTable   ********************************//
RQTable::RQTable()
{
  //
  LOG(INFO) << "RQTable:: constructed.";
}

RQTable::~RQTable()
{
  //
  LOG(INFO) << "RQTable:: deconstructed.";
}

bool RQTable::query(std::string key, char& ds_id)
{
  if (key_dsid_map.count(key) ){
    return false;
  }
  
  ds_id = key_dsid_map[key];
  return true;
}

int RQTable::add_key_dsid_pair(std::string key, char ds_id)
{
  if (key_dsid_map.count(key) ){
    LOG(ERROR) << "add_key_dsid_pair:: already added key=" << key;
    return 1;
  }
  key_dsid_map[key] = ds_id;
  //
  LOG(INFO) << "add_key_dsid_pair:: added pair <" << key << ", " << ds_id << ">";
  return 0;
}

int RQTable::update_entry(std::string key, char ds_id)
{
  if (!key_dsid_map.count(key) ){
    LOG(ERROR) << "update_entry:: non-existing key=" << key;
    return 1;
  }
  key_dsid_map[key] = ds_id;
  //
  LOG(INFO) << "update_entry:: updated pair <" << key << ", " << ds_id << ">";
  return 0;
}

int RQTable::del_key(std::string key)
{
  if (!key_dsid_map.count(key) ){
    LOG(ERROR) << "del_key:: non-existing key=" << key;
    return 1;
  }
  std::map<std::string, char>::iterator it;
  it=key_dsid_map.find(key);
  key_dsid_map.erase (it);
  //
  LOG(INFO) << "del_key:: deleted key=" << key;
  return 0;
}
//************************************  BCServer  *********************************//
BCServer::BCServer(int app_id, int num_clients, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb)
: app_id(app_id),
  num_clients(num_clients),
  msg_size(msg_size),
  base_comm_var_name(base_comm_var_name),
  f_cb(f_cb),
  ds_driver_ ( new DSpacesDriver(num_clients, app_id) )
{
  //
  LOG(INFO) << "BCServer:: constructed.";
}

BCServer::~BCServer()
{
  ds_driver_->finalize();
  //
  LOG(INFO) << "BCServer:: destructed.";
}

void BCServer::init_listen_client(int client_id)
{
  std::string var_name = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->reg_cb_on_get(var_name, f_cb);
  ds_driver_->init_riget_thread(var_name, msg_size);
  //
  LOG(INFO) << "init_listen_client:: done for client_id= " << client_id;
}

void BCServer::init_listen_all()
{
  //Assume app_id of each client app is ordered as 1,2,...,num_clients
  for(int i=1; i<=num_clients; i++){
    init_listen_client(i);
  }
  //
  LOG(INFO) << "init_listen_all:: done.";
}

void BCServer::reinit_listen_client(int client_id)
{
  std::string var_name = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->init_riget_thread(var_name, msg_size);
  //
  LOG(INFO) << "reinit_listen_client:: done for client_id= " << client_id;
}

//************************************  BCClient   ********************************//
BCClient::BCClient(int app_id, int num_others, int msg_size,
                   std::string base_comm_var_name)
: app_id(app_id),
  num_others(num_others),
  max_msg_size(max_msg_size),
  base_comm_var_name(base_comm_var_name),
  ds_driver_ ( new DSpacesDriver(num_others, app_id) )
{
  comm_var_name = base_comm_var_name + boost::lexical_cast<std::string>(app_id);
  ds_driver_->lock_on_write(comm_var_name.c_str() );
  //
  LOG(INFO) << "BCClient:: constructed.";
}

BCClient::~BCClient()
{
  ds_driver_->finalize();
  //
  LOG(INFO) << "BCClient:: destructed.";
}

int BCClient::send(std::string type, std::string msg)
{
  std::map<std::string, std::string> msg_map;
  msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  msg_map["type"] = type;
  
  if (type.compare("r_get") == 0){
    msg_map["key"] = msg;
  }
  else if (type.compare("r_put") == 0){
  }
  else{
    LOG(ERROR) << "send:: unknown type= " << type;
    return 1;
  }
  //
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << msg_map;
  std::string msg_str(ss.str());
  
  int msg_size = msg_str.size();
  if (msg_size > max_msg_size){
    LOG(ERROR) << "put_ri_msg:: msg_size= " << msg_size << " > max_msg_size= " << max_msg_size;
    return 1;
  }
  
  //1 dimensional char array
  uint64_t gdim = max_msg_size;
  uint64_t lb = 0;
  uint64_t ub = max_msg_size-1;
  
  char *data_ = (char*)malloc(max_msg_size*sizeof(char));
  strcpy(data_, msg_str.c_str() );
  for (int i=msg_size; i<max_msg_size-msg_size; i++){
    data_[i] = '\0';
  }
  int result = ds_driver_->sync_put_without_lock(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  //LOG(INFO) << "send:: sync_put_without_lock returned " << result;
  free(data_);
  
  ds_driver_->lock_on_write(comm_var_name.c_str() );
  
  return 0;
}
//************************************  RIManager  ********************************//
RIManager::RIManager(char id, int num_cnodes, int app_id)
: num_cnodes(num_cnodes),
  app_id(app_id),
  bc_server_(new BCServer(app_id, num_cnodes, RI_MSG_SIZE, "ri_req_", 
                          boost::bind(&RIManager::handle_ri_req, this, _1) ) )
{
  bc_server_->init_listen_all();
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
  //ri_req: serialized std::map<std::string, std::string>
  std::map<std::string, std::string> ri_req_map;
  
  std::stringstream ss;
  ss << ri_req;
  boost::archive::text_iarchive ia(ss);
  ia >> ri_req_map;
  //
  LOG(INFO) << "handle_ri_req:: ri_req_map= ";
  for (std::map<std::string, std::string>::const_iterator it=ri_req_map.begin(); it!=ri_req_map.end(); ++it){
    std::cout << "\t" << it->first << ":" << it->second << "\n";
  }
  
  int app_id = boost::lexical_cast<int>(ri_req_map["app_id"]);
  bc_server_->reinit_listen_client(app_id);
  
  std::string type = ri_req_map["type"];
  
  if (type.compare("r_get") == 0){
    handle_r_get(ri_req_map["key"]);
  }
  else if (type.compare("r_put") == 0){
    //
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
}

void RIManager::handle_r_get(std::string key)
{
  //
  LOG(INFO) << "handle_r_get:: key= " << key;
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

void TestClient::put_ri_msg(std::string type, std::string msg)
{
  std::map<std::string, std::string> ri_msg_map;
  
  ri_msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  ri_msg_map["type"] = type;
  
  if (type.compare("r_get") == 0){
    ri_msg_map["key"] = msg;
  }
  else if (type.compare("r_put") == 0){
    //
  }
  else{
    LOG(ERROR) << "put_ri_msg:: unknown type= " << type;
  }
  //
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << ri_msg_map;
  std::string ri_msg_str(ss.str());
  
  
  int ri_msg_size = ri_msg_str.size();
  if (ri_msg_size > RI_MSG_SIZE){
    LOG(ERROR) << "put_ri_msg:: ri_msg_size= " << ri_msg_size << " > RI_MSG_SIZE= " << RI_MSG_SIZE;
    return;
  }
  //usleep(2*1000*1000);
  
  std::string var_name = "ri_req_" + boost::lexical_cast<std::string>(app_id);
  //1 dimensional char array
  uint64_t gdim = RI_MSG_SIZE;
  uint64_t lb = 0;
  uint64_t ub = RI_MSG_SIZE-1;
  
  char *data_ = (char*)malloc(RI_MSG_SIZE*sizeof(char));
  strcpy(data_, ri_msg_str.c_str() );
  for (int i=ri_msg_size; i<RI_MSG_SIZE-ri_msg_size; i++){
    data_[i] = '\0';
  }
  int result = ds_driver_->sync_put_without_lock(var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  LOG(INFO) << "put_ri_msg:: sync_put_without_lock returned " << result;
  
  free(data_);
  
  lock_ri_var(app_id);
}
