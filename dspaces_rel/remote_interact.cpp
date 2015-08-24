#include "remote_interact.h"

/********************************************  RFPManager  ****************************************/
RFPManager::RFPManager(char data_id_t, std::string trans_protocol,
                       std::string ib_laddr, std::list<std::string> ib_lport_list,
                       std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir,
                       boost::shared_ptr<DSpacesDriver> ds_driver_)
: data_id_t(data_id_t), trans_protocol(trans_protocol),
  t_manager_(boost::make_shared<TManager>(trans_protocol, 
                                          ib_laddr, ib_lport_list,
                                          gftp_lintf, gftp_laddr, gftp_lport, tmpfs_dir) ),
  ds_driver_(ds_driver_)
{
  // 
  LOG(INFO) << "RFPManager:: constructed.";
}

RFPManager::~RFPManager() { LOG(INFO) << "RFPManager:: destructed."; }

int RFPManager::wa_put(std::string laddr, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "wa_put:: started laddr= " << laddr << ", lport= " << lport << "; \n"
            << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  int data_length = get_data_length(ndim, gdim_, lb_, ub_);
  if (data_length == 0) {
    LOG(ERROR) << "wa_put:: data_length = 0!";
    return 1;
  }
  void* data_ = malloc(size*data_length);
  // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "wa_put:: ds_driver_->get for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  std::string data_id = patch_sfc::get_data_id(data_id_t, key, ver, lb_, ub_);
  if (t_manager_->init_put(laddr, lport, tmpfs_dir, data_id, data_type, data_length, data_) ) {
    LOG(ERROR) << "wa_put:: t_manager_->init_put failed for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  free(data_);
  // 
  LOG(INFO) << "wa_put:: done for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  return 0;
}

int RFPManager::wa_get(std::string laddr, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "wa_get:: started laddr= " << laddr << ", lport= " << lport << "; \n"
            << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  
  std::string data_id = patch_sfc::get_data_id(data_id_t, key, ver, lb_, ub_);
  
  data_id__recved_size_map[data_id] = 0;
  data_id__data_map[data_id] = malloc(size*get_data_length(ndim, gdim_, lb_, ub_) );
  
  if (t_manager_->init_get(lport, data_id, data_type, boost::bind(&RFPManager::handle_recv, this, _1, _2, _3) ) ) {
    LOG(ERROR) << "wa_get:: t_manager_->init_get failed for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_id__data_map[data_id] ) ) {
    LOG(ERROR) << "wa_get:: ds_driver_->sync_put failed for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  free(data_id__data_map[data_id] );
  
  data_id__data_map.erase(data_id__data_map.find(data_id) );
  data_id__recved_size_map.erase(data_id__recved_size_map.find(data_id) );
  // 
  LOG(INFO) << "wa_get:: done for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  
  return 0;
}

void RFPManager::handle_recv(std::string data_id, int data_length, void* data_)
{
  if (!data_id__recved_size_map.count(data_id) ) {
    LOG(ERROR) << "handle_recv:: data is received for an data_id= " << data_id;
    return;
  }
  
  int recved_size = data_id__recved_size_map[data_id];
  LOG(INFO) << "handle_recv:: for data_id= " << data_id
            << ", recved data_size= " << data_size
            << ", total_recved_size= " << (float)(recved_size + data_size)/1024/1024 << "(MB)";
  
  char* data_t_ = static_cast<char*>(key_ver__data_map[kv] );
  memcpy(data_t_ + recved_size, data_, data_size);
  
  data_id__recved_size_map[data_id] += data_size;
}

int RFPManager::get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for (int i = 0; i < ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i] ) {
      LOG(ERROR) << "get_data_length:: lb= " << lb << " is not feasible!";
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb) {
      LOG(ERROR) << "get_data_length:: ub= " << ub << " is not feasible!";
      return 0;
    }
    dim_length[i] = ub - lb + 1;
  }
  
  int volume = 1;
  for (int i = 0; i < ndim; i++) {
    volume *= (size_t)dim_length[i];
  
  return volume;
}

/********************************************  RIManager  ****************************************/
RIManager::RIManager(int cl_id, int num_client, char data_id_t,
                     char ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
                     std::string data_trans_protocol, std::string ib_laddr, std::list<std::string> ib_lport_list,
                     std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir)
: cl_id(cl_id), num_client(num_client),
  ds_id(ds_id),
  sdm_slave_(boost::make_shared<SDMSlave>(data_id_t, boost::bind(RIManager::handle_dm_act, this, _1),
                                          ds_id, control_lip, control_lport, join_control_lip, join_control_lport,
                                          boost::bind(RIManager::handle_ri_msg, this, _1) ) ),
  ds_driver_(new DSpacesDriver(cl_id, num_cnodes) ),
  bc_server_(boost::make_shared<BCServer>(cl_id, num_cnodes, APP_RIMANAGER_MAX_MSG_SIZE, "req_app_", 
                                          boost::bind(&RIManager::handle_app_req, this, _1), ds_driver_) ),
  rfp_manager_(boost::make_shared<RFPManager>(LUCOOR_DATA_ID, data_trans_protocol,
                                              ib_laddr, ib_lport_list,
                                              gftp_lintf, gftp_laddr, gftp_lport, tmpfs_dir, ds_driver_),
  signals(io_service, SIGINT)
{
  bc_server_->init_listen_all();
  
  signals.async_wait(boost::bind(&RIManager::close, this) );
  io_service.run();
  // 
  LOG(INFO) << "RIManager:: constructed; \n" << to_str();
}

RIManager::~RIManager() { LOG(INFO) << "RIManager:: destructed."; }

void RIManager::close()
{
  LOG(INFO) << "close:: closing...";
  sdm_slave_->close();
  ds_driver_->close();
  ri_man_syncer.close();
  // 
  signals.async_wait(boost::bind(&RIManager::close, this) );
  LOG(INFO) << "close:: done.";
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t cl_id= " << cl_id << "\n"
     << "\t num_client= " << num_client << "\n"
     << "\t ds_id= " << ds_id << "\n"
     << "\t sdm_slave= \n" << sdm_slave_->to_str() << "\n"
     << "\t rfp_manager= \n" << rfp_manager_->to_str() << "\n";
  
  return ss.str();
}

// ----------------------------------------  handle app_req  ------------------------------------ //
void RIManager::handle_app_req(char* app_req)
{
  std::map<std::string, std::string> app_req_map = msg_coder.decode(app_req);
  // 
  int cl_id = boost::lexical_cast<int>(app_req_map["cl_id"] );
  cl_id__bc_client_map[cl_id] = boost::make_shared<BCClient>(cl_id, num_client, CL__RIMANAGER_MAX_MSG_SIZE, "reply_app_", ds_driver_);
  // 
  std::string type = app_req_map["type"];
  
  if (str_str_equals(type, GET) )
    handle_get(false, cl_id, app_req_map);
  else if (str_str_equals(type, BLOCKING_GET) )
    handle_get(true, cl_id, app_req_map);
  else if (str_str_equals(type, PUT) ) {
    handle_put(cl_id, app_req_map);
    // bc_server_->reinit_listen_client(cl_id);
  }
  else
    LOG(ERROR) << "handle_app_req:: unknown type= " << type;
  // 
  bc_server_->reinit_listen_client(cl_id);
}

void RIManager::handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map)
{
  LOG(INFO) << "handle_get:: get_map= \n" << patch_sfc::map_to_str<>(get_map);
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_get:: msg_coder.decode_msg_map failed! get_map= \n" << patch_sfc::map_to_str<>(get_map);
    return;
  }
  
  if (blocking)
    get_map["type"] = BLOCKING_GET_REPLY;
  else
    get_map["type"] = GET_REPLY;
  
  if (sdm_slave_->get(blocking, key, ver, lb_, ub_) ) {
    LOG(INFO) << "handle_get:: does not exist " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    get_map["ds_id"] = '?';
  }
  else
    get_map["ds_id"] = ds_id;
  // 
  cl_id__bc_client_map[cl_id]->send(reply_msg_map);
  bc_server_->reinit_listen_client(cl_id);
}

int RIManager::remote_get(char ds_id, std::map<std::string, std::string> r_get_map)
{
  LOG(INFO) << "remote_get:: started for <key= " << r_get_map["key"] << ", ver= " << r_get_map["ver"] << ">.";
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(r_get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_get:: decode_msg_map failed!";
    return 1;
  }
  
  key_ver_pair kv = std::make_pair(key, ver);
  rget_time_profiler.add_event(kv, ("<" + key + ", " + boost::lexical_cast<std::string>(ver) + ">") );
  key_ver_being_fetched_vector.push_back(kv);
  
  if (remote_fetch(ds_id, r_get_map) ) {
    LOG(INFO) << "remote_get:: remote_fetch failed!";
    key_ver_being_fetched_vector.del(kv);
    return 1;
  }
  
  key_ver_being_fetched_vector.del(kv);
  being_fetched_syncer.notify(kv);
  rget_time_profiler.end_event(kv);
  LOG(INFO) << "remote_get:: rget_time_profiler= \n" << rget_time_profiler.to_str();
  // 
  // patch_ds::free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "remote_get:: done for <key= " << r_get_map["key"] << ", ver= " << r_get_map["ver"] << ">.";
  return 0;
}

void RIManager::handle_put(int p_id, std::map<std::string, std::string> put_map)
{
  LOG(INFO) << "handle_put:: started for <key= " << put_map["key"] << ", ver= " << put_map["ver"] << ">.";
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(put_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) )
    LOG(ERROR) << "handle_l_put:: msg_coder.decode_msg_map failed!";
  else {
    boost::thread t(&RIManager::handle_possible_remote_places, this, key, ver);
    
    // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL);
    rq_table.put_key_ver(key, ver, p_id, data_type, this->id, size, ndim, gdim_, lb_, ub_);
    pbuffer_->reg_key_ver(p_id, std::make_pair(key, ver) );
    
    if (bcast_rq_table() )
      LOG(ERROR) << "handle_l_put:: bcast_rq_table failed!";
  }
  // 
  // patch_ds::free_all<uint64_t*>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_put:: done for <key= " << put_map["key"] << ", ver= " << put_map["ver"] << ">.";
  // LOG(INFO) << "handle_put:: rq_table=\n" << rq_table.to_str();
}

void RIManager::handle_del(key_ver_pair kv)
{
  // 
  LOG(INFO) << "handle_del:: done for <key= " << kv.first << ", ver= " << kv.second << ">.";
}

// ------------------------------------  handle rimsg  ------------------------------------------ //
void RIManager::handle_rimsg(std::map<std::string, std::string> ri_msg_map)
{
  std::string type = ri_msg_map["type"];
  
  if (str_str_equals(type, RI_RQUERY) )
    handle_rquery(false, ri_msg_map);
  else
    LOG(ERROR) << "handle_rimsg:: unknown type= " << type;
  // 
  LOG(INFO) << "handle_rimsg:: done.";
}

void RIManager::handle_rquery(bool subscribe, std::map<std::string, std::string> r_query_map)
{
  LOG(INFO) << "handle_rquery:: r_query_map= \n" << patch_sfc::map_to_str<>(r_query_map);
  
  std::string key = r_query_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(r_query_map["ver"] );
  
  std::map<std::string, std::string> rq_reply_map;
  rq_reply_map["type"] = RI_RQUERY_REPLY;
  rq_reply_map["key"] = key;
  rq_reply_map["ver"] = r_query_map["ver"];
  
  char to_id = r_query_map["id"].c_str()[0];
  int p_id;
  std::string data_type;
  char ds_id;
  int dummy = -1;
  uint64_t* dummy_;
  if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, dummy, dummy, dummy_, dummy_, dummy_) ) {
    // LOG(INFO) << "handle_rquery:: does not exist; key= " << key;
    rq_reply_map["ds_id"] = '?';
    
    if (subscribe) {
      rs_table.push_subscriber(key, ver, to_id);
      LOG(INFO) << "handle_rquery:: id= " << to_id << " got subscribed for <key= " << key << ", ver= " << ver << ">";
    }
  }
  else {
    rq_reply_map["p_id"] = boost::lexical_cast<std::string>(p_id);
    rq_reply_map["p_id"] = data_type;
    rq_reply_map["ds_id"] = ds_id;
  }
  
  if (sdm_node_->send_msg(to_id, SDM_RIMSG, rq_reply_map) )
    LOG(ERROR) << "handle_rquery:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
  // 
  LOG(INFO) << "handle_rquery:: done.";
}

void RIManager::handle_rq_reply(std::map<std::string, std::string> rq_reply_map)
{
  LOG(INFO) << "handle_rq_reply:: rq_reply_map= \n" << patch_sfc::map_to_str<>(rq_reply_map);
  
  std::string key = rq_reply_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(rq_reply_map["ver"] );
  char ds_id = rq_reply_map["ds_id"].c_str()[0];
  if (ds_id != '?') {
    int p_id = boost::lexical_cast<int>(rq_reply_map["p_id"] );
    std::string data_type = rq_reply_map["data_type"];
    rq_table.put_key_ver(key, ver, p_id, data_type, ds_id, 0, 0, NULL, NULL, NULL);
    pbuffer_->reg_key_ver(p_id, std::make_pair(key, ver) );
  }
  
  rq_syncer.notify(std::make_pair(key, ver) );
  // 
  LOG(INFO) << "handle_rq_reply:: done.";
}


void RIManager::handle_rcmsg(std::map<std::string, std::string> rcmsg_map)
{
  
}
