#include "sdm_control.h"

/***********************************  SDMSlave : SDMCEntity  **************************************/
SDMSlave::SDMSlave(char data_id_t, std::string type,
                   char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                   func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                   boost::shared_ptr<QTable<int> > qtable_)
: SDMCEntity(type,
             ds_id, lip, lport, joinhost_lip, joinhost_lport,
             rimsg_recv_cb),
  data_id_t(data_id_t), dm_act_cb(dm_act_cb), type(type), qtable_(qtable_)
{
  // 
  LOG(INFO) << "SDMSlave:: constructed; \n" << to_str();
}

SDMSlave::~SDMSlave() { LOG(INFO) << "SDMSlave:: destructed"; }

int SDMSlave::close()
{
  SDMCEntity::close();
  sdm_s_syncer.close();
  
  LOG(INFO) << "SDMSlave:: closed.";
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
    LOG(ERROR) << "reg_app:: already reged app_id= " << app_id;
    return 1;
  }
  app_id_v.push_back(app_id);
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_REG_APP;
  msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  if (send_cmsg_to_master(msg_map) ) {
    LOG(ERROR) << "reg_app:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(SDM_REG_APP + "_" + msg_map["app_id"] );
  sdm_s_syncer.add_sync_point(sync_point, 1);
  sdm_s_syncer.wait(sync_point);
  sdm_s_syncer.del_sync_point(sync_point);
  
  return 0;
}

int SDMSlave::put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  LOG(INFO) << "put:: notify= " << notify << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  qtable_->add(key, ver, lcoor_, ucoor_, p_id);
  
  if (notify) {
    std::map<std::string, std::string> msg_map;
    msg_map["type"] = SDM_PUT_NOTIFICATION;
    msg_map["p_id"] = boost::lexical_cast<std::string>(p_id);
    if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
      LOG(ERROR) << "handle_sdm_query:: msg_coder.encode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
      return 1;
    }
    
    if (send_cmsg_to_master(msg_map) ) {
      LOG(ERROR) << "put:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
      return 1;
    }
  }
  
  return 0;
}

// Does not return data, returns 0 if data is available in local dspaces
int SDMSlave::get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  LOG(INFO) << "get:: " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    LOG(ERROR) << "get:: qtable_->query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
    std::map<std::string, std::string> sq_msg_map;
    sq_msg_map["type"] = SDM_SQUERY;
    if (blocking)
      sq_msg_map["blocking"] = "";
    if (msg_coder.encode_msg_map(sq_msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
      LOG(ERROR) << "get:: msg_coder.encode_msg_map failed for msg_map= " << patch_all::map_to_str<>(sq_msg_map);
      return 1;
    }
    if (send_cmsg_to_master(sq_msg_map) ) {
      LOG(ERROR) << "get:: send_cmsg_to_master failed; " << patch_all::map_to_str<>(sq_msg_map);
      return 1;
    }
    // sdm_squery_reply is received if data is not available or data is available and moved to slave's dspaces
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
    sdm_s_syncer.add_sync_point(sync_point, 1);
    sdm_s_syncer.wait(sync_point);
    sdm_s_syncer.del_sync_point(sync_point);
    
    p_id_v.clear();
    if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
      LOG(ERROR) << "get:: qtable_->query failed after squery; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      return 1;
    }
  }
  
  return 0;
}

void SDMSlave::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
}

void SDMSlave::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
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
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
}

void SDMSlave::handle_sdm_mquery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  msg_map["type"] = SDM_MQUERY_REPLY;
  
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    LOG(INFO) << "handle_sdm_mquery:: qtable_->query failed; " KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    msg_map["ack"] = "n";
  }
  else {
    msg_map["ack"] = "a";
    if (p_id_v.size() > 1)
      LOG(WARNING) << "handle_sdm_mquery:: put by multi-p; " << LUCOOR_TO_STR(lcoor_, ucoor_);
    msg_map["p_id"] = boost::lexical_cast<std::string>(p_id_v[0] );
  }
  
  if (send_cmsg_to_master(msg_map) )
    LOG(ERROR) << "handle_sdm_mquery:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_squery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_squery_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  sdm_s_syncer.notify(patch_sdm::hash_str(
    SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_reg_app_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_reg_app_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  sdm_s_syncer.notify(patch_sdm::hash_str(
    SDM_REG_APP + "_" + msg_map["app_id"] ) );
}

void SDMSlave::handle_sdm_move(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  dm_act_cb(msg_map);
  
  msg_map["type"] = SDM_MOVE_REPLY;
  if (send_cmsg_to_master(msg_map) ) {
    LOG(ERROR) << "handle_sdm_move:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
}

void SDMSlave::handle_sdm_del(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_del:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  dm_act_cb(msg_map);
  
  msg_map["type"] = SDM_DEL_REPLY;
  if (send_cmsg_to_master(msg_map) ) {
    LOG(ERROR) << "handle_sdm_del:: send_cmsg_to_master failed; msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
}

/*************************************  MSDMSlave : SDMSlave  *************************************/
MSDMSlave::MSDMSlave(char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb)
: SDMSlave(KV_DATA_ID, type,
           ds_id, lip, lport, joinhost_lip, joinhost_lport,
           rimsg_recv_cb, dm_act_cb,
           boost::make_shared<KVTable<int> >() )
{
  // 
  LOG(INFO) << "MSDMSlave:: constructed.";
}

MSDMSlave::~MSDMSlave() { LOG(INFO) << "MSDMSlave:: destructed."; }

/*************************************  SSDMSlave : SDMSlave  *************************************/
SSDMSlave::SSDMSlave(char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb)
: SDMSlave(LUCOOR_DATA_ID, type,
           ds_id, lip, lport, joinhost_lip, joinhost_lport,
           rimsg_recv_cb, dm_act_cb,
           boost::make_shared<RTable<int> >() )
{
  // 
  LOG(INFO) << "SSDMSlave:: constructed.";
}

SSDMSlave::~SSDMSlave() { LOG(INFO) << "SSDMSlave:: destructed."; }

/*************************************  SDMMaster : SDMSlave  *************************************/
SDMMaster::SDMMaster(char data_id_t,
                     char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
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
  LOG(INFO) << "SDMMaster:: constructed; \n" << to_str();
}

SDMMaster::~SDMMaster() { LOG(INFO) << "SDMMaster:: destructed"; }

int SDMMaster::close()
{
  SDMSlave::close();
  sdm_m_syncer.close();
  
  LOG(INFO) << "SDMMaster:: closed.";
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
    LOG(ERROR) << "sdm_mquery:: msg_coder.encode_msg_map failed for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  if (broadcast_cmsg_to_slaves(msg_map) ) {
    LOG(ERROR) << "sdm_mquery:: broadcast_cmsg_to_slaves failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
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
  char ds_id = get_id();
  if (wa_space_->reg_app(app_id, ds_id) ) {
    LOG(ERROR) << "handle_sdm_reg_app:: wa_space_->reg_app failed for <app_id= " << app_id << ", ds_id= " << ds_id << ">.";
    return 1;
  }
  
  return 0;
}

int SDMMaster::put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  LOG(INFO) << "put:: notify= " << notify << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
  if (qtable_->add(key, ver, lcoor_, ucoor_, p_id) ) {
    LOG(ERROR) << "put:: qtable_->add failed for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
    
  return wa_space_->put(p_id, key, ver, lcoor_, ucoor_);
}

// Does not return data, returns 0 if data is available in local dspaces
int SDMMaster::get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  LOG(INFO) << "get:: " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    LOG(ERROR) << "get:: qtable_->query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
    if (sdm_mquery(key, ver, lcoor_, ucoor_) ) {
      LOG(ERROR) << "get:: sdm_mquery failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      return 1;
    }
    
    p_id_v.clear();
    if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
      LOG(ERROR) << "get:: qtable_->query failed after squery; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      return 1;
    }
  }
  
  return 0;
}

void SDMMaster::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  if (str_cstr_equals(msg_map["node_type"], "s") ) { // Slave
    wa_space_->reg_ds(boost::lexical_cast<char>(msg_map["id"] ) );
    
    num_slaves++;
  }
  if (str_cstr_equals(msg_map["node_type"], "m") ) { // Master
    LOG(ERROR) << "handle_conn_up:: only one master is supported for now! Aborting...";
    close();
    exit(1);
  }
  
}

void SDMMaster::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
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
  else {
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
    SDMSlave::handle_msg_in(msg_map);
  }
}

void SDMMaster::handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  if (msg_map["ack"].compare("n") == 0)
    return;
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  if (wa_space_->put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_) )
    LOG(ERROR) << "handle_sdm_mquery_reply:: wa_space_->put failed for p_id= " << msg_map["p_id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  else
    sdm_m_syncer.notify(patch_sdm::hash_str(
      SDM_MQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_all::free_all<COOR_T>(2,  lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_reg_app(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_reg_app:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  if (wa_space_->reg_app(boost::lexical_cast<int>(msg_map["app_id"] ), boost::lexical_cast<char>(msg_map["id"] ) ) ) {
    LOG(ERROR) << "handle_sdm_reg_app:: wa_space_->reg_app failed for <app_id= " << msg_map["app_id"] << ", ds_id= " << msg_map["id"] << ">.";
    return;
  }
  msg_map["type"] = SDM_REG_APP_REPLY;
  if (send_cmsg(boost::lexical_cast<char>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_sdm_squery:: send_cmsg failed for msg_map= " << patch_all::map_to_str<>(msg_map);
    return;
  }
}

void SDMMaster::handle_sdm_put_notification(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_put_notification:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_put_notification:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  wa_space_->put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_);
  
  unsigned int sync_point = patch_sdm::hash_str(
    "B" + SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  sdm_m_syncer.notify(sync_point);
  
  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_squery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_squery:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  bool move_act = true;
  std::vector<char> ds_id_v;
  if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    LOG(INFO) << "handle_sdm_squery:: not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    // sdm_mquery(key, ver, lcoor_, ucoor_);
    // if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    //   LOG(INFO) << "handle_sdm_squery:: after sdm_mquery still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      
      if (msg_map.count("blocking") == 0)
        move_act = false;
      else {
        LOG(INFO) << "handle_sdm_squery:: blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
        unsigned int sync_point = patch_sdm::hash_str(
          "B" + SDM_SQUERY + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
        sdm_m_syncer.add_sync_point(sync_point, 1);
        sdm_m_syncer.wait(sync_point);
        sdm_m_syncer.del_sync_point(sync_point);
        LOG(INFO) << "handle_sdm_squery:: done blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      
        if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
          LOG(ERROR) << "handle_sdm_squery:: after blocking still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
          move_act = false;
        }
      }
    // }
  }
  if (move_act) {
    LOG(INFO) << "handle_sdm_squery:: move_act= " << move_act << ", ds_id_v= " << patch_all::vec_to_str<>(ds_id_v);
    
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    char from_id = ds_id_v[0];
    std::map<std::string, std::string> move_act_msg_map;
    if (msg_coder.encode_msg_map(move_act_msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
      LOG(ERROR) << "handle_sdm_squery:: msg_coder.encode_msg_map failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      return;
    }
    move_act_msg_map["type"] = SDM_MOVE;
    move_act_msg_map["to_id"] = msg_map["id"];
    if (from_id == get_id() ) {
      move_act_msg_map["id"] = from_id;
      dm_act_cb(move_act_msg_map);
    }
    else {
      if (send_cmsg(from_id, move_act_msg_map) ) {
        LOG(ERROR) << "handle_sdm_squery:: send_cmsg failed for msg_map= " << patch_all::map_to_str<>(move_act_msg_map);
        return;
      }
      
      LOG(INFO) << "handle_sdm_squery:: waiting for move; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      unsigned int sync_point = patch_sdm::hash_str(
        SDM_MOVE + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
      sdm_m_syncer.add_sync_point(sync_point, 1);
      sdm_m_syncer.wait(sync_point);
      sdm_m_syncer.del_sync_point(sync_point);
      LOG(INFO) << "handle_sdm_squery:: done waiting for move; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    }
  }
  msg_map["type"] = SDM_SQUERY_REPLY;
  if (send_cmsg(boost::lexical_cast<char>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_sdm_squery:: send_cmsg failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_move_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_move_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  sdm_m_syncer.notify(patch_sdm::hash_str(
    SDM_MOVE + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_del_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_del_reply:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_del_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
    return;
  }
  
  sdm_m_syncer.notify(patch_sdm::hash_str(
    SDM_DEL + "_" + patch_sdm::get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_wa_space_data_act(PREFETCH_DATA_ACT_T data_act_t, char to_id, key_ver_pair kv, lcoor_ucoor_pair lucoor_)
{
  std::map<std::string, std::string> msg_map;
  if (msg_coder.encode_msg_map(msg_map, NDIM, kv.first, kv.second, lucoor_.first, lucoor_.second) ) {
    LOG(ERROR) << "handle_wa_space_data_act:: msg_coder.encode_msg_map failed; " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
    return;
  }
  
  if (data_act_t == PREFETCH_DATA_ACT_PREFETCH) {
    std::vector<char> ds_id_v;
    if (wa_space_->query(kv.first, kv.second, lucoor_.first, lucoor_.second, ds_id_v) ) {
      LOG(ERROR) << "handle_wa_space_data_act:: data_act_t= " << data_act_t << ", non-existing data; " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
      return;
    }
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    char from_id = ds_id_v[0];
    msg_map["type"] = SDM_MOVE;
    msg_map["to_id"] = to_id;
    
    if (send_cmsg(from_id, msg_map) ) {
      LOG(ERROR) << "handle_wa_space_data_act:: send_cmsg failed for msg_map= " << patch_all::map_to_str<>(msg_map);
      return;
    }
    
    LOG(INFO) << "handle_wa_space_data_act:: waiting for move; from_id= " << from_id << ", to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_MOVE + "_" + patch_sdm::get_data_id(data_id_t, kv.first, kv.second, lucoor_.first, lucoor_.second) );
    sdm_m_syncer.add_sync_point(sync_point, 1);
    sdm_m_syncer.wait(sync_point);
    sdm_m_syncer.del_sync_point(sync_point);
    LOG(INFO) << "handle_wa_space_data_act:: done waiting; from_id= " << from_id << ", to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
  }
  else if (data_act_t == PREFETCH_DATA_ACT_DEL) {
    msg_map["type"] = SDM_DEL;
    if (send_cmsg(to_id, msg_map) ) {
      LOG(ERROR) << "handle_wa_space_data_act:: send_cmsg failed for msg_map= " << patch_all::map_to_str<>(msg_map);
      return;
    }
    
    LOG(INFO) << "handle_wa_space_data_act:: waiting for del; to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_DEL + "_" + patch_sdm::get_data_id(data_id_t, kv.first, kv.second, lucoor_.first, lucoor_.second) );
    sdm_m_syncer.add_sync_point(sync_point, 1);
    sdm_m_syncer.wait(sync_point);
    sdm_m_syncer.del_sync_point(sync_point);
    LOG(INFO) << "handle_wa_space_data_act:: done waiting for del; to_id= " << to_id << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor_.first, lucoor_.second);
  }
  
}

/************************************  MSDMMaster : SDMMaster  ************************************/
MSDMMaster::MSDMMaster(char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                       func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                       MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch)
: SDMMaster(KV_DATA_ID,
            ds_id, lip, lport, joinhost_lip, joinhost_lport,
            rimsg_recv_cb, dm_act_cb,
            boost::make_shared<KVTable<int> >(),
            boost::make_shared<MWASpace>(std::vector<char>(),
                                         malgo_t, max_num_key_ver_in_mpbuffer, w_prefetch, boost::bind(&SDMMaster::handle_wa_space_data_act, this, _1, _2, _3, _4) ) )
{
  // 
  LOG(INFO) << "MSDMMaster:: constructed; \n" << to_str();
}

MSDMMaster::~MSDMMaster() { LOG(INFO) << "MSDMMaster:: destructed."; }

std::string MSDMMaster::to_str()
{
  std::stringstream ss;
  
  return ss.str();
}

/************************************  SSDMMaster : SDMMaster  ************************************/
SSDMMaster::SSDMMaster(char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                       func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
                       SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch)
: SDMMaster(LUCOOR_DATA_ID,
            ds_id, lip, lport, joinhost_lip, joinhost_lport,
            rimsg_recv_cb, dm_act_cb,
            boost::make_shared<RTable<int> >(),
            boost::make_shared<SWASpace>(std::vector<char>(),
                                         salgo_t, lcoor_, ucoor_, sexpand_length, w_prefetch, boost::bind(&SDMMaster::handle_wa_space_data_act, this, _1, _2, _3, _4) ) )
{
  // 
  LOG(INFO) << "SSDMMaster:: constructed; \n" << to_str();
}

SSDMMaster::~SSDMMaster() { LOG(INFO) << "SSDMMaster:: destructed."; }

std::string SSDMMaster::to_str()
{
  std::stringstream ss;
  
  return ss.str();
}