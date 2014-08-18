#include "ds_client.h"

void debug_print(std::string key, unsigned int ver, int size, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "debug_print::";
  std::cout << "key= " << key << "\n"
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
                            std::string& key, unsigned int& ver, int& size,
                            int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_)
{
  try
  {
    key = msg_map["key"];
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
  //ds_driver_->finalize();
  //
  LOG(INFO) << "BCServer:: destructed.";
}

void BCServer::init_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  //
  ds_driver_->reg_cb_on_get(key, f_cb);
  ds_driver_->init_riget_thread(key, msg_size);
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
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->init_riget_thread(key, msg_size);
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
  LOG(INFO) << "BCClient:: constructed for comm_var_name= " << comm_var_name;
}

BCClient::~BCClient()
{
  //ds_driver_->finalize();
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
                      unsigned int* ver, int* size, int* ndim, 
                      uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  if (!key_dsid_map.count(key) ){
    return false;
  }
  ds_id = key_dsid_map[key];
  
  if (ver != NULL)
  {
    std::map<std::string, std::vector<uint64_t> > datainfo_map = key_datainfo_map[key];
    *ver = (unsigned int)datainfo_map["ver"].back();
    *size = (int)datainfo_map["size"].back();
    *ndim = (int)datainfo_map["ndim"].back();
    for(int i=0; i<*ndim; i++){
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
//************************************   syncer  **********************************//
syncer::syncer()
{
  LOG(INFO) << "syncer:: constructed.";
}

syncer::~syncer()
{
  LOG(INFO) << "syncer:: destructed.";
}

int syncer::add_sync_point(std::string key, int num_peers)
{
  if (key_cv_map.count(key) ){
    LOG(ERROR) << "add_sync_point:: already added key=" << key;
    return 1;
  }
  boost::shared_ptr<boost::condition_variable> t_cv_( new boost::condition_variable() );
  boost::shared_ptr<boost::mutex> t_m_( new boost::mutex() );
  
  key_cv_map[key] = t_cv_;
  key_m_map[key] = t_m_;
  key_numpeers_map[key] = num_peers;
  
  return 0;
}

int syncer::del_sync_point(std::string key)
{
  if (!key_cv_map.count(key) ){
    LOG(ERROR) << "del_sync_point:: non-existing key=" << key;
    return 1;
  }
  key_cv_map_it = key_cv_map.find(key);
  key_cv_map.erase(key_cv_map_it);
  
  key_m_map_it = key_m_map.find(key);
  key_m_map.erase(key_m_map_it);
  
  key_numpeers_map_it = key_numpeers_map.find(key);
  key_numpeers_map.erase(key_numpeers_map_it);
  
  return 0;
}

int syncer::wait(std::string key)
{
  boost::mutex::scoped_lock lock(*key_m_map[key]);
  key_cv_map[key]->wait(lock);
  
  return 0;
}

int syncer::notify(std::string key)
{
  int num_peers_to_wait = key_numpeers_map[key];
  --num_peers_to_wait;
  
  if (num_peers_to_wait == 0){
    key_cv_map[key]->notify_one();
    return 0;
  }
  key_numpeers_map[key] = num_peers_to_wait;
  
  return 0;
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
                             ds_driver_) )
{
  usleep(WAIT_TIME_FOR_BCCLIENT_DSLOCK);
  
  li_bc_server_->init_listen_all();
  ri_bc_server_->init_listen_all();
  //to avoid problem with bc_server and bc_client sync_with_time
  boost::shared_ptr<DHTNode> t_sp_(
    new DHTNode(id, boost::bind(&RIManager::handle_wamsg, this, _1),
                lip, lport, ipeer_lip, ipeer_lport) 
  );
  this->dht_node_ = t_sp_;
  //
  LOG(INFO) << "RIManager:: constructed.\n" << to_str();
}

RIManager::~RIManager()
{
  //dht_node_->close();
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
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if(imsg_coder.decode_i_msg(l_put_map, key, ver, size, ndim, gdim_, lb_, ub_) ){
    LOG(ERROR) << "handle_l_put:: decode_i_msg failed!";
  }
  //debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL);
  rq_table.put_key(key, this->id, ver, size, ndim, gdim_, lb_, ub_);
  //
  LOG(INFO) << "handle_l_put:: done.";
  LOG(INFO) << "handle_l_put:: rq_table=\n" << rq_table.to_str();
}
/********* handle ri_req *********/
void RIManager::handle_ri_req(char* ri_req)
{
  std::map<std::string, std::string> ri_req_map = imsg_coder.decode(ri_req);
  LOG(INFO) << "handle_ri_req:: ri_req_map=";
  print_str_map(ri_req_map);
  //
  int app_id = boost::lexical_cast<int>(ri_req_map["app_id"]);
  ri_bc_server_->reinit_listen_client(app_id);
  //
  //usleep(100*1000);
  boost::shared_ptr<BCClient> bc_client_(
    new BCClient(app_id, num_cnodes, RI_MAX_MSG_SIZE, "ri_reply_", ds_driver_)
  );
  appid_bcclient_map[app_id] = bc_client_;
  //
  std::string type = ri_req_map["type"];
  
  if (type.compare("r_get") == 0){
    handle_r_get(app_id, ri_req_map);
  }
  else if (type.compare("r_put") == 0){
    //
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
  
}

//Dor now assuming r_get will only be used for remote data
//TODO: a global get which will return local or remote data based on where data is
void RIManager::handle_r_get(int app_id, std::map<std::string, std::string> r_get_map)
{
  LOG(INFO) << "handle_r_get:: r_get_map=";
  print_str_map(r_get_map);
  
  std::string key = r_get_map["key"];
  
  std::map<std::string, std::string> msg_map;
  msg_map["key"] = key;
  
  char ds_id;
  if (!rq_table.get_key(key, ds_id, NULL, NULL, NULL, NULL, NULL, NULL) ){
    if (!remote_query(key) ){
      LOG(ERROR) << "handle_r_get:: remote_query failed!";
    }
    if (!rq_table.get_key(key, ds_id, NULL, NULL, NULL, NULL, NULL, NULL) ){
      LOG(INFO) << "handle_r_get:: key= " << key << " does not exist.";
      //
      msg_map["ds_id"] = '?';
    }
    else{
      LOG(INFO) << "handle_r_get:: key= " << key << " exists in ds_id= " << ds_id << ".";
      //
      msg_map["ds_id"] = ds_id;
    }
  }
  else{
    LOG(INFO) << "handle_r_get:: key= " << key << " exists in ds_id= " << ds_id << ".";
    //
    msg_map["ds_id"] = ds_id;
  }
  /*
  std::cout << "handle_r_get:: ;;;\n";
  usleep(10*1000*1000);
  std::cout << "handle_r_get:: ...\n";
  
  appid_bcclient_map[app_id]->send(msg_map);
  */
  //
  LOG(INFO) << "handle_r_get:: done.";
}

//PI: a key cannot be produced in multiple dataspaces
bool RIManager::remote_query(std::string key)
{
  LOG(INFO) << "remote_query:: started;";
  
  std::map<std::string, std::string> r_q_map;
  r_q_map["type"] = "r_query";
  r_q_map["key"] = key;
  
  if (broadcast_msg(RIMSG, r_q_map) ){
    LOG(ERROR) << "remote_query:: broadcast_msg failed!";
    return false;
  }
  
  rq_syncer.add_sync_point(key, dht_node_->get_num_peers() );
  rq_syncer.wait(key);
  rq_syncer.del_sync_point(key);
  
  LOG(INFO) << "remote_query:: done.";
  return true;
}

int RIManager::broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->broadcast_msg(msg_type, msg_map);
}

int RIManager::send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map)
{
  return dht_node_->send_msg(ds_id, msg_type, msg_map);
}
/********* handle wamsg *********/
void RIManager::handle_wamsg(std::map<std::string, std::string> wamsg_map)
{
  std::string type = wamsg_map["type"];
  
  if (type.compare("r_query") == 0){
    handle_r_query(wamsg_map);
  }
  else if (type.compare("rq_reply") == 0){
    handle_rq_reply(wamsg_map);
  }
  else{
    LOG(ERROR) << "handle_ri_req:: unknown type= " << type;
  }
  
  //
  LOG(INFO) << "handle_wamsg:: done.";
}

void RIManager::handle_r_query(std::map<std::string, std::string> r_query_map)
{
  //LOG(INFO) << "handle_r_query:: r_query_map=";
  //print_str_map(r_query_map);
  
  std::string key = r_query_map["key"];
  
  std::map<std::string, std::string> rq_reply_map;
  rq_reply_map["type"] = "rq_reply";
  rq_reply_map["key"] = key;
  
  char to_id = r_query_map["id"].c_str()[0];
  char ds_id;
  if(!rq_table.get_key(key, ds_id, NULL, NULL, NULL, NULL, NULL, NULL) ){
    //LOG(INFO) << "handle_r_query:: does not exist; key= " << key;
    rq_reply_map["ds_id"] = '?';
  }
  else{
    rq_reply_map["ds_id"] = ds_id;
  }
  
  //LOG(INFO) << "handle_r_query:: rq_reply_map=";
  //print_str_map(rq_reply_map);
  
  if(send_msg(to_id, RIMSG, rq_reply_map) ){
    LOG(ERROR) << "handle_r_query:: send_msg to to_id= " << to_id << " failed!";
  }
  //
  LOG(INFO) << "handle_r_query:: done.";
}

void RIManager::handle_rq_reply(std::map<std::string, std::string> rq_reply_map)
{
  LOG(INFO) << "handle_rq_reply:: rq_reply_map=";
  print_str_map(rq_reply_map);
  
  std::string key = rq_reply_map["key"];
  char ds_id = rq_reply_map["ds_id"].c_str()[0];
  if (ds_id != '?'){
    rq_table.put_key(key, ds_id, 0, 0, 0, NULL, NULL, NULL);
  }
  
  rq_syncer.notify(key);
  //
  LOG(INFO) << "handle_rq_reply:: done.";
}
