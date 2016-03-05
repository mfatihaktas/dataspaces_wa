#include "ds_client.h"

/*******************************************  BCServer  *******************************************/
BCServer::BCServer(int app_id, int base_client_id, int num_client, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), base_client_id(base_client_id), num_client(num_client), msg_size(msg_size),
  base_comm_var_name(base_comm_var_name), f_cb(f_cb),
  ds_driver_(ds_driver_)
{
  // 
  log_(INFO, "constructed;\n" << to_str() )
}

BCServer::BCServer(int app_id, int msg_size,
                   std::string base_comm_var_name, function_cb_on_recv f_cb,
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), base_client_id(0), num_client(0), msg_size(msg_size),
  base_comm_var_name(base_comm_var_name), f_cb(f_cb),
  ds_driver_(ds_driver_)
{
  // 
  log_(INFO, "constructed;" << to_str() )
}

BCServer::~BCServer()
{
  // ds_driver_->finalize();
  // 
  log_(INFO, "destructed.")
}

std::string BCServer::to_str()
{
  std::stringstream ss;
  ss << "\t base_comm_var_name= " << base_comm_var_name << "\n"
     << "\t app_id= " << app_id << "\n"
     << "\t base_client_id= " << base_client_id << "\n"
     << "\t num_client= " << num_client << "\n";
  
  return ss.str();
}

void BCServer::init_listen_all()
{
  //Assume app_id of each client app is ordered as 1,2,...,num_client
  for (int i = 1; i <= num_client; i++)
    init_listen_client(base_client_id + i);
  // 
  log_(INFO, "done.")
}

void BCServer::init_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  // 
  ds_driver_->reg_cb_on_get(key, f_cb);
  ds_driver_->init_get_thread(key, msg_size);
  // 
  log_(INFO, "done for client_id= " << client_id)
}

void BCServer::reinit_listen_client(int client_id)
{
  std::string key = base_comm_var_name + boost::lexical_cast<std::string>(client_id);
  
  ds_driver_->init_get_thread(key, msg_size);
  // 
  log_(INFO, "done for client_id= " << client_id)
}

/*******************************************  BCClient  *******************************************/
BCClient::BCClient(int app_id, int max_msg_size,
                   std::string base_comm_var_name, 
                   boost::shared_ptr<DSDriver> ds_driver_)
: app_id(app_id), max_msg_size(max_msg_size),
  base_comm_var_name(base_comm_var_name),
  ds_driver_(ds_driver_)
{
  comm_var_name = base_comm_var_name + boost::lexical_cast<std::string>(app_id);
  //ds_driver_->lock_on_write(comm_var_name.c_str() );
  // 
  log_(INFO, "constructed for comm_var_name= " << comm_var_name)
}

BCClient::~BCClient()
{
  //ds_driver_->finalize();
  // 
  log_(INFO, "destructed.")
}

int BCClient::send(std::map<std::string, std::string> msg_map)
{
  std::string msg_str;
  if (msg_coder.encode(msg_map, msg_str) ) {
    log_(ERROR, "msg_coder.encode failed; msg_map= \n" << patch::map_to_str<>(msg_map) )
    return 1;
  }
  
  int msg_size = msg_str.size();
  if (msg_size > max_msg_size) {
    log_(ERROR, "msg_size= " << msg_size << " > max_msg_size= " << max_msg_size)
    return 1;
  }
  
  // 1 dimensional char array
  // uint64_t gdim = 0; //max_msg_size;
  // uint64_t lb = 0;
  // uint64_t ub = 0; //max_msg_size-1;
  uint64_t* gdim_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* lb_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* ub_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  for (int i = 0; i < 3; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  
  char *data_ = (char*)malloc(max_msg_size*sizeof(char) );
  strcpy(data_, msg_str.c_str() );
  for (int i = msg_size; i < max_msg_size - msg_size; i++)
    data_[i] = '\0';
  
  int result = ds_driver_->sync_put(comm_var_name.c_str(), 0, max_msg_size*sizeof(char), 3, gdim_, lb_, ub_, data_);
  // int result = ds_driver_->sync_put_without_lock(comm_var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data_);
  free(data_);
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  
  return result;
}
