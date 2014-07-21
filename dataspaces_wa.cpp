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

std::map<std::string, std::string> RMessenger::gen_remote_get(int app_id, std::string var_name, unsigned int ver, 
                                                              int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_)
{
  std::map<std::string, std::string> msg_map;
  msg_map["app_id"] = boost::lexical_cast<std::string>(app_id);
  msg_map["type"] = "r_get";
  msg_map["var_name"] = var_name;
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
  bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "ri_req_") )
{
  //
  LOG(INFO) << "WADspacesDriver:: constructed.";
}

WADspacesDriver::~WADspacesDriver()
{
  //
  LOG(INFO) << "WADspacesDriver:: destructed.";
}

int WADspacesDriver::remote_get(std::string var_name, unsigned int ver, int size,
                                int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  std::map<std::string, std::string> msg_map = rmessenger.gen_remote_get(app_id, var_name, ver,  
                                                                         size, ndim, gdim_, lb_, ub_);
  bc_client_->send("r_get", msg_map);
  
  
}
