#ifndef _DS_CLIENT_H_
#define _DS_CLIENT_H_

#include "ds_drive.h"
#include "patch_ds.h"
#include "patch_sdm.h"

/*******************************************  BCServer  *******************************************/
// Server side of blocking communication channel over dataspaces
// Single server <-> many clients
typedef boost::function<void(char*)> function_cb_on_recv;
class BCServer {
  private:
    int app_id, base_client_id, num_client, msg_size;
    std::string base_comm_var_name;
    function_cb_on_recv f_cb;
    boost::shared_ptr<DSDriver> ds_driver_;
  public:
    BCServer(int app_id, int base_client_id, int num_client, int msg_size,
             std::string base_comm_var_name, function_cb_on_recv f_cb,
             boost::shared_ptr<DSDriver> ds_driver_);
    BCServer(int app_id, int msg_size,
             std::string base_comm_var_name, function_cb_on_recv f_cb,
             boost::shared_ptr<DSDriver> ds_driver_);
    
    ~BCServer();
    // int close();
    std::string to_str();
    
    void init_listen_client(int client_id);
    void init_listen_all();
    void reinit_listen_client(int client_id);
};

/*******************************************  BCClient  *******************************************/
// Client side of blocking communication channel over dataspaces
class BCClient {
  private:
    int app_id, max_msg_size;
    std::string base_comm_var_name;
    std::string comm_var_name;
    patch_sdm::MsgCoder msg_coder;
    boost::shared_ptr<DSDriver> ds_driver_;
  public:
    BCClient(int app_id, int max_msg_size, 
             std::string base_comm_var_name,
             boost::shared_ptr<DSDriver> ds_driver_);
    ~BCClient();
    // int close();
    
    int send(std::map<std::string, std::string> msg_map);
};

#endif //end of _DS_CLIENT_H_
