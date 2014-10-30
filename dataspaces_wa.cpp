#include "dataspaces_wa.h"

//***********************************  RMessenger  ********************************//
RMessenger::RMessenger()
{
  //
  LOG(INFO) << "RMessenger:: constructed.";
}

RMessenger::~RMessenger()
{
  //
  LOG(INFO) << "RMessenger:: destructed.";
}

std::map<std::string, std::string> RMessenger::gen_i_msg(std::string msg_type, int app_id, 
                                                         std::string data_type, std::string key,
                                                         unsigned int ver, int size, int ndim, 
                                                         uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["type"] = msg_type;
  msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  msg_map["data_type"] = data_type;
  msg_map["key"] = key;
  msg_map["ver"] = boost::lexical_cast<std::string>(ver);
  msg_map["size"] = boost::lexical_cast<std::string>(size);
  msg_map["ndim"] = boost::lexical_cast<std::string>(ndim);
  
  std::string gdim = "";
  std::string lb = "";
  std::string ub = "";
  for(int i=0; i<ndim; i++){
    gdim += boost::lexical_cast<std::string>(gdim_[i]);
    lb += boost::lexical_cast<std::string>(lb_[i]);
    ub += boost::lexical_cast<std::string>(ub_[i]);
    if (i<ndim-1){
      gdim += ",";
      lb += ",";
      ub += ",";
    }
  }
  msg_map["gdim"] = gdim;
  msg_map["lb"] = lb;
  msg_map["ub"] = ub;
  
  return msg_map;
}

//********************************  WADspacesDriver  ******************************//
WADspacesDriver::WADspacesDriver(int app_id, int num_local_peers)
: app_id(app_id),
  num_local_peers(num_local_peers),
  ds_driver_ ( new DSpacesDriver(num_local_peers, app_id) ),
  bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "req_app_", ds_driver_) )
{
  //
  LOG(INFO) << "WADspacesDriver:: constructed.";
}

WADspacesDriver::WADspacesDriver(MPI_Comm mpi_comm, int app_id, int num_local_peers)
: app_id(app_id),
  num_local_peers(num_local_peers),
  ds_driver_ ( new DSpacesDriver(mpi_comm, num_local_peers, app_id) ),
  bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "req_app_", ds_driver_) )
{
  //
  LOG(INFO) << "WADspacesDriver:: constructed.";
}

WADspacesDriver::~WADspacesDriver()
{
  //
  LOG(INFO) << "WADspacesDriver:: destructed.";
}

void WADspacesDriver::print_str_map(std::map<std::string, std::string> str_map)
{
  for (std::map<std::string, std::string>::const_iterator it=str_map.begin(); 
       it!=str_map.end(); ++it){
    std::cout << "\t" << it->first << ":" << it->second << "\n";
  }
}

int WADspacesDriver::put(std::string data_type, std::string key, unsigned int ver, int size,
                         int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  std::map<std::string, std::string> msg_map = rmessenger.gen_i_msg(PUT, app_id, data_type,
                                                                    key, ver, size, ndim, gdim_, lb_, ub_);
  if (bc_client_->send(msg_map) ){
    LOG(ERROR) << "put:: bc_client_->send failed!";
    return 1;
  }
  
  return ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_);
}

int WADspacesDriver::get(bool blocking, std::string data_type, std::string key, unsigned int ver, 
                         int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  LOG(INFO) << "get:: started for <key= " << key << ", ver= " << ver << ">.";
  std::string msg_type;
  if (blocking) {
    msg_type = BLOCKING_GET;
  }
  else {
    msg_type = GET;
  }
  std::map<std::string, std::string> msg_map = rmessenger.gen_i_msg(msg_type, app_id, data_type, 
                                                                    key, ver, size, ndim, gdim_, lb_, ub_);
  
  if (bc_client_->send(msg_map) ) {
    LOG(ERROR) << "get:: bc_client_->send failed!";
    return 1;
  }
  //
  boost::shared_ptr<BCServer> bc_server_( 
    new BCServer(app_id, 0, APP_RIMANAGER_MAX_MSG_SIZE, "reply_app_", 
                 boost::bind(&WADspacesDriver::handle_ri_reply, this, _1),
                 ds_driver_)
  );
  bc_server_->init_listen_client(app_id);
  
  key_ver_pair kv = std::make_pair(key, ver);
  rg_syncer.add_sync_point(kv, 1);
  rg_syncer.wait(kv);
  rg_syncer.del_sync_point(kv);
  
  if (key_ver__dsid_map[kv] == '?'){
    LOG(INFO) << "get:: <key= " << key << ", ver= " << ver << "> does not exist";
    key_ver__dsid_map.del(kv);
    return 1;
  }
  
  if (ds_driver_->get(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_) ){
    LOG(ERROR) << "get:: ds_driver_->get failed!";
    return 1;
  }
  key_ver__dsid_map.del(kv);
  //
  LOG(INFO) << "get:: done for <key= " << key << ", ver= " << ver << ">.";
  return 0;
}

int WADspacesDriver::handle_ri_reply(char* ri_reply)
{
  std::map<std::string, std::string> ri_reply_map = imsg_coder.decode(ri_reply);
  
  LOG(INFO) << "handle_ri_reply:: ri_reply_map=";
  print_str_map(ri_reply_map);
  
  std::string key = ri_reply_map["key"];
  unsigned int ver = boost::lexical_cast<unsigned int>(ri_reply_map["ver"]);
  
  key_ver_pair kv = std::make_pair(key, ver);
  key_ver__dsid_map[kv] = ri_reply_map["ds_id"].c_str()[0];
  
  rg_syncer.notify(kv);
  //
  LOG(INFO) << "handle_ri_reply:: done.";
  return 0;
}