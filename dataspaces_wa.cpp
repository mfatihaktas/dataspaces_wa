#include "dataspaces_wa.h"

/***************************************  WADSDriver  ****************************************/
// Note: Reason why base_client_id is added in BCClient's arg rather than eliminating it by taking 
// app_id arg as base_client_id + app_id is because DS throws segmentation fault in a weird way while
// initializing it with arbitrary app_id's.
// Overall, app_id is used for DSDriver, _app_id is used as the application id
WADSDriver::WADSDriver(int app_id, int base_client_id, int num_local_peers,
                       char data_id_t)
: app_id(app_id), base_client_id(base_client_id), num_local_peers(num_local_peers), data_id_t(data_id_t),
  _app_id(base_client_id + app_id),
  ds_driver_ (boost::make_shared<DSDriver>(num_local_peers, app_id) ),
  bc_client_(boost::make_shared<BCClient>(_app_id, RI_MSG_SIZE, "req_app_", ds_driver_) )
{
  // 
  LOG(INFO) << "WADSDriver:: constructed.";
}

WADSDriver::WADSDriver(int app_id, int base_client_id, int num_local_peers,
                       char data_id_t, MPI_Comm mpi_comm)
: app_id(app_id), base_client_id(base_client_id), num_local_peers(num_local_peers), data_id_t(data_id_t),
  _app_id(base_client_id + app_id),
  ds_driver_ (boost::make_shared<DSDriver>(mpi_comm, num_local_peers, app_id) ),
  bc_client_(boost::make_shared<BCClient>(_app_id, RI_MSG_SIZE, "req_app_", ds_driver_) )
{
  // 
  LOG(INFO) << "WADSDriver:: constructed.";
}

WADSDriver::~WADSDriver() { LOG(INFO) << "WADSDriver:: destructed."; }

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
  if (bc_client_->send(msg_map) ) {
    LOG(ERROR) << "put:: bc_client_->send failed; msg_map= " << patch_sfc::map_to_str<>(msg_map);
    return 1;
  }
  
  return 0;
}

int WADSDriver::get(bool blocking, std::string key, unsigned int ver, std::string data_type,
                    int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  LOG(INFO) << "get:: started for " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
  
  std::map<std::string, std::string> msg_map;
  if (blocking)
    msg_map["type"] = BLOCKING_GET;
  else
    msg_map["type"] = GET;
  msg_map["cl_id"] = boost::lexical_cast<std::string>(_app_id);
  msg_coder.encode_msg_map(msg_map, key, ver, data_type, size, ndim, gdim_, lb_, ub_);
  if (bc_client_->send(msg_map) ) {
    LOG(ERROR) << "get:: bc_client_->send failed!";
    return 1;
  }
  // usleep(1000);
  // 
  boost::shared_ptr<BCServer> bc_server_ = 
    boost::make_shared<BCServer>(_app_id, CL__RIMANAGER_MAX_MSG_SIZE, "reply_app_",
                                 boost::bind(&WADSDriver::handle_ri_reply, this, _1), ds_driver_);
  bc_server_->init_listen_client(_app_id);
  
  std::string data_id = get_data_id(data_id_t, key, ver, lb_, ub_);
  unsigned int sync_point = patch_sdm::hash_str(msg_map["type"] + "_" + data_id);
  syncer.add_sync_point(sync_point, 1);
  syncer.wait(sync_point);
  syncer.del_sync_point(sync_point);
  
  if (data_id__ds_id_map[data_id] == '?') {
    LOG(INFO) << "get:: " << KV_LUCOOR_TO_STR(key, ver, lb_, ub_);
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

int WADSDriver::handle_ri_reply(char* ri_reply_)
{
  std::map<std::string, std::string> ri_reply_map = msg_coder.decode(ri_reply_);
  LOG(INFO) << "handle_ri_reply:: ri_reply_map= \n" << patch_sfc::map_to_str<>(ri_reply_map);
  
  int ndim;
  std::string key;
  unsigned int ver;
  COOR_T *lb_, *ub_;
  if (msg_coder.decode_msg_map(ri_reply_map, ndim, key, ver, lb_, ub_) ) {
    LOG(ERROR) << "handle_ri_reply:: msg_coder.decode_msg_map failed for ri_reply_map= \n" << patch_sfc::map_to_str<>(ri_reply_map);
    return 1;
  }
  
  std::string data_id = get_data_id(data_id_t, key, ver, lb_, ub_);
  data_id__ds_id_map[data_id] = boost::lexical_cast<char>(ri_reply_map["ds_id"] );
  
  unsigned int sync_point = patch_sdm::hash_str(ri_reply_map["type"] + "_" + data_id);
  syncer.notify(sync_point);
  
  return 0;
}
