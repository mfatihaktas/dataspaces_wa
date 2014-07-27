#include "ds_client.h"

void debug_print(std::string var_name, unsigned int ver, int size, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "debug_print::";
  std::cout << "var_name= " << var_name << "\n"
            << "ver= " << ver << "\n"
            << "size= " << size << "\n"
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
  for (int i=0; i<size; i++){
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}

void print_str_map(std::map<std::string, std::string> str_map)
{
  for (std::map<std::string, std::string>::const_iterator it=str_map.begin(); 
       it!=str_map.end(); ++it){
    std::cout << "\t" << it->first << ":" << it->second << "\n";
  }
}

//***********************************  IMsgCoder  ********************************//
IMsgCoder::IMsgCoder()
{
  //
  LOG(INFO) << "IMsgCoder:: constructed.";
}

IMsgCoder::~IMsgCoder()
{
  //
  LOG(INFO) << "IMsgCoder:: destructed.";
}

std::map<std::string, std::string> IMsgCoder::decode(char* msg)
{
  //msg: serialized std::map<std::string, std::string>
  std::map<std::string, std::string> msg_map;
  
  std::stringstream ss;
  ss << msg;
  boost::archive::text_iarchive ia(ss);
  ia >> msg_map;
  //
  //LOG(INFO) << "decode:: msg_map= ";
  //print_str_map(msg_map);
  return msg_map;
}

int IMsgCoder::decode_i_msg(std::map<std::string, std::string> msg_map,
                            std::string& var_name, unsigned int& ver, int& size,
                            int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  try
  {
    var_name = msg_map["var_name"];
    ver = boost::lexical_cast<unsigned int>(msg_map["ver"]);
    size = boost::lexical_cast<int>(msg_map["size"]);
    ndim = boost::lexical_cast<int>(msg_map["ndim"]);
    
    boost::char_separator<char> sep(",");
    
    boost::tokenizer<boost::char_separator<char> > gdim_tokens(msg_map["gdim"], sep);
    boost::tokenizer<boost::char_separator<char> > lb_tokens(msg_map["lb"], sep);
    boost::tokenizer<boost::char_separator<char> > ub_tokens(msg_map["ub"], sep);
    
    uint64_t *t_gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    uint64_t *t_lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    uint64_t *t_ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t));
    int i;
    boost::tokenizer<boost::char_separator<char> >::iterator gdim_it = gdim_tokens.begin();
    boost::tokenizer<boost::char_separator<char> >::iterator lb_it = lb_tokens.begin();
    boost::tokenizer<boost::char_separator<char> >::iterator ub_it = ub_tokens.begin();
    for (int i=0; i<ndim; i++)
    {
      t_gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_it);
      t_lb_[i] = boost::lexical_cast<uint64_t>(*lb_it);
      t_ub_[i] = boost::lexical_cast<uint64_t>(*ub_it);
      
      gdim_it++;
      lb_it++;
      ub_it++;
    }
    gdim_ = t_gdim_;
    lb_ = t_lb_;
    ub_ = t_ub_;
  }
  catch(std::exception & ex)
  {
    LOG(ERROR) << "decode_i_msg:: Exception=" << ex.what();
    return 1;
  }
  
  return 0;
}

std::string IMsgCoder::encode(std::map<std::string, std::string> msg_map)
{
  std::stringstream ss;
  boost::archive::text_oarchive oa(ss);
  oa << msg_map;
  
  return ss.str();
}
//************************************  BCServer  *********************************//
BCServer::BCServer(int app_id, int num_clients, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSpacesDriver> ds_driver_)
: app_id(app_id),
  num_clients(num_clients),
  msg_size(msg_size),
  base_comm_var_name(base_comm_var_name),
  f_cb(f_cb),
  ds_driver_ ( ds_driver_ )
{
  //
  LOG(INFO) << "BCServer:: constructed for comm_var_name= " << base_comm_var_name;
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
BCClient::BCClient(int app_id, int num_others, int max_msg_size,
                   std::string base_comm_var_name, 
                   boost::shared_ptr<DSpacesDriver> ds_driver_)
: app_id(app_id),
  num_others(num_others),
  max_msg_size(max_msg_size),
  base_comm_var_name(base_comm_var_name),
  ds_driver_ ( ds_driver_ )
{
  comm_var_name = base_comm_var_name + boost::lexical_cast<std::string>(app_id);
  ds_driver_->lock_on_write(comm_var_name.c_str() );
  //
  LOG(INFO) << "BCClient:: constructed for comm_var_name= " << base_comm_var_name;
}

BCClient::~BCClient()
{
  ds_driver_->finalize();
  //
  LOG(INFO) << "BCClient:: destructed.";
}

int BCClient::send(std::map<std::string, std::string> msg_map)
{
  std::string msg_str = imsg_coder.encode(msg_map);
  
  int msg_size = msg_str.size();
  if (msg_size > max_msg_size){
    LOG(ERROR) << "send:: msg_size= " << msg_size << " > max_msg_size= " << max_msg_size;
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

bool RQTable::get_key(std::string key, char& ds_id,
                      unsigned int& ver, int& size, int& ndim, 
                      uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  if (!key_dsid_map.count(key) ){
    return false;
  }
  ds_id = key_dsid_map[key];
  
  if (ver != 0)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_datainfo_map[key];
    ver = (unsigned int)datainfo_map["ver"].back();
    size = (int)datainfo_map["size"].back();
    ndim = (int)datainfo_map["ndim"].back();
    for(int i=0; i<ndim; i++){
      gdim_[i] = datainfo_map["gdim"][i];
      lb_[i] = datainfo_map["lb"][i];
      ub_[i] = datainfo_map["ub"][i];
    }
  }
  
  return true;
}

int RQTable::put_key(std::string key, char ds_id, 
                     unsigned int ver, int size, int ndim, 
                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  if (key_dsid_map.count(key) ){
    return update_key(key, ds_id, ver, size, ndim, gdim_, lb_, ub_);
  }
  return add_key(key, ds_id, ver, size, ndim, gdim_, lb_, ub_);
}

int RQTable::add_key(std::string key, char ds_id, 
                     unsigned int ver, int size, int ndim, 
                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  key_dsid_map[key] = ds_id;
  
  if (ver != 0)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map;
    
    std::vector<uint64_t> ver_v;
    ver_v.push_back(ver);
    datainfo_map["ver"] = ver_v;
    
    std::vector<uint64_t> size_v;
    size_v.push_back((uint64_t)size);
    datainfo_map["size"] = size_v;
    
    std::vector<uint64_t> ndim_v;
    ndim_v.push_back((uint64_t)ndim);
    datainfo_map["ndim"] = ndim_v;
    
    std::vector<uint64_t> gdim_v;
    std::vector<uint64_t> lb_v;
    std::vector<uint64_t> ub_v;
    for(int i=0; i<ndim; i++){
      gdim_v.push_back(gdim_[i]);
      lb_v.push_back(lb_[i]);
      ub_v.push_back(ub_[i]);
    }
    datainfo_map["gdim"] = gdim_v;
    datainfo_map["lb"] = lb_v;
    datainfo_map["ub"] = ub_v;
    //
    key_datainfo_map[key] = datainfo_map;
  }
  //
  LOG(INFO) << "add_key:: added <" << key << ", " << ds_id << ">";
  return 0;
}

int RQTable::update_key(std::string key, char ds_id, 
                        unsigned int ver, int size, int ndim, 
                        uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  del_key(key);
  add_key(key, ds_id, ver, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "update_key:: updated <" << key << ", " << ds_id << ">";
  return 0;
}

int RQTable::del_key(std::string key)
{
  if (!key_dsid_map.count(key) ){
    LOG(ERROR) << "del_key:: non-existing key=" << key;
    return 1;
  }
  key_dsid_map_it = key_dsid_map.find(key);
  key_dsid_map.erase(key_dsid_map_it);
  
  key_datainfo_map_it = key_datainfo_map.find(key);
  key_datainfo_map.erase(key_datainfo_map_it);
  //
  LOG(INFO) << "del_key:: deleted key=" << key;
  return 0;
}

std::string RQTable::to_str()
{
  std::stringstream ss;
  for (key_dsid_map_it=key_dsid_map.begin(); key_dsid_map_it!=key_dsid_map.end(); ++key_dsid_map_it){
    std::string key = key_dsid_map_it->first;
    ss << "key=" << key << "\n";
    ss << "\tds_id=" << key_dsid_map_it->second << "\n";

    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_datainfo_map[key];
    ss << "\tver=" << datainfo_map["ver"].back() << "\n";
    ss << "\tsize=" << datainfo_map["size"].back() << "\n";
    int ndim = (int)datainfo_map["ndim"].back();
    std::string gdim = "";
    std::string lb = "";
    std::string ub = "";
    for(int i=0; i<ndim; i++){
      gdim += boost::lexical_cast<std::string>(datainfo_map["gdim"][i]);
      lb += boost::lexical_cast<std::string>(datainfo_map["lb"][i]);
      ub += boost::lexical_cast<std::string>(datainfo_map["ub"][i]);
      if (i < ndim-1){
        gdim += ",";
        lb += ",";
        ub += ",";
      }
    }
    ss << "\tndim=" << ndim << "\n";
    ss << "\tgdim=" << gdim << "\n";
    ss << "\tlb=" << lb << "\n";
    ss << "\tub=" << ub << "\n";
    ss << "\n";
  }
  
  return ss.str();
}
//************************************  RIManager  ********************************//
RIManager::RIManager(char id, int num_cnodes, int app_id,
                     char* lip, int lport, char* ipeer_lip, int ipeer_lport)
: id(id),
  num_cnodes(num_cnodes),
  app_id(app_id),
  ds_driver_ ( new DSpacesDriver(num_cnodes, app_id) ),
  li_bc_server_(new BCServer(app_id, num_cnodes, LI_MAX_MSG_SIZE, "li_req_", 
                             boost::bind(&RIManager::handle_li_req, this, _1),
                             ds_driver_) ),
  ri_bc_server_(new BCServer(app_id, num_cnodes, RI_MAX_MSG_SIZE, "ri_req_", 
                             boost::bind(&RIManager::handle_ri_req, this, _1),
                             ds_driver_) ),
  dht_node_(new DHTNode(id, boost::bind(&RIManager::handle_wamsg, this, _1),
                        lip, lport, 
                        ipeer_lip, ipeer_lport) )
{
  usleep(WAIT_TIME_FOR_BCCLIENT_DSLOCK);
  
  li_bc_server_->init_listen_all();
  ri_bc_server_->init_listen_all();
  //
  LOG(INFO) << "RIManager:: constructed.\n" << to_str();
}

RIManager::~RIManager()
{
  //
  LOG(INFO) << "RIManager:: destructed.";
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t num_cnodes=" << num_cnodes << "\n";
  ss << "\t app_id=" << app_id << "\n";
  ss << "\t dht_node=\n" << dht_node_->to_str() << "\n";
  //
  return ss.str();
}
/********* handle li_req *********/
void RIManager::handle_li_req(char* li_req)
{
  std::map<std::string, std::string> li_req_map = imsg_coder.decode(li_req);
  //
  int app_id = boost::lexical_cast<int>(li_req_map["app_id"]);
  li_bc_server_->reinit_listen_client(app_id);
  
  std::string type = li_req_map["type"];
  
  if (type.compare("l_get") == 0){
    //
  }
  else if (type.compare("l_put") == 0){
    handle_l_put(li_req_map);
  }
  else{
    LOG(ERROR) << "handle_li_req:: unknown type= " << type;
  }
}

void RIManager::handle_l_put(std::map<std::string, std::string> l_put_map)
{
  LOG(INFO) << "handle_l_put:: l_put_map=";
  print_str_map(l_put_map);
  
  std::string var_name;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if(imsg_coder.decode_i_msg(l_put_map, var_name, ver, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_l_put:: decode_i_msg failed!";
  }
  //debug_print(var_name, ver, size, ndim, gdim_, lb_, ub_, NULL);
  rq_table.put_key(var_name, this->id, ver, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "handle_l_put:: done.";
  LOG(INFO) << "handle_l_put:: rq_table=\n" << rq_table.to_str();
}
/********* handle ri_req *********/
void RIManager::handle_ri_req(char* ri_req)
{
  std::map<std::string, std::string> ri_req_map = imsg_coder.decode(ri_req);
  //
  int app_id = boost::lexical_cast<int>(ri_req_map["app_id"]);
  ri_bc_server_->reinit_listen_client(app_id);
  
  std::string type = ri_req_map["type"];
  
  if (type.compare("r_get") == 0){
    handle_r_get(ri_req_map);
  }
  else if (type.compare("r_put") == 0){
    //
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
  
}

void RIManager::handle_r_get(std::map<std::string, std::string> r_get_map)
{
  LOG(INFO) << "handle_r_get:: r_get_map=";
  print_str_map(r_get_map);
  
  //
  LOG(INFO) << "handle_r_get:: done.";
}
/********* handle wamsg *********/
void RIManager::handle_wamsg(std::map<std::string, std::string> wamsg_map)
{
  LOG(INFO) << "handle_wamsg:: wamsg_map=";
  print_str_map(wamsg_map);
  
  //
  LOG(INFO) << "handle_wamsg:: done.";
}
//************************************  TestClient  *******************************//
/*
void debug_print(const char* var_name, unsigned int ver, int size, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "debug_print::";
  std::cout << "var_name= " << var_name << "\n"
            << "ver= " << ver << "\n"
            << "size= " << size << "\n"
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
  for (int i=0; i<size; i++){
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
  if (ri_msg_size > RI_MAX_MSG_SIZE){
    LOG(ERROR) << "put_ri_msg:: ri_msg_size= " << ri_msg_size << " > RI_MAX_MSG_SIZE= " << RI_MAX_MSG_SIZE;
    return;
  }
  //usleep(2*1000*1000);
  
  std::string var_name = "ri_req_" + boost::lexical_cast<std::string>(app_id);
  //1 dimensional char array
  uint64_t gdim = RI_MAX_MSG_SIZE;
  uint64_t lb = 0;
  uint64_t ub = RI_MAX_MSG_SIZE-1;
  
  char *data_ = (char*)malloc(RI_MAX_MSG_SIZE*sizeof(char));
  strcpy(data_, ri_msg_str.c_str() );
  for (int i=ri_msg_size; i<RI_MAX_MSG_SIZE-ri_msg_size; i++){
    data_[i] = '\0';
  }
  int result = ds_driver_->sync_put_without_lock(var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  LOG(INFO) << "put_ri_msg:: sync_put_without_lock returned " << result;
  
  free(data_);
  
  lock_ri_var(app_id);
}
*/
