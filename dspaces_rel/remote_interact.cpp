#include "remote_interact.h"

/********************************************  RFPManager  ****************************************/
RFPManager::RFPManager(DATA_ID_T data_id_t, std::string trans_protocol,
                       std::string ib_lip, std::list<std::string> ib_lport_list,
                       std::string tcp_lip, int tcp_lport,
                       std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir,
                       boost::shared_ptr<DSDriver> ds_driver_)
: data_id_t(data_id_t),
  trans_(boost::make_shared<Trans>(trans_protocol, 
                                   ib_lip, ib_lport_list,
                                   tcp_lip, tcp_lport,
                                   gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir) ),
  ds_driver_(ds_driver_)
{
  // 
  log_(INFO, "constructed.")
}

RFPManager::~RFPManager() { log_(INFO, "destructed.") }

std::string RFPManager::to_str()
{
  std::stringstream ss;
  ss << "data_id_t= " << data_id_t << "\n"
     << "trans_= \n" << trans_->to_str() << "\n";
  
  return ss.str();
}

std::string RFPManager::get_lip() { return trans_->get_s_lip(); }
std::string RFPManager::get_lport() { return trans_->get_s_lport(); }
std::string RFPManager::get_tmpfs_dir() { return trans_->get_tmpfs_dir(); }

int RFPManager::wa_put(std::string lip, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  int err;
  log_(INFO, "started; lip= " << lip << ", lport= " << lport << "; \n"
             << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  int data_length = DSDriver::get_data_length(ndim, gdim_, lb_, ub_);
  if (data_length == 0) {
    log_(ERROR, "data_length = 0!")
    return 1;
  }
  void* data_ = malloc(size*data_length);
  // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  
  // return_if_err(ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_), err, 3,
  //               log_(ERROR, "ds_driver_->get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) ) )
  // try_n_times__return_if_err(ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_), err, 3,
  //                           log_(ERROR, "ds_driver_->get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) ) )
  // try_n_times__return_if_err(ds_driver_->reg_get_wait_for_completion(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_), err, 3,
  //                           log_(ERROR, "ds_driver_->reg_get_wait_for_completion failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) ) )
  return_if_err(ds_driver_->reg_get_wait_for_completion(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_), err,
                log_(ERROR, "ds_driver_->reg_get_wait_for_completion failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) ) )
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  return_if_err(trans_->init_put(lip, lport, tmpfs_dir, data_type, data_id, data_length, data_), err, free(data_);)
  free(data_); data_ = NULL;
  
  log_(INFO, "done; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  
  return 0;
}

int RFPManager::wa_get(std::string lip, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  log_(INFO, "started; lip= " << lip << ", lport= " << lport << "\n"
            << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  data_id__recved_size_map[data_id] = 0;
  data_id__data_map[data_id] = malloc(size*DSDriver::get_data_length(ndim, gdim_, lb_, ub_) );
  
  if (trans_->init_get(data_type, lport, data_id, boost::bind(&RFPManager::handle_recv, this, _1, _2, _3) ) ) {
    log_(ERROR, "trans_->init_get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
    return 1;
  }
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_id__data_map[data_id] ) ) {
    log_(ERROR, "ds_driver_->sync_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
    return 1;
  }
  
  free(data_id__data_map[data_id] ); data_id__data_map[data_id] = NULL;
  data_id__data_map.del(data_id);
  data_id__recved_size_map.del(data_id);
  // rfp_syncer.notify(patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) );
  // 
  log_(INFO, "done; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  return 0;
}

int RFPManager::clean_up_for_retry(std::string data_id)
{
  if (!data_id__data_map.contains(data_id) ) {
    log_(ERROR, "data_id__data_map does not contain data_id= " << data_id)
    return 1;
  }
  free(data_id__data_map[data_id] ); data_id__data_map[data_id] = NULL;
  data_id__data_map.del(data_id);
  data_id__recved_size_map.del(data_id);
  return 0;
}

bool RFPManager::is_being_get(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_)
{
  return data_id__recved_size_map.contains(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) );
}

int RFPManager::wait_for_get(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_)
{
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  if (data_id__recved_size_map.contains(data_id) ) {
    log_(INFO, "started; waiting on " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
    unsigned int sync_point = patch_sdm::hash_str(data_id);
    rfp_syncer.add_sync_point(sync_point, 1);
    rfp_syncer.wait(sync_point);
    rfp_syncer.del_sync_point(sync_point);
    log_(INFO, "done; waiting on " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  }
}

int RFPManager::notify_remote_get_done(std::string key, unsigned int ver, uint64_t* lb_, uint64_t* ub_)
{
  rfp_syncer.notify(patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) );
  return 0;
}

void RFPManager::handle_recv(std::string data_id, uint64_t data_size, void* data_)
{
  if (!data_id__recved_size_map.contains(data_id) ) {
    log_(ERROR, "data is recved for a non-existing data_id= " << data_id)
    return;
  }
  
  uint64_t recved_size = data_id__recved_size_map[data_id];
  log_(INFO, "for data_id= " << data_id
            << ", recved data_size= " << data_size
            << ", total_recved_size= " << (float)(recved_size + data_size)/1024/1024 << "(MB)")
  
  char* data_t_ = static_cast<char*>(data_id__data_map[data_id] );
  memcpy(data_t_ + recved_size, data_, data_size);
  
  data_id__recved_size_map[data_id] += data_size;
  
  free(data_); data_ = NULL;
  log_(INFO, "done; data_id= " << data_id)
}

/********************************************  RIManager  *****************************************/
RIManager::RIManager(int cl_id, int base_client_id, int num_peer, DATA_ID_T data_id_t,
                     std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                     std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                     std::string tcp_lip, int tcp_lport,
                     std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
: cl_id(cl_id), base_client_id(base_client_id), num_peer(num_peer), data_id_t(data_id_t),
  data_trans_protocol(data_trans_protocol),
  ds_driver_(new DSDriver(cl_id, num_peer) ),
  rfp_manager_(new RFPManager(data_id_t, data_trans_protocol,
                              ib_lip, ib_lport_list,
                              tcp_lip, tcp_lport,
                              gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir, ds_driver_) ),
  signals(io_service, SIGINT)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

RIManager::~RIManager() { log_(INFO, "destructed.") }

void RIManager::close()
{
  log_(INFO, "started.")
  sdm_slave_->close();
  lsdm_node_->close();
  ds_driver_->close();
  ri_syncer.close();
  // 
  signals.async_wait(boost::bind(&RIManager::close, this) );
  log_(INFO, "done.")
  // 
  exit(1); // ugly but kept hanging after ctrl-c
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t cl_id= " << cl_id << "\n"
     << "\t num_peer= " << num_peer << "\n"
     << "\t rfp_manager= \n" << rfp_manager_->to_str() << "\n";
  
  return ss.str();
}

// ----------------------------------------  handle app_req  ------------------------------------ //
void RIManager::handle_app_msg(std::map<std::string, std::string> msg_map)
{
  std::string type = msg_map["type"];
  int cl_id = boost::lexical_cast<int>(msg_map["cl_id"] );
  
  if (str_str_equals(type, GET) )
    handle_get(false, cl_id, msg_map);
  else if (str_str_equals(type, BLOCKING_GET) )
    handle_get(true, cl_id, msg_map);
  else if (str_str_equals(type, GET_DONE) )
    handle_get_done(cl_id, msg_map);
  else if (str_str_equals(type, PUT) )
    handle_put(cl_id, msg_map);
  else {
    log_(ERROR, "unknown type= " << type)
  }
}

void RIManager::handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map)
{
  log_(INFO, "get_map= \n" << patch::map_to_str<>(get_map) )
  
  sdm_slave_->reg_app(cl_id);
  
  if (blocking)
    get_map["type"] = BLOCKING_GET_REPLY;
  else
    get_map["type"] = GET_REPLY;
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t* gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed! get_map= \n" << patch::map_to_str<>(get_map) )
    get_map["ds_id"] = "-1";
  }
  else {
    rfp_manager_->wait_for_get(key, ver, lb_, ub_);
    std::string data_id = patch_sdm::get_data_id(data_id_t, get_map);
    if (failed_remote_get_data_id_v.contains(data_id) ) {
      log_(INFO, "failed remote get for data_id= " << data_id << ", returning failed get...")
      get_map["ds_id"] = "-1";
    }
    else {
      if (sdm_slave_->get(cl_id, blocking, key, ver, lb_, ub_) ) {
        log_(INFO, "does not exist " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
        get_map["ds_id"] = "-1";
      }
      else {
        busy_data_id_v.push_back(data_id);
        get_map["ds_id"] = boost::lexical_cast<std::string>(sdm_slave_->get_id() );
        // Note: If an app local-peer to SDMMaster initiated this get and data is put by another local-peer, 
        // add_access should not be called.
        // Note: add_access will be done by the SDMMaster upon recving SDM_SQUERY for early start of prefetching
        // if (sdm_slave_->add_access(cl_id, key, ver, lb_, ub_) ) {
        //   log_(ERROR, "sdm_slave_->add_access failed; c_id= " << cl_id << ", " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
        // }
      }
    }
  }
  // Note: For nstx_sync_sim
  // bool wait = true;
  // {
  //   boost::lock_guard<boost::mutex> guard(ns_m);
    
  //   if (ns_cl_id_v.contains(cl_id) ) {
  //     ns_cl_id_v.clear();
  //   }
    
  //   ns_cl_id_v.push_back(cl_id);
  //   if (ns_cl_id_v.size() > 6) {
  //     wait = false;
  //     for (std::vector<unsigned int>::iterator it = ns_sync_point_v.begin(); it != ns_sync_point_v.end(); it++)
  //       ri_syncer.notify(*it);
  //     ns_sync_point_v.clear();
  //   }
  // }
  // if (wait) {
  //   unsigned int sync_point = patch_sdm::hash_str("ns_sim" + patch_sdm::get_data_id(data_id_t, get_map) );
  //   ns_sync_point_v.push_back(sync_point);
  //   ri_syncer.add_sync_point(sync_point, 1);
  //   ri_syncer.wait(sync_point);
  //   ri_syncer.del_sync_point(sync_point);
  // }
  
  if (lsdm_node_->send_msg(cl_id, PACKET_RIMSG, get_map) ) {
    log_(ERROR, "lsdm_node_->send_msg_to_master failed; get_map= \n" << patch::map_to_str<>(get_map) )
    patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  }
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
}

int RIManager::handle_get_done(int cl_id, std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  busy_data_id_v.del(data_id);
  ri_syncer.notify(patch_sdm::hash_str("D" + data_id) );
  
  return 0;
}

void RIManager::handle_put(int p_id, std::map<std::string, std::string> put_map)
{
  log_(INFO, "put_map= \n" << patch::map_to_str<>(put_map) )
  
  sdm_slave_->reg_app(p_id);
  
  put_map["type"] = PUT_REPLY;
  
  std::string key, data_type;
  unsigned int ver;
  int size, ndim;
  uint64_t* gdim_, *lb_, *ub_;
  if (msg_coder.decode_msg_map(put_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed! get_map= \n" << patch::map_to_str<>(put_map) )
    put_map["ds_id"] = "-1";
  }
  else {
    if (sdm_slave_->put(true, key, ver, lb_, ub_, p_id) ) {
      log_(ERROR, "sdm_slave_->put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
      put_map["ds_id"] = "-1";
    }
    else {
      put_map["ds_id"] = boost::lexical_cast<std::string>(sdm_slave_->get_id() );
      data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) ] =
        boost::make_shared<data_info>(data_type, size, gdim_);
    }
  }
  if (lsdm_node_->send_msg(p_id, PACKET_RIMSG, put_map) )
    log_(ERROR, "lsdm_node_->send_msg_to_master failed; put_map= \n" << patch::map_to_str<>(put_map) )

  patch::free_all<uint64_t>(2, lb_, ub_);
}

// ------------------------------------  handle rimsg  ------------------------------------------ //
int RIManager::trans_info_query(int to_id, std::map<std::string, std::string> msg_map)
{
  msg_map["type"] = RI_TINFO_QUERY;
  if (sdm_slave_->send_rimsg(to_id, msg_map) ) {
    log_(ERROR, "sdm_slave_->send_rimsg failed msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  log_(INFO, "waiting for tinfo_query_reply for data_id= " << data_id)
  unsigned int sync_point = patch_sdm::hash_str(RI_TINFO_QUERY + "_" + data_id);
  ri_syncer.add_sync_point(sync_point, 1);
  ri_syncer.wait(sync_point);
  ri_syncer.del_sync_point(sync_point);
  log_(INFO, "done waiting for tinfo_query_reply for data_id= " << data_id)
  
  return 0;
}

void RIManager::handle_rimsg(std::map<std::string, std::string> msg_map)
{
  std::string type = msg_map["type"];
  
  if (str_str_equals(type, RI_TINFO_QUERY) )
    handle_tinfo_query(msg_map);
  else if (str_str_equals(type, RI_TINFO_QUERY_REPLY) )
    handle_tinfo_query_reply(msg_map);
  else if (str_str_equals(type, RI_GRIDFTP_PUT) )
    handle_gridftp_put(msg_map);
  else if (str_str_equals(type, RI_REPEAT_TRANS) )
    handle_repeat_trans(msg_map);
  else if (str_str_equals(type, RI_REPEAT_TRANS_REPLY) )
    handle_repeat_trans_reply(msg_map);
  else if (str_str_equals(type, RI_FAIL_REMOTE_GET) )
    handle_fail_remote_get(msg_map);
  else
    log_(ERROR, "unknown type= " << type)
}

void RIManager::handle_tinfo_query(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  msg_map["type"] = RI_TINFO_QUERY_REPLY;
  msg_map["lip"] = rfp_manager_->get_lip();
  msg_map["lport"] = rfp_manager_->get_lport();
  msg_map["tmpfs_dir"] = rfp_manager_->get_tmpfs_dir();
  
  if (str_str_equals(data_trans_protocol, INFINIBAND) || str_str_equals(data_trans_protocol, TCP) ) {
    // boost::thread t(&RIManager::remote_get, this, msg_map);
    std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
    data_id__remote_get_t_map[data_id] = boost::make_shared<boost::thread>(&RIManager::remote_get, this, msg_map);
  }
  
  if (sdm_slave_->send_rimsg(boost::lexical_cast<int>(msg_map["id"] ), msg_map) ) {
    log_(ERROR, "sdm_slave_->send_rimsg failed msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
}

int RIManager::remote_get(std::map<std::string, std::string> msg_map)
{
  int err;
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  std::string key, data_type;
  unsigned ver;
  int size, ndim;
  uint64_t* gdim_, *lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  // Note: Some get requests may get through wait_for_get in handle_get so will check here once again
  // if (rfp_manager_->is_being_get(key, ver, lb_, ub_) ) {
  //   log_(WARNING, "rfp_manager_->is_being_get data so cancel this blasphemy remote_get; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  //   return;
  // }
  
  if (rfp_manager_->wa_get(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"],
                           key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    log_(ERROR, "rfp_manager_->wa_get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
    patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
    return 1;
  }
  
  if (sdm_slave_->put(false, key, ver, lb_, ub_, REMOTE_P_ID) ) {
    log_(ERROR, "sdm_slave_->put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
    return 1;
  }
  else
    data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) ] =
      boost::make_shared<data_info>(data_type, size, gdim_);
  // Note: To avoid sdm_slave_->get to fail when wa_get is done but sdm_slave_->put is not done yet.
  return_if_err(rfp_manager_->notify_remote_get_done(key, ver, lb_, ub_), err)
  return_if_err(sdm_slave_->notify_remote_get_done(key, ver, lb_, ub_), err)
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
}

void RIManager::remote_put(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned ver;
  uint64_t* lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  int to_id = boost::lexical_cast<int>(msg_map["to_id"] );
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  unsigned int data_id_hash = patch_sdm::hash_str(data_id);
  boost::shared_ptr<data_info> data_info_ = data_id_hash__data_info_map[data_id_hash];
  boost::shared_ptr<trans_info> trans_info_ = data_id_hash__trans_info_map[data_id_hash];
  
  int err = rfp_manager_->wa_put(trans_info_->lip, trans_info_->lport, trans_info_->tmpfs_dir,
                                 key, ver, data_info_->data_type, data_info_->size, ndim, data_info_->gdim_, lb_, ub_);
  if (err) {
    log_(ERROR, "rfp_manager_->wa_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) << "\n"
                << "will NOT repeat.")
    msg_map["type"] = RI_FAIL_REMOTE_GET;
    return_err_if_ret_cond_flag(sdm_slave_->send_rimsg(to_id, msg_map), err, !=, 0,)
    
    // log_(ERROR, "rfp_manager_->wa_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) << "\n"
    //             << "will repeat...")
    // msg_map["type"] = RI_REPEAT_TRANS;
    // msg_map["lip"] = boost::lexical_cast<std::string>(trans_info_->lip);
    // msg_map["lport"] = boost::lexical_cast<std::string>(trans_info_->lport);
    
    // return_err_if_ret_cond_flag(sdm_slave_->send_rimsg(to_id, msg_map), err, !=, 0,)
    // log_(INFO, "waiting for act upon RI_REPEAT_TRANS_REPLY; data_id= " << data_id)
    // unsigned int sync_point = patch_sdm::hash_str(RI_REPEAT_TRANS + "_" + data_id);
    // ri_syncer.add_sync_point(sync_point, 1);
    // ri_syncer.wait(sync_point);
    // ri_syncer.del_sync_point(sync_point);
    // log_(INFO, "done waiting for act upon RI_REPEAT_TRANS_REPLY; data_id= " << data_id)
    
    // Note: done in handle_repeat_trans_reply
    // log_(INFO, "waiting for RI_REPEAT_TRANS_REPLY; data_id= " << data_id)
    // unsigned int sync_point = patch_sdm::hash_str(RI_REPEAT_TRANS + "_" + data_id);
    // ri_syncer.add_sync_point(sync_point, 1);
    // ri_syncer.wait(sync_point);
    // ri_syncer.del_sync_point(sync_point);
    // log_(INFO, "done waiting for RI_REPEAT_TRANS_REPLY; data_id= " << data_id)
    
    // if (msg_map.count("repeat") == 0) {
    //   log_(INFO, "apparently trans was successful, will not repeat; data_id= " << data_id)
    //   break;
    // }
    // else {
    //   log_(INFO, "indeed trans was failure, will repeat; data_id= " << data_id)
    //   err = rfp_manager_->wa_put(trans_info_->lip, trans_info_->lport, trans_info_->tmpfs_dir,
    //                             key, ver, data_info_->data_type, data_info_->size, ndim, data_info_->gdim_, lb_, ub_);
    // }
  }
  
  log_(INFO, "done; key= " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) )
  patch::free_all<uint64_t>(2, lb_, ub_);
}

void RIManager::handle_tinfo_query_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  // int from_id = boost::lexical_cast<int>(msg_map["id"] );
  // if (data_id_hash__trans_info_map.contains(from_id) ) {
  //   if (str_str_equals(data_trans_protocol, INFINIBAND) ) {
  //     data_id_hash__trans_info_map[from_id] = boost::make_shared<trans_info>(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"] );
  //     log_(WARNING, "updating data_id_hash__trans_info_map for ds_id= " << from_id)
  //   }
  // }
  // else
  //   data_id_hash__trans_info_map[from_id] = boost::make_shared<trans_info>(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"] );
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  unsigned int data_id_hash = patch_sdm::hash_str(data_id);
  data_id_hash__trans_info_map[data_id_hash] = boost::make_shared<trans_info>(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"] );
  
  ri_syncer.notify(patch_sdm::hash_str(RI_TINFO_QUERY + "_" + data_id) );
}

void RIManager::handle_gridftp_put(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  if (str_str_equals(data_trans_protocol, GRIDFTP) )
    remote_get(msg_map);
  else {
    log_(ERROR, "unexpected data_trans_protocol= " << data_trans_protocol)
    exit(1);
  }
}

void RIManager::handle_repeat_trans(std::map<std::string, std::string> msg_map)
{
  int err;
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  
  int ndim;
  std::string key;
  unsigned ver;
  uint64_t* gdim_, *lb_, *ub_;
  return_err_if_ret_cond_flag(msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_), err, !=, 0,)
  // rfp_manager_->wait_for_get(key, ver, lb_, ub_);
  if (sdm_slave_->query(key, ver, lb_, ub_) ) {
    log_(INFO, "requires repeat indeed; data_id= " << data_id)
    msg_map["repeat"] = "";
    msg_map["lport"] = rfp_manager_->get_lport();
    
    data_id__remote_get_t_map[data_id]->interrupt();
    data_id__remote_get_t_map[data_id].reset();
    data_id__remote_get_t_map.del(data_id);
    rfp_manager_->clean_up_for_retry(data_id);
    
    data_id__remote_get_t_map[data_id] = boost::make_shared<boost::thread>(&RIManager::remote_get, this, msg_map);
  }
  else {
    log_(INFO, "does not require repeat; data_id= " << data_id)
  }
  
  msg_map["type"] = RI_REPEAT_TRANS_REPLY;
  return_err_if_ret_cond_flag(sdm_slave_->send_rimsg(boost::lexical_cast<int>(msg_map["id"] ), msg_map), err, !=, 0,)
  
  patch::free_all<uint64_t>(2, lb_, ub_);
}

void RIManager::handle_repeat_trans_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  int err;
  
  int ndim;
  std::string key;
  unsigned ver;
  uint64_t* lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  int to_id = boost::lexical_cast<int>(msg_map["to_id"] );
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  boost::shared_ptr<data_info> data_info_ =
    data_id_hash__data_info_map[patch_sdm::hash_str(data_id) ];
  boost::shared_ptr<trans_info> trans_info_ = data_id_hash__trans_info_map[to_id];
  
  if (msg_map.count("repeat") == 0) {
    log_(INFO, "apparently trans was successful, will not repeat; data_id= " << data_id)
  }
  else {
    log_(INFO, "indeed trans was failure, will repeat; data_id= " << data_id)
    err = rfp_manager_->wa_put(trans_info_->lip, trans_info_->lport, trans_info_->tmpfs_dir,
                               key, ver, data_info_->data_type, data_info_->size, ndim, data_info_->gdim_, lb_, ub_);
    if (err) {
      log_(ERROR, "rfp_manager_->wa_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_) << "\n"
                  << "\t this is 2nd attempt and still failed, no more...")
    }
  }
  
  unsigned int sync_point = patch_sdm::hash_str(
    RI_REPEAT_TRANS + "_" + patch_sdm::get_data_id(data_id_t, msg_map) );
  ri_syncer.notify(sync_point);
  
  patch::free_all<uint64_t>(2, lb_, ub_);
}

void RIManager::handle_fail_remote_get(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  int err;
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  failed_remote_get_data_id_v.push_back(data_id);
  data_id__remote_get_t_map[data_id]->interrupt();
  log_(INFO, "remote_get_t interrupted for data_id= " << data_id)
  data_id__remote_get_t_map[data_id].reset();
  data_id__remote_get_t_map.del(data_id);
  rfp_manager_->clean_up_for_retry(data_id);
  
  int ndim;
  std::string key;
  unsigned ver;
  uint64_t* lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  return_err_if_ret_cond_flag(rfp_manager_->notify_remote_get_done(key, ver, lb_, ub_), err, !=, 0,)
  sdm_slave_->stop_get(data_id);
  
  patch::free_all<uint64_t>(2, lb_, ub_);
}

// ------------------------------------  handle dm_act  ----------------------------------------- //
void RIManager::handle_dm_act(std::map<std::string, std::string> dm_act_map)
{
  std::string type = dm_act_map["type"];
  
  if (str_str_equals(type, SDM_MOVE) )
    handle_dm_move(dm_act_map);
  else if (str_str_equals(type, SDM_DEL) )
    handle_dm_del(dm_act_map);
  else {
    log_(ERROR, "unknown type= " << type)
  }
}

void RIManager::handle_dm_move(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int to_id = boost::lexical_cast<int>(msg_map["to_id"] );
  if (!data_id_hash__trans_info_map.contains(to_id) || str_str_equals(data_trans_protocol, INFINIBAND) || str_str_equals(data_trans_protocol, TCP) ) {
    boost::shared_ptr<data_info> data_info_ =
      data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, msg_map) ) ];
    
    msg_map["data_type"] = data_info_->data_type;
    msg_map["size"] = boost::lexical_cast<std::string>(data_info_->size);
    msg_map["gdim_"] = patch::arr_to_str<>(NDIM, data_info_->gdim_);
    
    trans_info_query(to_id, msg_map);
  }
  
  remote_put(msg_map);
  
  if (str_str_equals(data_trans_protocol, GRIDFTP) ) {
    msg_map["type"] = RI_GRIDFTP_PUT;
    if (sdm_slave_->send_rimsg(to_id, msg_map) ) {
      log_(ERROR, "sdm_slave_->send_rimsg failed msg_map= \n" << patch::map_to_str<>(msg_map) )
      return;
    }
  }
  log_(INFO, "done; key= " << msg_map["key"] )
}

int RIManager::handle_dm_del(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  // if (data_id_t != KV_DATA_ID) {
  //   log_(WARNING, "dspaces_remove is supported only for KV_DATA_ID!")
  //   return 1;
  // }
  
  // std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  // if (busy_data_id_v.contains(data_id) ) {
  //   log_(INFO, "waiting for del; data_id= " << data_id)
  //   unsigned int sync_point = patch_sdm::hash_str("D" + data_id);
  //   ri_syncer.add_sync_point(sync_point, 1);
  //   ri_syncer.wait(sync_point);
  //   ri_syncer.del_sync_point(sync_point);
  //   log_(INFO, "done waiting for del; data_id= " << data_id)
  // }
  
  // int err;
  // return_if_err(ds_driver_->del(msg_map["key"].c_str(), boost::lexical_cast<unsigned int>(msg_map["ver"] ) ), err)
  // log_(INFO, "deled; data_id= " << data_id)
  
  return 0;
}
