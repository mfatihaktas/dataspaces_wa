#include "sdm_control.h"

/***********************************  SDMSlave : SDMCEntity  **************************************/
SDMSlave::SDMSlave(DATA_ID_T data_id_t, std::string type,
                   int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                   func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                   boost::shared_ptr<QTable<int> > qtable_)
: SDMCEntity(type,
             ds_id, lip, lport, joinhost_lip, joinhost_lport,
             rimsg_recv_cb),
  data_id_t(data_id_t), dm_act_cb(dm_act_cb), type(type), qtable_(qtable_)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

SDMSlave::~SDMSlave() { log_(INFO, "destructed") }

int SDMSlave::close()
{
  SDMCEntity::close();
  sdm_s_syncer.close();
  
  log_(INFO, "closed.")
}

std::string SDMSlave::to_str()
{
  std::stringstream ss;
  ss << "data_id_t= " << data_id_t << "\n"
     << "type= " << type << "\n"
     << "SDMCEntity::to_str= \n" << SDMCEntity::to_str() << "\n";
  
  return ss.str();
}

int SDMSlave::reg_app(int app_id)
{
  if (std::find(app_id_v.begin(), app_id_v.end(), app_id) != app_id_v.end() ) {
    log_(ERROR, "already reged app_id= " << app_id)
    return 1;
  }
  app_id_v.push_back(app_id);
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_REG_APP;
  msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  if (send_cmsg_to_master(msg_map) ) {
    log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(SDM_REG_APP + "_" + msg_map["app_id"] );
  sdm_s_syncer.add_sync_point(sync_point, 1);
  sdm_s_syncer.wait(sync_point);
  sdm_s_syncer.del_sync_point(sync_point);
  
  return 0;
}

int SDMSlave::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_ACCESS;
  msg_map["app_id"] = boost::lexical_cast<std::string>(c_id);
  if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.encode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  if (send_cmsg_to_master(msg_map) ) {
    log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  return 0;
}

int SDMSlave::put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  log_(INFO, "notify= " << notify << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  qtable_->add(key, ver, lcoor_, ucoor_, p_id);
  
  if (notify) {
    std::map<std::string, std::string> msg_map;
    msg_map["type"] = SDM_PUT_NOTIFICATION;
    msg_map["p_id"] = boost::lexical_cast<std::string>(p_id);
    if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
      log_(ERROR, "msg_coder.encode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
      return 1;
    }
    
    if (send_cmsg_to_master(msg_map) ) {
      log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
      return 1;
    }
  }
  
  return 0;
}

// Does not return data, returns 0 if data is available in local dspaces
int SDMSlave::get(int app_id, bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  // log_(INFO, KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    log_(ERROR, "qtable_->query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  
    std::map<std::string, std::string> sq_msg_map;
    sq_msg_map["type"] = SDM_SQUERY;
    sq_msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
    if (blocking)
      sq_msg_map["blocking"] = "";
    if (msg_coder.encode_msg_map(sq_msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
      log_(ERROR, "msg_coder.encode_msg_map failed for msg_map= " << patch::map_to_str<>(sq_msg_map) )
      return 1;
    }
    if (send_cmsg_to_master(sq_msg_map) ) {
      log_(ERROR, "send_cmsg_to_master failed; " << patch::map_to_str<>(sq_msg_map) )
      return 1;
    }
    // sdm_squery_reply is received if data is not available or data is available and moved to slave's dspaces
    log_(INFO, "waiting on sdm_squery_reply; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
    sdm_s_syncer.add_sync_point(sync_point, 1);
    sdm_s_syncer.wait(sync_point);
    sdm_s_syncer.del_sync_point(sync_point);
    
    p_id_v.clear();
    if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
      log_(ERROR, "qtable_->query failed after squery; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      return 1;
    }
  }
  
  return 0;
}

void SDMSlave::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
}

void SDMSlave::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  std::string type = msg_map["type"];
  if (type.compare(SDM_MQUERY) == 0)
    handle_sdm_mquery(msg_map);
  else if (type.compare(SDM_SQUERY_REPLY) == 0)
    handle_sdm_squery_reply(msg_map);
  else if (type.compare(SDM_REG_APP_REPLY) == 0)
    handle_sdm_reg_app_reply(msg_map);
  else if (type.compare(SDM_MOVE) == 0)
    handle_sdm_move(msg_map);
  else if (type.compare(SDM_DEL) == 0)
    handle_sdm_del(msg_map);
  else
    log_(ERROR, "unknown type= " << type)
}

void SDMSlave::handle_sdm_mquery(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  msg_map["type"] = SDM_MQUERY_REPLY;
  
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    log_(INFO, "qtable_->query failed; " KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
    msg_map["ack"] = "n";
  }
  else {
    msg_map["ack"] = "a";
    if (p_id_v.size() > 1)
      log_(WARNING, "put by multi-p; " << LUCOOR_TO_STR(lcoor_, ucoor_) )
    msg_map["p_id"] = boost::lexical_cast<std::string>(p_id_v[0] );
  }
  
  if (send_cmsg_to_master(msg_map) )
    log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  patch::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_squery_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  sdm_s_syncer.notify(patch_sdm::hash_str(
    SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_reg_app_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  sdm_s_syncer.notify(patch_sdm::hash_str(SDM_REG_APP + "_" + msg_map["app_id"] ) );
}

void SDMSlave::handle_sdm_move(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  dm_act_cb(msg_map);
  
  // Note: Cause SDMMaster to let ri_manager continue with the blocking squery before ri_manager actually
  // does sdm_slave_->put the data. This causes available data to seem unavailable. I moved the following
  // to ri_manager remote_get after it does sdm_slave_->put because there is always supposed to be a
  // remote_get as a response to the sdm_move.
  // msg_map["type"] = SDM_MOVE_REPLY;
  // if (send_cmsg_to_master(msg_map) ) {
  //   log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
  //   return;
  // }
}

void SDMSlave::handle_sdm_del(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  dm_act_cb(msg_map);
  
  msg_map["type"] = SDM_DEL_REPLY;
  if (send_cmsg_to_master(msg_map) ) {
    log_(ERROR, "send_cmsg_to_master failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
}

/*************************************  MSDMSlave : SDMSlave  *************************************/
MSDMSlave::MSDMSlave(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb)
: SDMSlave(KV_DATA_ID, "s",
           ds_id, lip, lport, joinhost_lip, joinhost_lport,
           rimsg_recv_cb, dm_act_cb,
           boost::make_shared<KVTable<int> >() )
{
  // 
  log_(INFO, "constructed.")
}

MSDMSlave::~MSDMSlave() { log_(INFO, "destructed.") }

/*************************************  SSDMSlave : SDMSlave  *************************************/
SSDMSlave::SSDMSlave(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb)
: SDMSlave(LUCOOR_DATA_ID, "s",
           ds_id, lip, lport, joinhost_lip, joinhost_lport,
           rimsg_recv_cb, dm_act_cb,
           boost::make_shared<RTable<int> >() )
{
  // 
  log_(INFO, "constructed.")
}

SSDMSlave::~SSDMSlave() { log_(INFO, "destructed.") }

/*************************************  SDMMaster : SDMSlave  *************************************/
SDMMaster::SDMMaster(DATA_ID_T data_id_t,
                     int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                     boost::shared_ptr<QTable<int> > qtable_, boost::shared_ptr<WASpace> wa_space_)
: SDMSlave(data_id_t, "m",
           ds_id, lip, lport, joinhost_lip, joinhost_lport,
           rimsg_recv_cb, dm_act_cb,
           qtable_),
  wa_space_(wa_space_),
  num_slaves(0)
{
  wa_space_->reg_ds(ds_id);
  // 
  log_(INFO, "constructed.")
}

SDMMaster::~SDMMaster() { log_(INFO, "destructed") }

int SDMMaster::close()
{
  SDMSlave::close();
  sdm_m_syncer.close();
  
  log_(INFO, "closed.")
}

std::string SDMMaster::to_str()
{
  std::stringstream ss;
  ss << "wa_space= \n" << wa_space_->to_str() << "\n";
  
  return ss.str();
}

int SDMMaster::sdm_mquery(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_MQUERY;
  if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.encode_msg_map failed for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
    return 1;
  }
  
  if (broadcast_cmsg_to_slaves(msg_map) ) {
    log_(ERROR, "broadcast_cmsg_to_slaves failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(
    SDM_MQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  sdm_m_syncer.add_sync_point(sync_point, num_slaves);
  sdm_m_syncer.wait(sync_point);
  sdm_m_syncer.del_sync_point(sync_point);
  
  return 0;
}

int SDMMaster::reg_app(int app_id)
{
  if (std::find(app_id_v.begin(), app_id_v.end(), app_id) != app_id_v.end() ) {
    log_(ERROR, "already reged app_id= " << app_id)
    return 1;
  }
  
  if (wa_space_->reg_app(app_id, get_id() ) ) {
    log_(ERROR, "wa_space_->reg_app failed for <app_id= " << app_id << ", ds_id= " << get_id() << ">")
    return 1;
  }
  app_id_v.push_back(app_id);
  
  return 0;
}

int SDMMaster::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  return wa_space_->add_access(c_id, key, ver, lcoor_, ucoor_);
}

int SDMMaster::put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  log_(INFO, "notify= " << notify << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  
  if (qtable_->add(key, ver, lcoor_, ucoor_, p_id) ) {
    log_(ERROR, "qtable_->add failed for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(
    "B" + SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  sdm_m_syncer.notify(sync_point);
  
  return wa_space_->put(p_id, key, ver, lcoor_, ucoor_);
}

// Does not return data, returns 0 if data is available in local dspaces
int SDMMaster::get(int c_id, bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  log_(INFO, "c_id= " << c_id << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  if (std::find(app_id_v.begin(), app_id_v.end(), c_id) == app_id_v.end() ) {
    log_(ERROR, "non-reged c_id= " << c_id)
    return 1;
  }
  
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    log_(ERROR, "qtable_->query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  
    if (sdm_mquery(key, ver, lcoor_, ucoor_) ) {
      log_(ERROR, "sdm_mquery failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      return 1;
    }
    
    p_id_v.clear();
    if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
      log_(ERROR, "qtable_->query failed after squery; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      return 1;
    }
  }
  
  if (add_access(c_id, key, ver, lcoor_, ucoor_) )
    log_(ERROR, "add_access failed; c_id= " << c_id << "," << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  
  return 0;
}

void SDMMaster::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  if (str_cstr_equals(msg_map["node_type"], "s") ) { // Slave
    wa_space_->reg_ds(boost::lexical_cast<int>(msg_map["id"] ) );
    
    num_slaves++;
  }
  if (str_cstr_equals(msg_map["node_type"], "m") ) { // Master
    log_(ERROR, "only one master is supported for now! Aborting...")
    close();
    exit(1);
  }
  
}

void SDMMaster::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  std::string type = msg_map["type"];
  if (type.compare(SDM_MQUERY_REPLY) == 0)
    handle_sdm_mquery_reply(msg_map);
  else if (type.compare(SDM_REG_APP) == 0)
    handle_sdm_reg_app(msg_map);
  else if (type.compare(SDM_PUT_NOTIFICATION) == 0)
    handle_sdm_put_notification(msg_map);
  else if (type.compare(SDM_SQUERY) == 0)
    handle_sdm_squery(msg_map);
  else if (type.compare(SDM_MOVE_REPLY) == 0)
    handle_sdm_move_reply(msg_map);
  else if (type.compare(SDM_DEL_REPLY) == 0)
    handle_sdm_del_reply(msg_map);
  else if (type.compare(SDM_ACCESS) == 0)
    handle_sdm_access(msg_map);
  else {
    log_(ERROR, "unknown type= " << type)
    SDMSlave::handle_msg_in(msg_map);
  }
}

void SDMMaster::handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  if (msg_map["ack"].compare("n") == 0)
    return;
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  if (wa_space_->put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_) )
    log_(ERROR, "wa_space_->put failed for p_id= " << msg_map["p_id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
  else
    sdm_m_syncer.notify(patch_sdm::hash_str(
      SDM_MQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch::free_all<COOR_T>(2,  lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_reg_app(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "handle_sdm_msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  if (wa_space_->reg_app(boost::lexical_cast<int>(msg_map["app_id"] ), boost::lexical_cast<int>(msg_map["id"] ) ) ) {
    log_(ERROR, "handle_sdm_wa_space_->reg_app failed for <app_id= " << msg_map["app_id"] << ", ds_id= " << msg_map["id"] << ">.")
    return;
  }
  msg_map["type"] = SDM_REG_APP_REPLY;
  if (send_cmsg(boost::lexical_cast<int>(msg_map["id"] ), msg_map) ) {
    log_(ERROR, "handle_sdm_send_cmsg failed for msg_map= " << patch::map_to_str<>(msg_map) )
    return;
  }
}

void SDMMaster::handle_sdm_put_notification(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  wa_space_->put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_);
  
  unsigned int sync_point = patch_sdm::hash_str(
    "B" + SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  sdm_m_syncer.notify(sync_point);
  
  patch::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_squery(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  int to_id = boost::lexical_cast<int>(msg_map["id"] );
  // Note: Some get requests may get through wait_for_get in handle_get so will check here to avoid double remote_put/get
  std::vector<std::string>& moving_data_id_v = ds_id__moving_data_id_v_map[to_id];
  if (std::find(moving_data_id_v.begin(), moving_data_id_v.end(), data_id) != moving_data_id_v.end() ) {
    log_(WARNING, "already moving the data to_id= " << to_id << ", data_id= " << data_id)
    return;
  }
  // Note: Reason for doing this here rather than in wait_for_move is handle_sdm_squery and handle_wa_space_data_act
  // can still be called for the same data_id otherwise
  moving_data_id_v.push_back(data_id);
  log_(INFO, "to_id= " << to_id << ", moving_data_id_v= " << patch::vec_to_str<>(moving_data_id_v) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  bool move_act = true;
  std::vector<int> ds_id_v;
  if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    log_(INFO, "not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
    // sdm_mquery(key, ver, lcoor_, ucoor_);
    // if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    //   log_(INFO, "after sdm_mquery still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      
      if (msg_map.count("blocking") == 0)
        move_act = false;
      else {
        log_(INFO, "blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
        unsigned int sync_point = patch_sdm::hash_str("B" + SDM_SQUERY + "_" + data_id);
        sdm_m_syncer.add_sync_point(sync_point, 1);
        sdm_m_syncer.wait(sync_point);
        sdm_m_syncer.del_sync_point(sync_point);
        log_(INFO, "done blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      
        if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
          log_(ERROR, "after blocking still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
          move_act = false;
        }
      }
    // }
  }
  if (move_act) {
    log_(INFO, "move_act= " << move_act << ", ds_id_v= " << patch::vec_to_str<>(ds_id_v) )
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    int from_id = ds_id_v[0];
    boost::thread(&SDMMaster::wait_for_move, this, from_id, to_id, msg_map);
    
    std::map<std::string, std::string> move_act_msg_map;
    if (msg_coder.encode_msg_map(move_act_msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
      log_(ERROR, "msg_coder.encode_msg_map failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
      return;
    }
    move_act_msg_map["type"] = SDM_MOVE;
    move_act_msg_map["id"] = boost::lexical_cast<std::string>(from_id);
    move_act_msg_map["to_id"] = msg_map["id"];
    if (from_id == get_id() ) {
      dm_act_cb(move_act_msg_map);
    }
    else {
      if (send_cmsg(from_id, move_act_msg_map) ) {
        log_(ERROR, "send_cmsg failed for msg_map= " << patch::map_to_str<>(move_act_msg_map) )
        return;
      }
    }
  }

  patch::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::wait_for_move(int from_id, int to_id, std::map<std::string, std::string> msg_map)
{
  log_(INFO, "waiting; from_id= " << from_id << ", to_id= " << to_id << ", msg_map= \n" << patch::map_to_str<>(msg_map) )
  std::string data_id = patch_sdm::get_data_id(data_id_t, msg_map);
  // ds_id__moving_data_id_v_map[to_id].push_back(data_id);
  // log_(INFO, "to_id= " << to_id << ", moving_data_id_v= " << patch::vec_to_str<>(ds_id__moving_data_id_v_map[to_id] ) )
  
  unsigned int sync_point = patch_sdm::hash_str(
    SDM_MOVE + "_" + data_id);
  sdm_m_syncer.add_sync_point(sync_point, 1);
  sdm_m_syncer.wait(sync_point);
  sdm_m_syncer.del_sync_point(sync_point);
  log_(INFO, "done waiting; from_id= " << from_id << ", to_id= " << to_id << ", msg_map= \n" << patch::map_to_str<>(msg_map) )
  std::vector<std::string>& moving_data_id_v = ds_id__moving_data_id_v_map[to_id];
  moving_data_id_v.erase(std::find(moving_data_id_v.begin(), moving_data_id_v.end(), data_id) );

  msg_map["type"] = SDM_SQUERY_REPLY;
  if (send_cmsg(boost::lexical_cast<int>(to_id), msg_map) ) {
    log_(ERROR, "send_cmsg failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
}

void SDMMaster::handle_sdm_move_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  sdm_m_syncer.notify(patch_sdm::hash_str(
    SDM_MOVE + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_del_reply(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  sdm_m_syncer.notify(patch_sdm::hash_str(SDM_DEL + "_" + patch_sdm::get_data_id(data_id_t, msg_map) ) );
}

void SDMMaster::handle_sdm_access(std::map<std::string, std::string> msg_map)
{
  log_(INFO, "msg_map= \n" << patch::map_to_str<>(msg_map) )
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    log_(ERROR, "msg_coder.decode_msg_map failed for msg_map= \n" << patch::map_to_str<>(msg_map) )
    return;
  }
  
  if (add_access(boost::lexical_cast<int>(msg_map["app_id"] ), key, ver, lcoor_, ucoor_) )
    log_(ERROR, "add_access failed; app_id= " << msg_map["app_id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) )
}

void SDMMaster::handle_wa_space_data_act(PREFETCH_DATA_ACT_T data_act_t, int to_id, key_ver_pair kv, lcoor_ucoor_pair lucoor_)
{
  log_(INFO, "started; data_act_t= " << data_act_t << ", to_id= " << to_id
             << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second) )
  
  std::map<std::string, std::string> msg_map;
  if (msg_coder.encode_msg_map(msg_map, NDIM, kv.first, kv.second, lucoor_.first, lucoor_.second) ) {
    log_(ERROR, "msg_coder.encode_msg_map failed; " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second) )
    return;
  }
  
  if (data_act_t == PREFETCH_DATA_ACT_PREFETCH) {
    // Note: Some get requests may get through wait_for_get in handle_get so will check here to avoid double remote_put/get
    std::string data_id = patch_sdm::get_data_id(data_id_t, kv.first, kv.second, lucoor_.first, lucoor_.second);
    std::vector<std::string>& moving_data_id_v = ds_id__moving_data_id_v_map[to_id];
    if (std::find(moving_data_id_v.begin(), moving_data_id_v.end(), data_id) != moving_data_id_v.end() ) {
      log_(WARNING, "already moving the data to_id= " << to_id << ", data_id= " << data_id)
      return;
    }
    
    moving_data_id_v.push_back(data_id);
    log_(INFO, "to_id= " << to_id << ", moving_data_id_v= " << patch::vec_to_str<>(moving_data_id_v) )
    
    std::vector<int> ds_id_v;
    if (wa_space_->query(kv.first, kv.second, lucoor_.first, lucoor_.second, ds_id_v) ) {
      log_(ERROR, "data_act_t= " << data_act_t << ", non-existing data; " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second) )
      return;
    }
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    int from_id = ds_id_v[0];
    boost::thread(&SDMMaster::wait_for_move, this, from_id, to_id, msg_map);
    
    msg_map["type"] = SDM_MOVE;
    msg_map["id"] = boost::lexical_cast<std::string>(from_id);
    msg_map["to_id"] = boost::lexical_cast<std::string>(to_id);
    if (from_id == get_id() ) {
      dm_act_cb(msg_map);
    }
    else {
      if (send_cmsg(from_id, msg_map) ) {
        log_(ERROR, "send_cmsg failed for msg_map= " << patch::map_to_str<>(msg_map) )
        return;
      }
    }
  }
  else if (data_act_t == PREFETCH_DATA_ACT_DEL) {
    msg_map["type"] = SDM_DEL;
    if (send_cmsg(to_id, msg_map) ) {
      log_(ERROR, "send_cmsg failed for msg_map= " << patch::map_to_str<>(msg_map) )
      return;
    }
    
    log_(INFO, "waiting for del; to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second) )
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_DEL + "_" + patch_sdm::get_data_id(data_id_t, kv.first, kv.second, lucoor_.first, lucoor_.second) );
    sdm_m_syncer.add_sync_point(sync_point, 1);
    sdm_m_syncer.wait(sync_point);
    sdm_m_syncer.del_sync_point(sync_point);
    log_(INFO, "done waiting for del; to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second) )
  }
  
}

/************************************  MSDMMaster : SDMMaster  ************************************/
MSDMMaster::MSDMMaster(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                       func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                       MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch)
: SDMMaster(KV_DATA_ID,
            ds_id, lip, lport, joinhost_lip, joinhost_lport,
            rimsg_recv_cb, dm_act_cb,
            boost::make_shared<KVTable<int> >(),
            boost::make_shared<MWASpace>(std::vector<int>(),
                                         malgo_t, max_num_key_ver_in_mpbuffer, w_prefetch, boost::bind(&SDMMaster::handle_wa_space_data_act, this, _1, _2, _3, _4) ) )
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

MSDMMaster::~MSDMMaster() { log_(INFO, "destructed.") }

std::string MSDMMaster::to_str()
{
  std::stringstream ss;
  ss << "SDMMaster::to_str= \n" << SDMMaster::to_str() << "\n";
  
  return ss.str();
}

/************************************  SSDMMaster : SDMMaster  ************************************/
SSDMMaster::SSDMMaster(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                       func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                       SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch)
: SDMMaster(LUCOOR_DATA_ID,
            ds_id, lip, lport, joinhost_lip, joinhost_lport,
            rimsg_recv_cb, dm_act_cb,
            boost::make_shared<RTable<int> >(),
            boost::make_shared<SWASpace>(std::vector<int>(),
                                         salgo_t, lcoor_, ucoor_, sexpand_length, w_prefetch, boost::bind(&SDMMaster::handle_wa_space_data_act, this, _1, _2, _3, _4) ) )
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

SSDMMaster::~SSDMMaster() { log_(INFO, "destructed.") }

std::string SSDMMaster::to_str()
{
  std::stringstream ss;
  ss << "SDMMaster::to_str= \n" << SDMMaster::to_str() << "\n";
  
  return ss.str();
}
