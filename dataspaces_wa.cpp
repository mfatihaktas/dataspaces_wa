#include "dataspaces_wa.h"

const int SLEEP_FOR_RI = 2000000;

/***************************************  WADSDriver  ****************************************/
// Note: Reason why base_client_id is added in BCClient's arg rather than eliminating it by taking 
// app_id arg as base_client_id + app_id is because DS throws segmentation fault in a weird way while
// initializing it with arbitrary app_id's.
// Overall, app_id is used for DSDriver, _app_id is used as the application id
WADSDriver::WADSDriver(int app_id, int base_client_id, int num_local_peers,
                       DATA_ID_T data_id_t,
                       std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
: app_id(app_id), base_client_id(base_client_id), num_local_peers(num_local_peers), data_id_t(data_id_t),
  _app_id(base_client_id + app_id),
  ds_driver_ (boost::make_shared<DSDriver>(app_id, num_local_peers) )
  // lsdm_node_(boost::make_shared<SDMNode>(
  //   "s", true,
  //   app_id, lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport,
  //   boost::bind(&WADSDriver::handle_ri_msg, this, _1) ) )
{
  // Note: To give time to ri_manager to start the lsdm_server
  usleep(SLEEP_FOR_RI);
  lsdm_node_ = boost::make_shared<SDMNode>(
    "s", true,
    _app_id, lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport,
    boost::bind(&WADSDriver::handle_ri_msg, this, _1) );
  // 
  LOG(INFO) << "WADSDriver:: constructed; \n" << to_str();
}

WADSDriver::WADSDriver(int app_id, int base_client_id, int num_local_peers,
                       DATA_ID_T data_id_t, MPI_Comm& mpi_comm,
                       std::string lcontrol_lip, int lcontrol_lport, std::string joinhost_lcontrol_lip, int joinhost_lcontrol_lport)
: app_id(app_id), base_client_id(base_client_id), num_local_peers(num_local_peers), data_id_t(data_id_t),
  _app_id(base_client_id + app_id),
  ds_driver_ (boost::make_shared<DSDriver>(app_id, num_local_peers, mpi_comm) )
  // lsdm_node_(boost::make_shared<SDMNode>(
  //   "s", true,
  //   app_id, lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport,
  //   boost::bind(&WADSDriver::handle_ri_msg, this, _1) ) )
{
  // Note: To give time to ri_manager to start the lsdm_server
  usleep(SLEEP_FOR_RI);
  lsdm_node_ = boost::make_shared<SDMNode>(
    "s", true,
    _app_id, lcontrol_lip, lcontrol_lport, joinhost_lcontrol_lip, joinhost_lcontrol_lport,
    boost::bind(&WADSDriver::handle_ri_msg, this, _1) );
  // 
  LOG(INFO) << "WADSDriver:: constructed; \n" << to_str();
}

WADSDriver::~WADSDriver() { LOG(INFO) << "WADSDriver:: destructed."; }

std::string WADSDriver::to_str()
{
  std::stringstream ss;
  ss << "_app_id= " << _app_id << "\n"
     << "num_local_peers= " << num_local_peers << "\n"
     << "data_id_t= " << data_id_t << "\n"
     << "lsdm_node= \n" << lsdm_node_->to_str() << "\n";
  
  return ss.str();
}

int WADSDriver::put(std::string key, unsigned int ver, std::string data_type,
                    int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  if (ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "put:: ds_driver_->sync_put failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = PUT;
  msg_map["cl_id"] = boost::lexical_cast<std::string>(_app_id);
  msg_coder.encode_msg_map(msg_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_);
  if (lsdm_node_->send_msg_to_master(PACKET_RIMSG, msg_map) ) {
    LOG(ERROR) << "put:: lsdm_node_->send_msg_to_master failed; msg_map= " << patch_all::map_to_str<>(msg_map);
    return 1;
  }
  // To check if put was a success
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  unsigned int sync_point = patch_sdm::hash_str(PUT_REPLY + "_" + data_id);
  syncer.add_sync_point(sync_point, 1);
  syncer.wait(sync_point);
  syncer.del_sync_point(sync_point);
  
  if (data_id__ds_id_map[data_id] == -1) {
    data_id__ds_id_map.del(data_id);
    return 1;
  }
  data_id__ds_id_map.del(data_id);
  
  return 0;
}

int WADSDriver::get(bool blocking, std::string key, unsigned int ver, std::string data_type,
                    int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  LOG(INFO) << "get:: started for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  // 
  std::map<std::string, std::string> msg_map;
  if (blocking)
    msg_map["type"] = BLOCKING_GET;
  else
    msg_map["type"] = GET;
  msg_map["cl_id"] = boost::lexical_cast<std::string>(_app_id);
  msg_coder.encode_msg_map(msg_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_);
  if (lsdm_node_->send_msg_to_master(PACKET_RIMSG, msg_map) ) {
    LOG(ERROR) << "get:: lsdm_node_->send_msg_to_master failed; msg_map= " << patch_all::map_to_str<>(msg_map);
    return 1;
  }
  // 
  // usleep(1000);
  std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
  unsigned int sync_point;
  if (blocking)
    sync_point = patch_sdm::hash_str(BLOCKING_GET_REPLY + "_" + data_id);
  else
    sync_point = patch_sdm::hash_str(GET_REPLY + "_" + data_id);
  syncer.add_sync_point(sync_point, 1);
  syncer.wait(sync_point);
  syncer.del_sync_point(sync_point);
  
  if (data_id__ds_id_map[data_id] == -1) {
    data_id__ds_id_map.del(data_id);
    return 1;
  }
  
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "get:: ds_driver_->get failed; " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
    return 1;
  }
  data_id__ds_id_map.del(data_id);
  
  return 0;
}

void WADSDriver::handle_ri_msg(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_ri_msg:: msg_map= \n" << patch_all::map_to_str<>(msg_map);
  
    int ndim;
    std::string key;
    unsigned int ver;
    COOR_T *lb_, *ub_;
    if (msg_coder.decode_msg_map(msg_map, ndim, key, ver, lb_, ub_) ) {
      LOG(ERROR) << "handle_ri_reply:: msg_coder.decode_msg_map failed for msg_map= \n" << patch_all::map_to_str<>(msg_map);
      patch_all::free_all<COOR_T>(2, lb_, ub_);
      return;
    }
    
    std::string data_id = patch_sdm::get_data_id(data_id_t, key, ver, lb_, ub_);
    data_id__ds_id_map[data_id] = boost::lexical_cast<int>(msg_map["ds_id"] );
    
    unsigned int sync_point = patch_sdm::hash_str(msg_map["type"] + "_" + data_id);
    syncer.notify(sync_point);
    
    patch_all::free_all<COOR_T>(2, lb_, ub_);
}
