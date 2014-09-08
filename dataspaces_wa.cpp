#include "dataspaces_wa.h"

void wa_print_str_map(std::map<std::string, std::string> str_map)
{
  for (std::map<std::string, std::string>::const_iterator it=str_map.begin(); 
       it!=str_map.end(); ++it){
    std::cout << "\t" << it->first << ":" << it->second << "\n";
  }
}

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
  li_bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "li_req_", ds_driver_) ),
  ri_bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "ri_req_", ds_driver_) )
{
  //usleep(WAIT_TIME_FOR_BCSERVER_DSLOCK);
  //
  LOG(INFO) << "WADspacesDriver:: constructed.";
}

WADspacesDriver::~WADspacesDriver()
{
  //
  LOG(INFO) << "WADspacesDriver:: destructed.";
}

int WADspacesDriver::local_put(std::string data_type, std::string key, unsigned int ver, int size,
                               int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  std::map<std::string, std::string> msg_map = rmessenger.gen_i_msg(LOCAL_PUT, app_id, data_type,
                                                                    key, ver, size, ndim, gdim_, lb_, ub_);
  if (li_bc_client_->send(msg_map) ){
    LOG(ERROR) << "local_put:: li_bc_client_->send failed!";
    return 1;
  }
  
  return ds_driver_->sync_put(key.c_str(), ver, size, ndim, gdim_, lb_, ub_, data_);
}

int WADspacesDriver::remote_get(std::string data_type, std::string key, unsigned int ver, int size,
                                int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  std::map<std::string, std::string> msg_map = rmessenger.gen_i_msg(REMOTE_GET, app_id, data_type, 
                                                                    key, ver, size, ndim, gdim_, lb_, ub_);
  if (ri_bc_client_->send(msg_map)){
    LOG(ERROR) << "remote_get:: ri_bc_client_->send failed!";
    return 1;
  }
  //
  boost::shared_ptr<BCServer> bc_server_( 
    new BCServer(app_id, 0, RI_MAX_MSG_SIZE, "ri_reply_", 
                 boost::bind(&WADspacesDriver::handle_ri_reply, this, _1),
                 ds_driver_)
  );
  bc_server_->init_listen_client(app_id);
  
  rg_syncer.add_sync_point(key, 1);
  rg_syncer.wait(key);
  rg_syncer.del_sync_point(key);
  //
  LOG(INFO) << "remote_get:: done.";
}

int WADspacesDriver::handle_ri_reply(char* ri_reply)
{
  std::map<std::string, std::string> ri_reply_map = imsg_coder.decode(ri_reply);
  
  LOG(INFO) << "handle_ri_reply:: ri_reply_map=";
  wa_print_str_map(ri_reply_map);
  
  std::string key = ri_reply_map["key"];
  
  rg_syncer.notify(key);
  //
  LOG(INFO) << "handle_ri_reply:: done.";
}