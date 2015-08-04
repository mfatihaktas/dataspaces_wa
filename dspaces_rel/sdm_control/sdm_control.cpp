#include "sdm_control.h"

/******************************************  HSDMMaster  ******************************************/
HSDMMaster::HSDMMaster(char id, std::string type,
                       std::string lip, int lport,
                       std::string joinhost_lip, int joinhost_lport,
                       func_rimsg_recv_cb _rimsg_recv_cb,
                       int pbuffer_size, int pexpand_length,
                       COOR_T* lcoor_, COOR_T* ucoor_)
: SDMMaster(id, type, lip, lport, joinhost_lip, joinhost_lport,
            _rimsg_recv_cb),
  wa_space(std::vector<char>(), pbuffer_size, pexpand_length,
           lcoor_, ucoor_),
  num_slaves(0)
{
  // 
  LOG(INFO) << "HSDMMaster:: constructed";
}

HSDMMaster::~HSDMMaster() { LOG(INFO) << "HSDMMaster:: destructed"; }

int HSDMMaster::sdm_mquery(COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_MQUERY;
  if (msg_coder.encode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "sdm_mquery:: msg_coder.encode_msg_map failed for " << LUCOOR_TO_STR(lcoor_, ucoor_);
    return 1;
  }
  
  if (broadcast_msg_to_slaves(msg_map) ) {
    LOG(ERROR) << "sdm_mquery:: broadcast_msg_to_slaves failed for msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  std::string sync_point = patch_sfc::arr_to_str(NDIM, lcoor_) + "/" + patch_sfc::arr_to_str(NDIM, ucoor_);
  sdm_mquery_syncer.add_sync_point(sync_point, num_slaves);
  sdm_mquery_syncer.wait();
  sdm_mquery_syncer.del_sync_point(sync_point);
  
  return 0;
}

void HSDMMaster::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  num_slaves++;
}

void HSDMMaster::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  std::string type = msg_map["type"];
  if (type.compare(SDM_MQUERY_REPLY) == 0)
    handle_sdm_mquery_reply(msg_map);
  else if (type.compare(SDM_PUT_NOTIFICATION) == 0)
    handle_sdm_put_notification(msg_map);
  else if (type.compare(SDM_SQUERY) == 0)
    handle_sdm_squery(msg_map);
  else if (type.compare(SDM_MOVE_REPLY) == 0)
    handle_sdm_move_reply(msg_map);
  else
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
}

void HSDMMaster::handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  if (msg_map["ack"].compare("n") == 0)
    return;
  
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery_reply:: msg_coder.decode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  wa_space.put(boost::lexical_cast<int>(msg_map["p_id"] ), lcoor_, ucoor_);
  
  sdm_mquery_syncer.notify(msg_map["lcoor_"] + "/" + msg_map["ucoor_"] );
}

void HSDMMaster::handle_sdm_put_notification(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_put_notification:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_put_notification:: msg_coder.decode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  wa_space.put(boost::lexical_cast<int>(msg_map["p_id"] ), lcoor_, ucoor_);
}

void HSDMMaster::handle_sdm_squery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_squery:: msg_coder.decode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  bool move_act = true;
  std::vector<char> ds_id_v;
  if (wa_space.query(lcoor_, ucoor_, ds_id_v) ) {
    LOG(INFO) << "handle_sdm_squery:: not available " << LUCOOR_TO_STR(lcoor_, ucoor_);
    sdm_mquery(lcoor_, ucoor_);
    if (wa_space.query(lcoor_, ucoor_, ds_id_v) ) {
      LOG(INFO) << "handle_sdm_squery:: after sdm_mquery still not available " << LUCOOR_TO_STR(lcoor_, ucoor_);
      move_act = false;
    }
  }
  if (move_act) {
    std::map<std::string, std::string> move_act_msg_map;
    move_act_msg_map["type"] = SDM_MOVE;
    move_act_msg_map["to_id"] = msg_map["id"];
    // TODO: choose from ds_id_v wisely -- considering proximity, load etc.
    char from_id = boost::lexical_cast<char>(ds_id_v[0] );
    if (send_msg(from_id, move_act_msg_map) ) {
      LOG(ERROR) << "handle_sdm_squery:: send_msg failed for msg_map= " << patch_sfc::map_to_str<>(move_act_msg_map);
      return;
    }
    
    LOG(INFO) << "handle_sdm_squery:: waiting for move; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << LUCOOR_TO_STR(lcoor_, ucoor_);
    std::string sync_point = patch_sfc::arr_to_str(NDIM, lcoor_) + "/" + patch_sfc::arr_to_str(NDIM, ucoor_);
    sdm_move_syncer.add_sync_point(sync_point, 1);
    sdm_move_syncer.wait();
    sdm_move_syncer.del_sync_point(sync_point);
    LOG(INFO) << "handle_sdm_squery:: done waiting; from_id= " << from_id << ", to_id= " << msg_map["id"] << ", " << LUCOOR_TO_STR(lcoor_, ucoor_);
  }
  msg_map["type"] = SDM_SQUERY_REPLY;
  if (send_msg(boost::lexical_cast<char>(msg_map["id"] ), msg_map) ) {
    LOG(ERROR) << "handle_sdm_squery:: send_msg failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
}

void HSDMMaster::handle_sdm_move_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  sdm_move_syncer.notify(msg_map["lcoor_"] + "/" + msg_map["ucoor_"] );
}

/******************************************  HSDMSlave  *******************************************/
HSDMSlave::HSDMSlave(char ds_id, std::string type,
                     std::string lip, int lport,
                     std::string joinhost_lip, int joinhost_lport,
                     func_rimsg_recv_cb _rimsg_recv_cb,
                     function_cb_on_dm_act cb_on_dm_act)
: SDMSlave(id, type, lip, lport, joinhost_lip, joinhost_lport,
           _rimsg_recv_cb),
  cb_on_dm_act(cb_on_dm_act)
{
  // 
  LOG(INFO) << "HSDMSlave:: constructed";
}

HSDMSlave::~HSDMSlave() { LOG(INFO) << "HSDMSlave:: destructed"; }

int HSDMSlave::put(COOR_T* lcoor_, COOR_T* ucoor_, int p_id)
{
  rtable.add(lcoor_, ucoor_, p_id);
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = SDM_PUT_NOTIFICATION;
  if (msg_coder.encode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_query:: msg_coder.encode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "put:: send_msg_to_master failed; " << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  return 0;
}

// Does not return data, returns 0 if data is available in local dspaces
int HSDMSlave::get(COOR_T* lcoor_, COOR_T* ucoor_)
{
  std::vector<int> p_id_v;
  if (rtable.query(lcoor_, ucoor_, p_id_v) )
    LOG(ERROR) << "get:: rtable.query failed; " << LUCOOR_TO_STR(lcoor_, ucoor_);
  
  std::map<std::string, std::string> sq_msg_map;
  sq_msg_map["type"] = SDM_SQUERY;
  if (msg_coder.encode_msg_map(sq_msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "get:: msg_coder.encode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(sq_msg_map);
    return 1;
  }
  if (send_msg_to_master(sq_msg_map) ) {
    LOG(ERROR) << "get:: send_msg_to_master failed; " << patch_sfc::map_to_str<>(sq_msg_map);
    return 1;
  }
  // sdm_squery_reply is received if data is not available or data is available and moved to slave's dspaces
  std::string sync_point = patch_sfc::arr_to_str(NDIM, lcoor_) + "/" + patch_sfc::arr_to_str(NDIM, ucoor_);
  sdm_squery_syncer.add_sync_point(sync_point, 1);
  sdm_squery_syncer.wait();
  sdm_squery_syncer.del_sync_point(sync_point);
  
  p_id_v.clear();
  if (rtable.query(lcoor_, ucoor_, p_id_v) ) {
    LOG(ERROR) << "get:: rtable.query failed after squery; " << LUCOOR_TO_STR(lcoor_, ucoor_);
    return 1;
  }
  
  return 0;
}

void HSDMSlave::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  // LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  std::string type = msg_map["type"];
  if (type.compare(SDM_MQUERY) == 0)
    handle_sdm_mquery(msg_map);
  else if (type.compare(SDM_SQUERY_REPLY) == 0)
    handle_sdm_squery_reply(msg_map);
  else if (type.compare(SDM_MOVE) == 0)
    handle_sdm_move(msg_map);
  else
    LOG(ERROR) << "handle_msg_in:: unknown type= " << type;
}

void HSDMSlave::handle_sdm_mquery(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_mquery:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  COOR_T *lcoor_, *ucoor_;
  if (msg_coder.decode_msg_map(msg_map, lcoor_, ucoor_) ) {
    LOG(ERROR) << "handle_sdm_mquery:: msg_coder.decode_msg_map failed for msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
  
  msg_map["type"] = SDM_MQUERY_REPLY;
  
  std::vector<int> p_id_v;
  if (rtable.query(lcoor_, ucoor_, p_id_v) ) {
    LOG(INFO) << "handle_sdm_mquery:: rtable.query failed; " << LUCOOR_TO_STR(lcoor_, ucoor_);
    msg_map["ack"] = "n";
  }
  else {
    msg_map["ack"] = "a";
    if (p_id_v.size() > 1)
      LOG(WARNING) << "handle_sdm_mquery:: put by multi-p; " << LUCOOR_TO_STR(lcoor_, ucoor_);
    msg_map["p_id"] = boost::lexical_cast<std::string>(p_id_v[0] );
  }
  
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "handle_sdm_mquery:: send_msg_to_master failed; " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
}

void HSDMSlave::handle_sdm_squery_reply(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_squery_reply:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  sdm_squery_syncer.notify(msg_map["lcoor_"] + "/" + msg_map["ucoor_"] );
}

void HSDMSlave::handle_sdm_move(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_sdm_move:: msg_map= \n" << patch_sfc::map_to_str<>(msg_map);
  
  cb_on_dm_act(msg_map);
  
  msg_map["type"] = SDM_MOVE_REPLY;
  if (send_msg_to_master(msg_map) ) {
    LOG(ERROR) << "handle_sdm_move:: send_msg_to_master failed; " << patch_sfc::map_to_str<>(msg_map);
    return;
  }
}
