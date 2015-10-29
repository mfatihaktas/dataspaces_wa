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
  LOG(INFO) << "RFPManager:: constructed.";
}

RFPManager::~RFPManager() { LOG(INFO) << "RFPManager:: destructed."; }

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
  for (int i = 0; i < ndim; i++)
    volume *= (size_t)dim_length[i];
  
  return volume;
}

int RFPManager::wa_put(std::string lip, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "wa_put:: started lip= " << lip << ", lport= " << lport << "; \n"
            << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  int data_length = get_data_length(ndim, gdim_, lb_, ub_);
  if (data_length == 0) {
    LOG(ERROR) << "wa_put:: data_length = 0!";
    return 1;
  }
  void* data_ = malloc(size*data_length);
  // patch_ds::debug_print(key, ver, size, ndim, gdim_, lb_, ub_, NULL, 0);
  int get_return = ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_);
  if (get_return == -EINVAL)
    LOG(ERROR) << "wa_put:: -EINVAL !!!";
  else if (get_return == -ENOMEM)
    LOG(ERROR) << "wa_put:: -ENOMEM !!!";
  else if (get_return == -EAGAIN)
    LOG(ERROR) << "wa_put:: -EAGAIN !!!";
  if (get_return) {
    LOG(ERROR) << "wa_put:: ds_driver_->get failed; get_return= " << get_return << ", " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  if (trans_->init_put(lip, lport, tmpfs_dir, data_type, data_id, data_length, data_) ) {
    LOG(ERROR) << "wa_put:: trans_->init_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  free(data_);
  // 
  LOG(INFO) << "wa_put:: done for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  return 0;
}

int RFPManager::wa_get(std::string lip, std::string lport, std::string tmpfs_dir,
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  LOG(INFO) << "wa_get:: started lip= " << lip << ", lport= " << lport << "; \n"
            << "\t" << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  data_id__recved_size_map[data_id] = 0;
  data_id__data_map[data_id] = malloc(size*get_data_length(ndim, gdim_, lb_, ub_) );
  
  if (trans_->init_get(data_type, lport, data_id, boost::bind(&RFPManager::handle_recv, this, _1, _2, _3) ) ) {
    LOG(ERROR) << "wa_get:: trans_->init_get failed for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_id__data_map[data_id] ) ) {
    LOG(ERROR) << "wa_get:: ds_driver_->sync_put failed for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  free(data_id__data_map[data_id] );
  
  data_id__data_map.del(data_id);
  data_id__recved_size_map.del(data_id);
  // rfp_syncer.notify(patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) );
  // 
  LOG(INFO) << "wa_get:: done for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  
  return 0;
}

int RFPManager::wait_for_get(std::string key, unsigned int ver, uint64_t *lb_, uint64_t *ub_)
{
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  if (data_id__recved_size_map.contains(data_id) ) {
    LOG(INFO) << "wait_for_get:: waiting on " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    unsigned int sync_point = patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) );
    rfp_syncer.add_sync_point(sync_point, 1);
    rfp_syncer.wait(sync_point);
    rfp_syncer.del_sync_point(sync_point);
    LOG(INFO) << "wait_for_get:: done waiting on " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  }
}

int RFPManager::notify_remote_get_done(std::string key, unsigned int ver, uint64_t *lb_, uint64_t *ub_)
{
  return rfp_syncer.notify(patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) );
}

void RFPManager::handle_recv(std::string data_id, int data_size, void* data_)
{
  if (!data_id__recved_size_map.contains(data_id) ) {
    LOG(ERROR) << "handle_recv:: data is received for a non-existing data_id= " << data_id;
    return;
  }
  
  int recved_size = data_id__recved_size_map[data_id];
  LOG(INFO) << "handle_recv:: for data_id= " << data_id
            << ", recved data_size= " << data_size
            << ", total_recved_size= " << (float)(recved_size + data_size)/1024/1024 << "(MB)";
  
  char* data_t_ = static_cast<char*>(data_id__data_map[data_id] );
  memcpy(data_t_ + recved_size, data_, data_size);
  
  data_id__recved_size_map[data_id] += data_size;
}

/********************************************  RIManager  *****************************************/
RIManager::RIManager(int cl_id, int base_client_id, int num_client, DATA_ID_T data_id_t,
                     std::string lcontrol_lip, int lcontrol_lport, std::string join_lcontrol_lip, int join_lcontrol_lport,
                     std::string data_trans_protocol, std::string ib_lip, std::list<std::string> ib_lport_list,
                     std::string tcp_lip, int tcp_lport,
                     std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
: cl_id(cl_id), base_client_id(base_client_id), num_client(num_client), data_id_t(data_id_t),
  data_trans_protocol(data_trans_protocol),
  ds_driver_(new DSDriver(cl_id, num_client) ),
  lsdm_node_(boost::make_shared<SDMNode>(
    "m", true,
    boost::lexical_cast<int>(cl_id), lcontrol_lip, lcontrol_lport, join_lcontrol_lip, join_lcontrol_lport,
    boost::bind(&RIManager::handle_app_msg, this, _1) ) ),
  rfp_manager_(new RFPManager(data_id_t, data_trans_protocol,
                              ib_lip, ib_lport_list,
                              tcp_lip, tcp_lport,
                              gftp_lintf, gftp_lip, gftp_lport, tmpfs_dir, ds_driver_) ),
  signals(io_service, SIGINT)
{
  // 
  LOG(INFO) << "RIManager:: constructed; \n" << to_str();
}

RIManager::~RIManager() { LOG(INFO) << "RIManager:: destructed."; }

void RIManager::close()
{
  LOG(INFO) << "close:: closing...";
  lsdm_node_->close();
  sdm_slave_->close();
  ds_driver_->close();
  ri_syncer.close();
  // 
  signals.async_wait(boost::bind(&RIManager::close, this) );
  LOG(INFO) << "close:: done.";
  // 
  exit(1); // ugly but kept hanging after ctrl-c
}

std::string RIManager::to_str()
{
  std::stringstream ss;
  ss << "\t cl_id= " << cl_id << "\n"
     << "\t num_client= " << num_client << "\n"
     << "\t lsdm_node= \n" << lsdm_node_->to_str() << "\n"
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
  else if (str_str_equals(type, PUT) )
    handle_put(cl_id, msg_map);
  else
    LOG(ERROR) << "handle_app_msg:: unknown type= " << type;
}

void RIManager::handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map)
{
  LOG(INFO) << "handle_get:: get_map= \n" << patch_all::map_to_str<>(get_map);
  
  sdm_slave_->reg_app(cl_id);
  
  if (blocking)
    get_map["type"] = BLOCKING_GET_REPLY;
  else
    get_map["type"] = GET_REPLY;
  
  std::string key;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  std::string data_type;
  if (msg_coder.decode_msg_map(get_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_get:: msg_coder.decode_msg_map failed! get_map= \n" << patch_all::map_to_str<>(get_map);
    get_map["ds_id"] = "-1";
  }
  else {
    rfp_manager_->wait_for_get(key, ver, lb_, ub_);
    if (sdm_slave_->get(cl_id, blocking, key, ver, lb_, ub_) ) {
      LOG(INFO) << "handle_get:: does not exist " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
      get_map["ds_id"] = "-1";
    }
    else {
      get_map["ds_id"] = boost::lexical_cast<std::string>(sdm_slave_->get_id() );
      // Note: If an app local-peer to SDMMaster initiated this get and data is put by another local-peer, 
      // add_access should not be called.
      if (sdm_slave_->add_access(cl_id, key, ver, lb_, ub_) )
        LOG(ERROR) << "handle_get:: sdm_slave_->add_access failed; c_id= " << cl_id << ", " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    }
  }
  if (lsdm_node_->send_msg(cl_id, PACKET_RIMSG, get_map) )
    LOG(ERROR) << "handle_get:: lsdm_node_->send_msg_to_master failed; get_map= \n" << patch_all::map_to_str<>(get_map);
  
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
}

void RIManager::handle_put(int p_id, std::map<std::string, std::string> put_map)
{
  LOG(INFO) << "handle_put:: put_map= \n" << patch_all::map_to_str<>(put_map);
  
  sdm_slave_->reg_app(p_id);
  
  put_map["type"] = PUT_REPLY;
  
  std::string key, data_type;
  unsigned int ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if (msg_coder.decode_msg_map(put_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "handle_put:: msg_coder.decode_msg_map failed! get_map= \n" << patch_all::map_to_str<>(put_map);
    put_map["ds_id"] = "-1";
  }
  else {
    if (sdm_slave_->put(true, key, ver, lb_, ub_, p_id) ) {
      LOG(ERROR) << "handle_put:: sdm_slave_->put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
      put_map["ds_id"] = "-1";
    }
    else {
      put_map["ds_id"] = boost::lexical_cast<std::string>(sdm_slave_->get_id() );
      data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) ] =
        boost::make_shared<data_info>(data_type, size, gdim_);
    }
  }
  if (lsdm_node_->send_msg(p_id, PACKET_RIMSG, put_map) )
    LOG(ERROR) << "handle_put:: lsdm_node_->send_msg_to_master failed; put_map= \n" << patch_all::map_to_str<>(put_map);

  patch_all::free_all<uint64_t>(2, lb_, ub_);
}

// ------------------------------------  handle rimsg  ------------------------------------------ //
int RIManager::trans_info_query(int to_id, std::map<std::string, std::string> msg_map)
{
  msg_map["type"] = RI_TINFO_QUERY;
  if (sdm_slave_->send_rimsg(to_id, msg_map) ) {
    LOG(ERROR) << "trans_info_query:: sdm_slave_->send_rimsg failed msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(
    RI_TINFO_QUERY + "_" + patch_sdm::get_data_id(data_id_t, msg_map) );
  ri_syncer.add_sync_point(sync_point, 1);
  ri_syncer.wait(sync_point);
  ri_syncer.del_sync_point(sync_point);
  
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
  else
    LOG(ERROR) << "handle_rimsg:: unknown type= " << type;
}

void RIManager::handle_tinfo_query(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_tinfo_query:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  msg_map["type"] = RI_TINFO_QUERY_REPLY;
  msg_map["lip"] = rfp_manager_->get_lip();
  msg_map["lport"] = rfp_manager_->get_lport();
  msg_map["tmpfs_dir"] = rfp_manager_->get_tmpfs_dir();
  
  if (str_str_equals(data_trans_protocol, INFINIBAND) || str_str_equals(data_trans_protocol, TCP) )
    boost::thread(&RIManager::remote_get, this, msg_map);
  
  if (sdm_slave_->send_rimsg(boost::lexical_cast<int>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_tinfo_query:: sdm_slave_->send_rimsg failed msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
}

void RIManager::remote_get(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "remote_get:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  std::string key, data_type;
  unsigned ver;
  int size, ndim;
  uint64_t *gdim_, *lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_get:: msg_coder.decode_msg_map failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  if (rfp_manager_->wa_get(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"],
                           key, ver, data_type, size, ndim, gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_get:: rfp_manager_->wa_get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
    return;
  }
  
  if (sdm_slave_->put(false, key, ver, lb_, ub_, REMOTE_P_ID) )
    LOG(ERROR) << "remote_get:: sdm_slave_->put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  else
    data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) ] =
      boost::make_shared<data_info>(data_type, size, gdim_);
  // Note: To avoid sdm_slave_->get to fail when wa_get is done but sdm_slave_->put is not done yet.
  rfp_manager_->notify_remote_get_done(key, ver, lb_, ub_);
  
  msg_map["type"] = SDM_MOVE_REPLY;
  if (sdm_slave_->send_cmsg_to_master(msg_map) ) {
    LOG(ERROR) << "remote_get:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  patch_all::free_all<uint64_t>(2, lb_, ub_);
}

void RIManager::remote_put(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "remote_put:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned ver;
  uint64_t *lb_, *ub_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_) ) {
    LOG(ERROR) << "remote_put:: msg_coder.decode_msg_map failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  boost::shared_ptr<data_info> data_info_ =
    data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) ) ];
  boost::shared_ptr<trans_info> trans_info_ = ds_id__trans_info_map[boost::lexical_cast<int>(msg_map["to_id"] ) ];
  
  if (rfp_manager_->wa_put(trans_info_->lip, trans_info_->lport, trans_info_->tmpfs_dir,
                          key, ver, data_info_->data_type, data_info_->size, ndim, data_info_->gdim_, lb_, ub_) ) {
    LOG(ERROR) << "remote_put:: rfp_manager_->wa_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return;
  }
  
  // Note: To resolve unexpected data_size and data_id received by the tcp_server.
  // unsigned int data_id_hash = patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_) );
  // boost::shared_ptr<trans_info> trans_info_ = ds_id__trans_info_map[boost::lexical_cast<int>(msg_map["to_id"] ) ];
  
  // if (rfp_manager_->wa_put(trans_info_->lip, trans_info_->lport, trans_info_->tmpfs_dir,
  //                         key, ver, data_id_hash__data_info_map[data_id_hash]->data_type, data_id_hash__data_info_map[data_id_hash]->size, ndim, data_id_hash__data_info_map[data_id_hash]->gdim_, lb_, ub_) ) {
  //   LOG(ERROR) << "remote_put:: rfp_manager_->wa_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  //   return;
  // }
  
  patch_all::free_all<uint64_t>(2, lb_, ub_);
}

void RIManager::handle_tinfo_query_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_tinfo_query_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int from_id = boost::lexical_cast<int>(msg_map["id"] );
  
  if (ds_id__trans_info_map.contains(from_id) ) {
    if (str_str_equals(data_trans_protocol, INFINIBAND) ) {
      ds_id__trans_info_map[from_id] = boost::make_shared<trans_info>(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"] );
      LOG(WARNING) << "handle_tinfo_query_reply:: updating ds_id__trans_info_map for ds_id= " << from_id;
    }
  }
  else
    ds_id__trans_info_map[from_id] = boost::make_shared<trans_info>(msg_map["lip"], msg_map["lport"], msg_map["tmpfs_dir"] );
  
  ri_syncer.notify(patch_sdm::hash_str(
    RI_TINFO_QUERY + "_" + patch_sdm::get_data_id(data_id_t, msg_map) ) );
}

void RIManager::handle_gridftp_put(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_gridftp_put:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  if (str_str_equals(data_trans_protocol, GRIDFTP) )
    remote_get(msg_map);
  else {
    LOG(ERROR) << "handle_gridftp_put:: unexpected data_trans_protocol= " << data_trans_protocol;
    exit(1);
  }
}

// ------------------------------------  handle dm_act  ----------------------------------------- //
void RIManager::handle_dm_act(std::map<std::string, std::string> dm_act_map)
{
  std::string type = dm_act_map["type"];
  
  if (str_str_equals(type, SDM_MOVE) )
    handle_dm_move(dm_act_map);
  else if (str_str_equals(type, SDM_DEL) )
    handle_dm_del(dm_act_map);
  else
    LOG(ERROR) << "handle_dm_act:: unknown type= " << type;
}

void RIManager::handle_dm_move(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_dm_move:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int to_id = boost::lexical_cast<int>(msg_map["to_id"] );
  if (!ds_id__trans_info_map.contains(to_id) || str_str_equals(data_trans_protocol, INFINIBAND) || str_str_equals(data_trans_protocol, TCP) ) {
    boost::shared_ptr<data_info> data_info_ =
      data_id_hash__data_info_map[patch_sdm::hash_str(patch_sdm::get_data_id(data_id_t, msg_map) ) ];
    
    msg_map["data_type"] = data_info_->data_type;
    msg_map["size"] = boost::lexical_cast<std::string>(data_info_->size);
    msg_map["gdim_"] = patch_all::arr_to_str(NDIM, data_info_->gdim_);
    
    trans_info_query(to_id, msg_map);
  }
  
  remote_put(msg_map);
  
  if (str_str_equals(data_trans_protocol, GRIDFTP) ) {
    msg_map["type"] = RI_GRIDFTP_PUT;
    if (sdm_slave_->send_rimsg(to_id, msg_map) ) {
      LOG(ERROR) << "handle_dm_move:: sdm_slave_->send_rimsg failed msg_map= \n" << patch_all::map_to_str<>(msg_map);
      return;
    }
  }
}

void RIManager::handle_dm_del(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_dm_del:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
}
