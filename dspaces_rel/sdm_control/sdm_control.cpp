#include "sdm_control.h"

/*******************************************  SDMMaster  ******************************************/
SDMMaster::SDMMaster(char predictor_t,
                     char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb rimsg_recv_cb,
                     int pbuffer_size, int pexpand_length, COOR_T* lcoor_, COOR_T* ucoor_)
: SDMCEntity(ds_id, "m", lip, lport, joinhost_lip, joinhost_lport,
             rimsg_recv_cb, boost::bind(&SDMMaster::handle_recv, this, _1) ),
  predictor_t(predictor_t),
  wa_space(predictor_t, std::vector<char>(), 
           pbuffer_size, pexpand_length, lcoor_, ucoor_),
  num_slaves(0)
{
  if (predictor_t == HILBERT_PREDICTOR)
    data_id_t = LUCOOR_DATA_ID;
    // data_id_t = patch_sdm::KV_DATA_ID;
  // 
  LOG(INFO) << "SDMMaster:: constructed";
}

SDMMaster::~SDMMaster() { LOG(INFO) << "SDMMaster:: destructed"; }

int SDMMaster::sdm_mquery(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_MQUERY;
  if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "sdm_mquery:: msg_coder.encode_msg_map failed for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  if (broadcast_msg_to_slaves(msg_map) ) {
    LOG(ERROR) << "sdm_mquery:: broadcast_msg_to_slaves failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(
    SDM_MQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  sdm_m_syncer.add_sync_point(sync_point, num_slaves);
  sdm_m_syncer.wait(sync_point);
  sdm_m_syncer.del_sync_point(sync_point);
  
  return 0;
}

void SDMMaster::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  num_slaves++;
}

void SDMMaster::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
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
  else
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
}

void SDMMaster::handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  if (msg_map["ack"].compare("n") == 0)
    return;
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  if (wa_space.put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_) )
    LOG(ERROR) << "handle_sdm_mquery_reply:: wa_space.put failed for p_id= " << msg_map["p_id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  else
    sdm_m_syncer.notify(patch_sdm::hash_str(
      SDM_MQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_sdm::free_all<COOR_T>(2,  lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_reg_app(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_reg_app:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  if (wa_space.reg_app(boost::lexical_cast<int>(msg_map["app_id"] ), boost::lexical_cast<char>(msg_map["id"] ) ) ) {
    LOG(ERROR) << "handle_sdm_reg_app:: wa_space.reg_app failed for <app_id= " << msg_map["app_id"] << ", ds_id= " << msg_map["id"] << ">.";
    return;
  }
  msg_map["type"] = SDM_REG_APP_REPLY;
  if (send_msg(boost::lexical_cast<char>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_sdm_squery:: send_msg failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
}

void SDMMaster::handle_sdm_put_notification(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_put_notification:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_put_notification:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  wa_space.put(boost::lexical_cast<int>(msg_map["p_id"] ), key, ver, lcoor_, ucoor_);
  
  unsigned int sync_point = patch_sdm::hash_str(
    "B" + SDM_SQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
  if (sdm_m_syncer.contains(sync_point) )
    sdm_m_syncer.notify(sync_point);
  
  patch_sdm::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_squery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_squery:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  bool move_act = true;
  std::vector<char> ds_id_v;
  if (wa_space.query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    LOG(INFO) << "handle_sdm_squery:: not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    // sdm_mquery(key, ver, lcoor_, ucoor_);
    // if (wa_space.query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    //   LOG(INFO) << "handle_sdm_squery:: after sdm_mquery still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      
      if (msg_map.count("blocking") == 0)
        move_act = false;
      else {
        LOG(INFO) << "handle_sdm_squery:: blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
        unsigned int sync_point = patch_sdm::hash_str(
          "B" + SDM_SQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
        sdm_m_syncer.add_sync_point(sync_point, 1);
        sdm_m_syncer.wait(sync_point);
        sdm_m_syncer.del_sync_point(sync_point);
        LOG(INFO) << "handle_sdm_squery:: done blocking for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
      
        if (wa_space.query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
          LOG(ERROR) << "handle_sdm_squery:: after blocking still not available " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
          move_act = false;
        }
      }
    // }
  }
  if (move_act) {
    LOG(INFO) << "handle_sdm_squery:: move_act= " << move_act << ", ds_id_v= " << patch_sfc::vec_to_str<>(ds_id_v);
    
    std::map<std::string, std::string> move_act_msg_map;
    move_act_msg_map["type"] = SDM_MOVE;
    move_act_msg_map["key"] = msg_map["key"];
    move_act_msg_map["ver"] = msg_map["ver"];
    move_act_msg_map["ndim"] = msg_map["ndim"];
    move_act_msg_map["lcoor_"] = msg_map["lcoor_"];
    move_act_msg_map["ucoor_"] = msg_map["ucoor_"];
    move_act_msg_map["to_id"] = msg_map["id"];
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    char from_id = ds_id_v[0];
    if (send_msg(from_id, move_act_msg_map) ) {
      LOG(ERROR) << "handle_sdm_squery:: send_msg failed for msg_map= " << patch_sfc::map_to_str<>(move_act_msg_map);
      return;
    }

    LOG(INFO) << "handle_sdm_squery:: waiting for move; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_MOVE + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
    sdm_m_syncer.add_sync_point(sync_point, 1);
    sdm_m_syncer.wait(sync_point);
    sdm_m_syncer.del_sync_point(sync_point);
    LOG(INFO) << "handle_sdm_squery:: done waiting; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  }
  msg_map["type"] = SDM_SQUERY_REPLY;
  if (send_msg(boost::lexical_cast<char>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_sdm_squery:: send_msg failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  patch_sdm::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMMaster::handle_sdm_move_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_move_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  sdm_m_syncer.notify(patch_sdm::hash_str(
    SDM_MOVE + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_sdm::free_all<COOR_T>(2, lcoor_, ucoor_);
}

/*******************************************  SDMSlave  *******************************************/
SDMSlave::SDMSlave(char data_id_t, func_dm_act_cb dm_act_cb,
                   char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                   func_rimsg_recv_cb rimsg_recv_cb)
: SDMCEntity(ds_id, "s", lip, lport, joinhost_lip, joinhost_lport,
             rimsg_recv_cb, boost::bind(&SDMSlave::handle_recv, this, _1) ),
  data_id_t(data_id_t), dm_act_cb(dm_act_cb),
  qtable_(boost::make_shared<RTable<int> >() )
{
  // 
  LOG(INFO) << "SDMSlave:: constructed";
}

SDMSlave::~SDMSlave() { LOG(INFO) << "SDMSlave:: destructed"; }

int SDMSlave::send_msg_to_master(std::map<std::string, std::string> msg_map)
{
  return sdm_node.send_msg_to_master(msg_map);
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
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "reg_app:: send_msg_to_master failed; msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  unsigned int sync_point = patch_sdm::hash_str(SDM_REG_APP + "_" + msg_map["app_id"] );
  sdm_s_syncer.add_sync_point(sync_point, 1);
  sdm_s_syncer.wait(sync_point);
  sdm_s_syncer.del_sync_point(sync_point);
  
  return 0;
}

int SDMSlave::put(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  qtable_->add(key, ver, lcoor_, ucoor_, p_id);
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_PUT_NOTIFICATION;
  msg_map["p_id"] = boost::lexical_cast<std::string>(p_id);
  if (msg_coder.encode_msg_map(msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_query:: msg_coder.encode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "put:: send_msg_to_master failed; msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  return 0;
}

// Does not return data, returns 0 if data is available in local dspaces
int SDMSlave::get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::vector<int> p_id_v;
  if (qtable_->query(key, ver, lcoor_, ucoor_, p_id_v) ) {
    LOG(ERROR) << "get:: qtable_->query failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
  
    std::map<std::string, std::string> sq_msg_map;
    sq_msg_map["type"] = SDM_SQUERY;
    if (blocking)
      sq_msg_map["blocking"] = "";
    if (msg_coder.encode_msg_map(sq_msg_map, NDIM, key, ver, lcoor_, ucoor_) ) {
      LOG(ERROR) << "get:: msg_coder.encode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(sq_msg_map);
      return 1;
    }
    if (send_msg_to_master(sq_msg_map) ) {
      LOG(ERROR) << "get:: send_msg_to_master failed; " << patch_sfc::map_to_str<>(sq_msg_map);
      return 1;
    }
    // sdm_squery_reply is received if data is not available or data is available and moved to slave's dspaces
    unsigned int sync_point = patch_sdm::hash_str(
      SDM_SQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) );
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
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
}

void SDMSlave::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  std::string type = msg_map["type"];
  if (type.compare(SDM_MQUERY) == 0)
    handle_sdm_mquery(msg_map);
  else if (type.compare(SDM_SQUERY_REPLY) == 0)
    handle_sdm_squery_reply(msg_map);
  else if (type.compare(SDM_REG_APP_REPLY) == 0)
    handle_sdm_reg_app_reply(msg_map);
  else if (type.compare(SDM_MOVE) == 0)
    handle_sdm_move(msg_map);
  else
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
}

void SDMSlave::handle_sdm_mquery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
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
  
  if (send_msg_to_master(msg_map) )
    LOG(ERROR) << "handle_sdm_mquery:: send_msg_to_master failed; msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  patch_sdm::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_squery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_squery_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  sdm_s_syncer.notify(patch_sdm::hash_str(
    SDM_SQUERY + "_" + get_data_id(data_id_t, key, ver, lcoor_, ucoor_) ) );

  patch_sdm::free_all<COOR_T>(2, lcoor_, ucoor_);
}

void SDMSlave::handle_sdm_reg_app_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_reg_app_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  sdm_s_syncer.notify(patch_sdm::hash_str(
    SDM_REG_APP + "_" + msg_map["app_id"] ) );
}

void SDMSlave::handle_sdm_move(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  dm_act_cb(msg_map);
  
  msg_map["type"] = SDM_MOVE_REPLY;
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "handle_sdm_move:: send_msg_to_master failed; msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return;
  }
}
