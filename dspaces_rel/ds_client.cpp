#include "ds_client.h"

/*******************************************  BCServer  *******************************************/
BCServer::BCServer(int app_id, int base_client_id, int num_client, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), base_client_id(base_client_id), num_client(num_client), msg_size(msg_size),
  base_comm_var_name(base_comm_var_name), f_cb(f_cb),
  ds_driver_ ( ds_driver_ )
{
  // 
  LOG(INFO) << "BCServer:: constructed;\n" << to_str();
}

BCServer::BCServer(int app_id, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), base_client_id(0), num_client(0), msg_size(msg_size),
  base_comm_var_name(base_comm_var_name), f_cb(f_cb),
  ds_driver_ ( ds_driver_ )
{
  // 
  LOG(INFO) << "BCServer:: constructed;" << to_str();
}

BCServer::~BCServer()
{
  // ds_driver_->finalize();
  // 
  LOG(INFO) << "BCServer:: destructed.";
}

std::string BCServer::to_str()
{
  std::stringstream ss;
  ss << "\t base_comm_var_name= " << base_comm_var_name << "\n"
     << "\t app_id= " << app_id << "\n"
     << "\t base_client_id= " << base_client_id << "\n"
     << "\t num_client= " << num_client << "\n";
  
  return ss.str();
}

void BCServer::init_listen_all()
{
  //Assume app_id of each client app is ordered as 1,2,...,num_client
  for (int i = 1; i <= num_client; i++)
    init_listen_client(base_client_id + i);
  // 
  LOG(INFO) << "init_listen_all:: done.";
}

void BCServer::init_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  // 
  ds_driver_->reg_cb_on_get(key, f_cb);
  ds_driver_->init_get_thread(key, msg_size);
  // 
  LOG(INFO) << "init_listen_client:: done for client_id= " << client_id;
}

void BCServer::reinit_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->init_get_thread(key, msg_size);
  // 
  LOG(INFO) << "reinit_listen_client:: done for client_id= " << client_id;
}

/*******************************************  BCClient  *******************************************/
BCClient::BCClient(int app_id, int max_msg_size,
                   std::string base_comm_var_name, 
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), max_msg_size(max_msg_size),
  base_comm_var_name(base_comm_var_name),
  ds_driver_(ds_driver_)
{
  comm_var_name = base_comm_var_name + boost::lexical_cast<std::string>(app_id);
  //ds_driver_->lock_on_write(comm_var_name.c_str() );
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
  std::string msg_str = msg_coder.encode(msg_map);
  
  int msg_size = msg_str.size();
  if (msg_size > max_msg_size) {
    LOG(ERROR) << "send:: msg_size= " << msg_size << " > max_msg_size= " << max_msg_size;
    return 1;
  }
  
  // 1 dimensional char array
  // uint64_t gdim = 0; //max_msg_size;
  // uint64_t lb = 0;
  // uint64_t ub = 0; //max_msg_size-1;
  uint64_t* gdim_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* lb_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* ub_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  for (int i = 0; i < 3; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  
  char *data_ = (char*)malloc(max_msg_size*sizeof(char) );
  strcpy(data_, msg_str.c_str() );
  for (int i = msg_size; i < max_msg_size - msg_size; i++)
    data_[i] = '\0';
  
  int result = ds_driver_->sync_put(comm_var_name.c_str(), 0, max_msg_size*sizeof(char), 3, gdim_, lb_, ub_, data_);
  // int result = ds_driver_->sync_put_without_lock(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  free(data_);
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  
  return result;
}

/*******************************************  RIManager  ******************************************/
/*
RIManager::RIManager(int app_id, int num_cnodes,
                     char id, std::string dht_lip, int dht_lport, std::string ipeer_dht_lip, int ipeer_dht_lport,
                     std::string wa_trans_protocol, std::string wa_laddr, std::string wa_gftp_lintf, std::string wa_gftp_lport, 
                     std::string tmpfs_dir, std::list<std::string> wa_ib_lport_list,
                     bool w_prefetch, size_t buffer_size, char* alphabet_, size_t alphabet_size, size_t context_size)
: app_id(app_id), num_cnodes(num_cnodes),
  id(id),
  wa_trans_protocol(wa_trans_protocol), wa_laddr(wa_laddr), wa_gftp_lintf(wa_gftp_lintf), wa_gftp_lport(wa_gftp_lport),
  tmpfs_dir(tmpfs_dir),
  sdm_node_(new SDMNode(id, dht_lip, dht_lport,
                        ipeer_dht_lip, ipeer_dht_lport, 
                        boost::bind(&RIManager::handle_rimsg, this, _1), boost::bind(&RIManager::handle_rcmsg, this, _1) ) ),
  ds_driver_(new DSDriver(app_id, num_cnodes) ),
  bc_server_(new BCServer(app_id, num_cnodes, APP_RIMANAGER_MAX_MSG_SIZE, "req_app_", 
                          boost::bind(&RIManager::handle_app_req, this, _1),
                          ds_driver_) ),
  rfp_manager_(new RFPManager(wa_trans_protocol, ds_driver_, wa_ib_lport_list, wa_gftp_lintf, wa_gftp_lport, tmpfs_dir) ),
  pbuffer_(new PBuffer(id, num_cnodes + 1, W_PPM, 
                       w_prefetch, boost::bind(&RIManager::handle_prefetch, this, _1, _2), boost::bind(&RIManager::handle_del, this, _1) ) ),
  signals(io_service, SIGINT)
{
  bc_server_->init_listen_all();
  
  signals.async_wait(boost::bind(&RIManager::close, this) );
  io_service.run();
  // 
  LOG(INFO) << "RIManager:: constructed; \n" << to_str();
}

RIManager::~RIManager()
{
  // sdm_node_->close();
  // 
  LOG(INFO) << "RIManager:: destructed.";
}

void RIManager::close()
{
  sdm_node_->close();
  ds_driver_->close();
  
  rq_syncer.close();
  rp_syncer.close();
  handle_rp_syncer.close();
  bget_syncer.close();
  rf_wa_get_syncer.close();
  rp_wa_get_syncer.close();
  being_fetched_syncer.close();
  gftp_bping_syncer.close();
  // 
  signals.async_wait(boost::bind(&RIManager::close, this) );
  LOG(INFO) << "close:: closed.";
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t id= " << id << "\n";
  ss << "\t num_cnodes= " << num_cnodes << "\n";
  ss << "\t app_id= " << app_id << "\n";
  ss << "\t wa_trans_protocol= " << wa_trans_protocol << "\n";
  ss << "\t wa_laddr= " << wa_laddr << "\n";
  ss << "\t wa_gftp_lport= " << wa_gftp_lport << "\n";
  ss << "\t sdm_node= \n" << sdm_node_->to_str() << "\n";
  
  return ss.str();
}

int RIManager::bcast_rq_table()
{
  // std::map<std::string, std::string> rq_table_map = rq_table.to_str_str_map();
  std::map<std::string, std::string> rq_table_map = rq_table.to_unmarked_str_str_map();
  rq_table.mark_all();
  
  rq_table_map["type"] = RI_RQTABLE;
  rq_table_map["id"] = id;
  
  return sdm_node_->broadcast_msg(SDM_RIMSG, rq_table_map);
}

// ----------------------------------------  handle app_req  ------------------------------------ //
void RIManager::handle_app_req(char* app_req)
{
  std::map<std::string, std::string> app_req_map = msg_coder.decode(app_req);
  // 
  int app_id = boost::lexical_cast<int>(app_req_map["app_id"]);
  app_id__bc_client_map[app_id] = boost::make_shared<BCClient>(app_id, num_cnodes, APP_RIMANAGER_MAX_MSG_SIZE, "reply_app_", ds_driver_);
  // 
  std::string type = app_req_map["type"];
  
  if (type.compare(GET) == 0)
    handle_get(false, app_id, app_req_map);
  else if (type.compare(BLOCKING_GET) == 0)
    handle_get(true, app_id, app_req_map);
  else if (type.compare(PUT) == 0) {
    handle_put(app_id, app_req_map);
    bc_server_->reinit_listen_client(app_id);
  }
  else
    LOG(ERROR) << "handle_app_req:: unknown type= " << type;
  // 
  // bc_server_->reinit_listen_client(app_id);
}

void RIManager::handle_get(bool blocking, int app_id, std::map<std::string, std::string> get_map)
{
  LOG(INFO) << "handle_get:: get_map= \n" << patch_sdm::map_to_str<>(get_map);
  
  std::string key = get_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(get_map["ver"] );
  // 
  std::map<std::string, std::string> reply_msg_map;
  if (blocking)
    reply_msg_map["type"] = BLOCKING_GET_REPLY;
  else
    reply_msg_map["type"] = GET_REPLY;
  reply_msg_map["key"] = key;
  reply_msg_map["ver"] = get_map["ver"];
  // 
  int p_id;
  std::string data_type;
  char ds_id;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
    if (remote_query(blocking, key, ver) ) {
      LOG(ERROR) << "handle_get:: remote_query failed!";
      ds_id = '?';
    }
    else {
      if (blocking) {
        key_ver_pair kv = std::make_pair(key, ver);
        bget_syncer.add_sync_point(kv, 1);
        bget_syncer.wait(kv);
        bget_syncer.del_sync_point(kv);
        
        ds_id = this->id;
      }
      else {
        if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
          LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> does not exist.";
          ds_id = '?';
        }
      }
    }
  }
  
  if (ds_id != '?') {
    pbuffer_->add_access(p_id, std::make_pair(key, ver) );
    
    if (ds_id == this->id)
      LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> exists in local ds_id= " << ds_id;
    else {
      // TODO: <key, ver> may be being prefetched, should check for it.
      key_ver_pair kv = std::make_pair(key, ver);
      if (key_ver_being_fetched_vector.contains(kv) ) {
        LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> is being fetched right now...";
        being_fetched_syncer.add_sync_point(kv, 1);
        being_fetched_syncer.wait(kv);
        being_fetched_syncer.del_sync_point(kv);
        
        ds_id = this->id;
      }
      else {
        LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> exists in remote ds_id= " << ds_id;
        if (remote_get(ds_id, get_map) ) {
          LOG(ERROR) << "handle_get:: remote_get failed for <key= " << key << ", ver= " << ver << ">.";
          ds_id = '?';
        }
        else
          ds_id = this->id;
      }
    }
  }
  rq_table.add_key_ver(key, ver, p_id, data_type, this->id, size, ndim, gdim_, lb_, ub_);
  // 
  reply_msg_map["ds_id"] = ds_id;
  app_id__bc_client_map[app_id]->send(reply_msg_map);
  bc_server_->reinit_listen_client(app_id);
  // 
  LOG(INFO) << "handle_get:: done; blocking= " << blocking << ", <key= " << key << ", ver= " << ver << ">.";
}

int RIManager::remote_get(char ds_id, std::map<std::string, std::string> r_get_map)
{
  LOG(INFO) << "remote_get:: started for <key= " << r_get_map["key"] << ", ver= " << r_get_map["ver"] << ">.";
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if(msg_coder.decode_msg_map(r_get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
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
  // patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
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
  // patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_put:: done for <key= " << put_map["key"] << ", ver= " << put_map["ver"] << ">.";
  // LOG(INFO) << "handle_put:: rq_table=\n" << rq_table.to_str();
}

void RIManager::handle_prefetch(char this_ds_id, key_ver_pair kv)
{
  LOG(INFO) << "HANDLE_PREFETCH:: STARTED for <key= " << kv.first << ", ver= " << kv.second << ">.";
  
  std::string key = kv.first;
  unsigned int ver = kv.second;
  // 
  int p_id;
  char ds_id;
  std::string data_type;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  // Note: <key, ver> should be in rq_table since pbuffer makes a prediction over the data readily put
  if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_prefetch:: rq_table.get_key_ver failed UNEXPECTEDLY!";
    return;
  }
  
  if (ds_id == this->id)
    LOG(INFO) << "handle_prefetch:: <key= " << key << ", ver= " << ver << "> ALREADY exists in local ds_id= " << ds_id;
  else {
    // TODO: <key, ver> may be being fetched, should check for it. This can happen when there is no gap between remote gets done by the user app.
    key_ver_pair kv = std::make_pair(key, ver);
    if (key_ver_being_fetched_vector.contains(kv) ) {
      LOG(INFO) << "handle_get:: <key= " << key << ", ver= " << ver << "> is being fetched right now...";
      being_fetched_syncer.add_sync_point(kv, 1);
      being_fetched_syncer.wait(kv);
      being_fetched_syncer.del_sync_point(kv);
    }
    else {
      LOG(INFO) << "handle_prefetch:: <key= " << key << ", ver= " << ver << "> exists in remote ds_id= " << ds_id;
      std::map<std::string, std::string> r_g_map;
      if (msg_coder.encode_msg_map(r_g_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
        LOG(ERROR) << "handle_prefetch:: msg_coder.encode_msg_map failed! for <key= " << key << ", ver= " << ver << ">.";
        return;
      }

      if (remote_get(ds_id, r_g_map) )
        LOG(ERROR) << "handle_prefetch:: remote_get failed for <key= " << key << ", ver= " << ver << ">.";
      else
        rq_table.add_key_ver(key, ver, p_id, data_type, this->id, size, ndim, gdim_, lb_, ub_);
    }
  }
  // 
  LOG(INFO) << "HANDLE_PREFETCH:: DONE for <key= " << key << ", ver= " << ver << ">.";
}

void RIManager::handle_del(key_ver_pair kv)
{
  // 
  LOG(INFO) << "handle_del:: done for <key= " << kv.first << ", ver= " << kv.second << ">.";
}

// -------------------------------------------  support  ---------------------------------------- //
void RIManager::handle_possible_remote_places(std::string key, unsigned int ver)
{
  LOG(INFO) << "handle_possible_remote_places:: started for <key= " << key << ", ver= " << ver << ">.";
  
  int num_rps = 0;
  char ds_id;
  while (!rs_table.pop_subscriber(key, ver, ds_id) ) {
    boost::thread t(&RIManager::remote_place, this, key, ver, ds_id);
    num_rps++;
  }
  
  if (num_rps) {
    key_ver_pair kv = std::make_pair(key, ver);
    handle_rp_syncer.add_sync_point(kv, num_rps);
    handle_rp_syncer.wait(kv);
    handle_rp_syncer.del_sync_point(kv);
  }
  // 
  LOG(INFO) << "handle_possible_remote_places:: done for <key= " << key << ", ver= " << ver << ">.";
}

// Note: a <key, ver> pair cannot be produced in multiple dataspaces
int RIManager::remote_query(bool subscribe, std::string key, unsigned int ver)
{
  LOG(INFO) << "remote_query:: started;";
  
  std::map<std::string, std::string> r_q_map;
  if (subscribe)
    r_q_map["type"] = RI_RBQUERY;
  else
    r_q_map["type"] = RI_RQUERY;
  r_q_map["key"] = key;
  r_q_map["ver"] = boost::lexical_cast<std::string>(ver);
  
  if (sdm_node_->broadcast_msg(SDM_RIMSG, r_q_map) ) {
    LOG(ERROR) << "remote_query:: sdm_node_->broadcast_msg failed!";
    return 1;
  }
  
  key_ver_pair kv = std::make_pair(key, ver);
  rq_syncer.add_sync_point(kv, sdm_node_->get_num_peers() );
  rq_syncer.wait(kv);
  rq_syncer.del_sync_point(kv);
  
  LOG(INFO) << "remote_query:: done.";
  return 0;
}

int RIManager::remote_subscribe(std::string key, unsigned int ver)
{
  LOG(INFO) << "remote_subscribe:: started for <key= " << key << ", ver= " << ver << ">";
  
  std::map<std::string, std::string> r_subs_map;
  r_subs_map["id"] = id;
  r_subs_map["key"] = key;
  r_subs_map["ver"] = boost::lexical_cast<std::string>(ver);
  
  if (sdm_node_->broadcast_msg(SDM_RIMSG, r_subs_map) ){
    LOG(ERROR) << "remote_subscribe:: sdm_node_->broadcast_msg failed!";
  }
  // 
  LOG(INFO) << "remote_subscribe:: done.";
  return 0;
}

int RIManager::remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map)
{
  LOG(INFO) << "remote_fetch:: started;";
  
  r_fetch_map["laddr"] = this->wa_laddr;
  r_fetch_map["lport"] = rfp_manager_->get_lport(); //Returns either next avail ib_lport or constant gftps_lport
  r_fetch_map["tmpfs_dir"] = this->tmpfs_dir;
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0)
    boost::thread t(&RIManager::handle_wa_get, this, "remote_fetch", r_fetch_map );
  
  r_fetch_map["type"] = RI_RFETCH;
  if (sdm_node_->send_msg(ds_id, SDM_RIMSG, r_fetch_map) ) {
    LOG(ERROR) << "remote_fetch:: sdm_node_->send_msg to to_id= " << ds_id << " failed!";
    return 1;
  }
  // TODO: sync will not have any affect for gridftp case since wa_get will return immediately
  std::string key = r_fetch_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(r_fetch_map["ver"] );
  key_ver_pair kv = std::make_pair(key, ver);
  rf_wa_get_syncer.add_sync_point(kv, 1);
  rf_wa_get_syncer.wait(kv);
  rf_wa_get_syncer.del_sync_point(kv);
#ifdef _GRIDFTP_
  if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
    int size, ndim;
    uint64_t *gdim_, *lb_, *ub_;
    std::string data_type;
    if (msg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      LOG(ERROR) << "remote_fetch:: msg_coder.decode_msg_map failed!";
      return 1;
    }
    if (rfp_manager_->gftpfile_read__ds_put(key, ver, size, ndim, gdim_, lb_, ub_) ) {
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      LOG(ERROR) << "remote_fetch:: rfp_manager_->gftpfile_read__ds_put failed!";
      return 1;
    }
    patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  }
#endif // _GRIDFTP_
  // 
  LOG(INFO) << "remote_fetch:: done.";
  return 0;
}

int RIManager::remote_place(std::string key, unsigned int ver, char to_id)
{
  LOG(INFO) << "remote_place:: started for <key= " << key << ", ver= " << ver << "> --> to_id= " << to_id;
  int p_id;
  char ds_id;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
    LOG(INFO) << "remote_place:: rq_table.get_key_ver failed!";
    // patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  }
  
  // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  std::map<std::string, std::string> r_place_map;
  r_place_map["type"] = RI_RPLACE;
  r_place_map["id"] = id;
  if (msg_coder.encode_msg_map(r_place_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_place:: encode_msg_map failed!";
    return 1;
  }
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0) {
    if (sdm_node_->send_msg(to_id, SDM_RIMSG, r_place_map) ) {
      LOG(ERROR) << "remote_place:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      return 1;
    }
    
    key_ver_pair kv = std::make_pair(key, ver);
    rp_syncer.add_sync_point(kv, 1 );
    rp_syncer.wait(kv);
    rp_syncer.del_sync_point(kv);
    
    laddr_lport__tmpfsdir_pair ll_t = key_ver___laddr_lport__tmpfsdir_map[kv];
    if (rfp_manager_->wa_put(key, ver, data_type, size, ndim, gdim_, lb_, ub_, ll_t.first.first, ll_t.first.second, ll_t.second) ) {
      LOG(ERROR) << "remote_place:: rfp_manager_->wa_put failed!";
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      return 1;
    }
    handle_rp_syncer.notify(kv);
  }
  else if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
    if (!gftpb_table.contains(to_id) ) {
      gftp_bping(to_id);
    }
    std::string laddr, lport, tmpfs_dir;
    if (gftpb_table.get(to_id, laddr, lport, tmpfs_dir) ) {
      LOG(ERROR) << "remote_place:: gftpb_table.get failed!";
      return 1;
    }
    
    if (rfp_manager_->wa_put(key, ver, "", size, ndim, gdim_, lb_, ub_, laddr, lport, tmpfs_dir) ) {
      LOG(ERROR) << "remote_place:: rfp_manager_->wa_put failed!";
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      return 1;
    }
    if (sdm_node_->send_msg(to_id, SDM_RIMSG, r_place_map) ) {
      LOG(ERROR) << "remote_place:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      return 1;
    }
  }
  // 
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  LOG(INFO) << "remote_place:: done.";
  return 0;
}

void RIManager::handle_wa_get(std::string called_from, std::map<std::string, std::string> str_str_map)
{
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(str_str_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "handle_wa_get:: decode_msg_map failed!";
    return;
  }
  // 
  // LOG(INFO) << "handle_wa_get:: patch_ds::debug_print=";
  // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  // 
  if (rfp_manager_->wa_get(str_str_map["laddr"], str_str_map["lport"], str_str_map["tmpfs_dir"],
                           key, ver, data_type, 
                           size, ndim, gdim_, lb_, ub_) )
    LOG(ERROR) << "handle_wa_get:: rfp_manager_->wa_get failed!";
  
  if (called_from.compare("remote_fetch") == 0)
    rf_wa_get_syncer.notify(std::make_pair(key, ver) );
  else if (called_from.compare("handle_rplace") == 0)
    rp_wa_get_syncer.notify(std::make_pair(key, ver) );
  
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
}

int RIManager::gftp_bping(char to_id)
{
  LOG(INFO) << "gftp_bping:: started for to_id= " << to_id;
  
  std::map<std::string, std::string> gftp_bping_map;
  gftp_bping_map["type"] = RI_GFTP_BPING;
  gftp_bping_map["id"] = this->id;
  
  if (sdm_node_->send_msg(to_id, SDM_RIMSG, gftp_bping_map) ) {
    LOG(ERROR) << "gftp_bping:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
    return 1;
  }
  gftp_bping_syncer.add_sync_point(to_id, 1 );
  gftp_bping_syncer.wait(to_id);
  gftp_bping_syncer.del_sync_point(to_id);
  // 
  LOG(INFO) << "gftp_bping:: done.";
  return 0;
}

// ------------------------------------  handle wamsg  ------------------------------------------ //
void RIManager::handle_rimsg(std::map<std::string, std::string> rimsg_map)
{
  std::string type = rimsg_map["type"];
  
  if (type.compare(RI_RQUERY) == 0)
    handle_rquery(false, rimsg_map);
  else if (type.compare(RI_RBQUERY) == 0)
    handle_rquery(true, rimsg_map);
  else if (type.compare(RI_RQUERY_REPLY) == 0)
    handle_rq_reply(rimsg_map);
  else if (type.compare(RI_RFETCH) == 0)
    handle_rfetch(rimsg_map);
  else if (type.compare(RI_RQTABLE) == 0)
    handle_rqtable(rimsg_map);
  else if (type.compare(RI_RPLACE) == 0)
    handle_rplace(rimsg_map);
  else if (type.compare(RI_RPLACE_REPLY) == 0)
    handle_rp_reply(rimsg_map);
#ifdef _GRIDFTP_
  else if (type.compare(RI_GFTPPUT_DONE) == 0)
    handle_gftpput_done(rimsg_map);
  else if (type.compare(RI_GFTP_BPING) == 0)
    handle_gftp_bping(rimsg_map);
  else if (type.compare(RI_GFTP_BPONG) == 0)
    handle_gftp_bpong(rimsg_map);
#endif // _GRIDFTP_
  else
    LOG(ERROR) << "handle_rimsg:: unknown type= " << type;
  // 
  LOG(INFO) << "handle_rimsg:: done.";
}

void RIManager::handle_rquery(bool subscribe, std::map<std::string, std::string> r_query_map)
{
  LOG(INFO) << "handle_rquery:: r_query_map= \n" << patch_sdm::map_to_str<>(r_query_map);
  
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
  LOG(INFO) << "handle_rq_reply:: rq_reply_map= \n" << patch_sdm::map_to_str<>(rq_reply_map);
  
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

void RIManager::handle_rfetch(std::map<std::string, std::string> r_fetch_map)
{
  LOG(INFO) << "handle_rfetch:: r_fetch_map= \n" << patch_sdm::map_to_str<>(r_fetch_map);
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
    LOG(ERROR) << "handle_rfetch:: decode_msg_map failed!";
    return;
  }
  
  if (rfp_manager_->wa_put(key, ver, data_type, size, ndim, gdim_, lb_, ub_,
                            r_fetch_map["laddr"], r_fetch_map["lport"], r_fetch_map["tmpfs_dir"] ) ) {
    LOG(ERROR) << "handle_rfetch:: rfp_manager_->wa_put failed!";
  }
  // 
#ifdef _GRIDFTP_
  if (wa_trans_protocol.compare(GRIDFTP) == 0) {
    char to_id = r_fetch_map["id"].c_str()[0];
    std::map<std::string, std::string> gftp_put_done_map;
    gftp_put_done_map["type"] = RI_GFTPPUT_DONE;
    gftp_put_done_map["id"] = to_id;
    gftp_put_done_map["key"] = key;
    gftp_put_done_map["ver"] = boost::lexical_cast<std::string>(ver);
    if (sdm_node_->send_msg(to_id, SDM_RIMSG, gftp_put_done_map) ) {
      LOG(ERROR) << "handle_rfetch:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
      patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
      return;
    }
  }
#endif // _GRIDFTP_
  // 
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  LOG(INFO) << "handle_rfetch:: done.";
}

void RIManager::handle_rqtable(std::map<std::string, std::string> r_rqtable_map)
{
  LOG(INFO) << "handle_rqtable:: r_rqtable_map= \n" << patch_sdm::map_to_str<>(r_rqtable_map);
  
  int count = 0;
  while (1) {
    std::string tail_str = boost::lexical_cast<std::string>(count);
    std::string key_str = "key_" + tail_str;
    if (r_rqtable_map.count(key_str) == 0)
      break;
    
    int ndim = boost::lexical_cast<int>(r_rqtable_map["ndim_" + tail_str] );
    uint64_t* gdim_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    uint64_t* lb_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    uint64_t* ub_ = (uint64_t*)malloc(ndim*sizeof(uint64_t) );
    
    
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > gdim_tokens(r_rqtable_map["gdim_" + tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator gdim_tokens_it = gdim_tokens.begin();
    boost::tokenizer<boost::char_separator<char> > lb_tokens(r_rqtable_map["lb_" + tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator lb_tokens_it = lb_tokens.begin();
    boost::tokenizer<boost::char_separator<char> > ub_tokens(r_rqtable_map["ub_" + tail_str], sep);
    boost::tokenizer<boost::char_separator<char> >::iterator ub_tokens_it = ub_tokens.begin();
    for (int i = 0; i < ndim; i++, gdim_tokens_it++, lb_tokens_it++, ub_tokens_it++) {
      gdim_[i] = boost::lexical_cast<uint64_t>(*gdim_tokens_it);
      lb_[i] = boost::lexical_cast<uint64_t>(*lb_tokens_it);
      ub_[i] = boost::lexical_cast<uint64_t>(*ub_tokens_it);
    }
    std::string key = r_rqtable_map[key_str];
    unsigned int ver= boost::lexical_cast<unsigned int>(r_rqtable_map["ver_" + tail_str] );
    int p_id = boost::lexical_cast<int>(r_rqtable_map["p_id_" + tail_str] );
    rq_table.put_key_ver(key, ver,
                         p_id, r_rqtable_map["data_type_" + tail_str], r_rqtable_map["ds_id_" + tail_str].c_str()[0],
                         boost::lexical_cast<int>(r_rqtable_map["size_" + tail_str] ),
                         ndim, gdim_, lb_, ub_ );
    // patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
    pbuffer_->reg_key_ver(p_id, std::make_pair(key, ver) );
    
    count++;
  }
  // 
  LOG(INFO) << "handle_rq_reply:: done.";
}

void RIManager::handle_rplace(std::map<std::string, std::string> r_place_map)
{
  LOG(INFO) << "handle_rplace:: r_place_map= \n" << patch_sdm::map_to_str<>(r_place_map);
  
  std::string key = r_place_map["key"];
  unsigned int ver= boost::lexical_cast<unsigned int>(r_place_map["ver"] );
  
  std::string lport = rfp_manager_->get_lport();
  r_place_map["laddr"] = wa_laddr;
  r_place_map["lport"] = lport;
  boost::thread t(&RIManager::handle_wa_get, this, "handle_rplace", r_place_map);
  
  if (this->wa_trans_protocol.compare(INFINIBAND) == 0) {
    std::map<std::string, std::string> rp_reply_map;
    rp_reply_map["type"] = RI_RPLACE_REPLY;
    rp_reply_map["key"] = key;
    rp_reply_map["ver"] = r_place_map["ver"];
    rp_reply_map["laddr"] = wa_laddr;
    rp_reply_map["lport"] = lport;
    char to_id = r_place_map["id"].c_str()[0];
    if (sdm_node_->send_msg(to_id, SDM_RIMSG, rp_reply_map) ) {
      LOG(ERROR) << "handle_rplace:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
      return;
    }
  }
  // 
  key_ver_pair kv = std::make_pair(key, ver);
  rp_wa_get_syncer.add_sync_point(kv, 1);
  rp_wa_get_syncer.wait(kv);
  rp_wa_get_syncer.del_sync_point(kv);
  
  int p_id;
  std::string data_type;
  char ds_id;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if (rq_table.get_key_ver(key, ver, p_id, data_type, ds_id, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_rplace:: rq_table.get_key_ver failed for <key= " << key << ", ver= " << ver << ">.";
    return;
  }
  rq_table.put_key_ver(key, ver, p_id, data_type, this->id, size, ndim, gdim_, lb_, ub_);
  pbuffer_->reg_key_ver(p_id, std::make_pair(key, ver) );
  
  // TODO: Following may throw exception in case of non-blocking get
  bget_syncer.notify(kv);
  LOG(INFO) << "handle_rplace:: done.";
  
  
  // if (this->wa_trans_protocol.compare(GRIDFTP) == 0) {
  //   int size, ndim;
  //   uint64_t *gdim_, *lb_, *ub_;
  //   std::string data_type;
  //   if (msg_coder.decode_msg_map(r_fetch_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
  //     patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  //     LOG(ERROR) << "remote_fetch:: msg_coder.decode_msg_map failed!";
  //     return 1;
  //   }
  //   if (rfp_manager_->gftpfile_read__ds_put(key, ver, size, ndim, gdim_, lb_, ub_) ) {
  //     patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  //     LOG(ERROR) << "remote_fetch:: rfp_manager_->gftpfile_read__ds_put failed!";
  //     return 1;
  //   }
  //   patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  // }
}

void RIManager::handle_rp_reply(std::map<std::string, std::string> rp_reply_map)
{
  LOG(INFO) << "handle_rp_reply:: rp_reply_map= \n" << patch_sdm::map_to_str<>(rp_reply_map);
  
  key_ver_pair kv = std::make_pair(rp_reply_map["key"], boost::lexical_cast<unsigned int>(rp_reply_map["ver"]) );
  key_ver___laddr_lport__tmpfsdir_map[kv] = std::make_pair(std::make_pair(rp_reply_map["laddr"], rp_reply_map["lport"]), rp_reply_map["tmpfs_dir"]);
  
  rp_syncer.notify(kv);
  // 
  LOG(INFO) << "handle_rp_reply:: done.";
}

void RIManager::handle_r_subscribe(std::map<std::string, std::string> r_subs_map)
{
  LOG(INFO) << "handle_r_subscribe:: r_subs_map= \n" << patch_sdm::map_to_str<>(r_subs_map);
  
  rs_table.push_subscriber(r_subs_map["key"], boost::lexical_cast<unsigned int>(r_subs_map["ver"]), 
                           boost::lexical_cast<char>(r_subs_map["id"]) );
  // 
  LOG(INFO) << "handle_r_subscribe:: done.";
}

#ifdef _GRIDFTP_
void RIManager::handle_gftpput_done(std::map<std::string, std::string> gftpput_done_map)
{
  LOG(INFO) << "handle_gftpput_done:: gftpput_done_map= \n" << patch_sdm::map_to_str<>(gftpput_done_map);
  
  rf_wa_get_syncer.notify(std::make_pair(gftpput_done_map["key"], boost::lexical_cast<unsigned int>(gftpput_done_map["ver"] ) ) );
  // 
  LOG(INFO) << "handle_gftpput_done:: done.";
}

void RIManager::handle_gftp_bping(std::map<std::string, std::string> gftp_bping_map)
{
  LOG(INFO) << "handle_gftp_bping:: gftp_bping_map= \n" << patch_sdm::map_to_str<>(gftp_bping_map);
  
  std::map<std::string, std::string> gftp_bpong_map;
  gftp_bpong_map["type"] = RI_GFTP_BPONG;
  gftp_bpong_map["id"] = this->id;
  gftp_bpong_map["laddr"] = this->wa_laddr;
  gftp_bpong_map["lport"] = this->wa_gftp_lport;
  gftp_bpong_map["tmpfs_dir"] = this->tmpfs_dir;
  
  char to_id = gftp_bping_map["id"].c_str()[0];
  if (sdm_node_->send_msg(to_id, SDM_RIMSG, gftp_bpong_map) ) {
    LOG(ERROR) << "handle_gftp_bping:: sdm_node_->send_msg to to_id= " << to_id << " failed!";
    return;
  }
  // 
  LOG(INFO) << "handle_gftp_bping:: done.";
}

void RIManager::handle_gftp_bpong(std::map<std::string, std::string> gftp_bpong_map)
{
  LOG(INFO) << "handle_gftp_bpong:: gftp_bpong_map= \n" << patch_sdm::map_to_str<>(gftp_bpong_map);
  
  char ds_id = gftp_bpong_map["id"].c_str()[0];
  gftpb_table.add(ds_id, gftp_bpong_map["laddr"], gftp_bpong_map["lport"], gftp_bpong_map["tmpfs_dir"] );
  gftp_bping_syncer.notify(ds_id);
  // 
  LOG(INFO) << "handle_gftp_bpong:: done.";
}
#endif // _GRIDFTP_

void RIManager::handle_rcmsg(std::map<std::string, std::string> rcmsg_map)
{
  
}
*/
