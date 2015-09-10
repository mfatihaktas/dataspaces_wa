#ifndef _REMOTE_INTERACT_H_
#define _REMOTE_INTERACT_H_

#include "profiler.h"
#include "trans.h"
#include "sdm_control.h"
#include "ds_client.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

/******************************************  RFPManager  ******************************************/
typedef boost::function<void(RECV_ID_T, int, void*)> data_recv_cb_func;

class RFPManager { // Remote Fetch & Place
  private:
    char data_id_t;
    
    boost::shared_ptr<TManager> t_manager_;
    boost::shared_ptr<DSDriver> ds_driver_;
    
    std::map<std::string, int> data_id__recved_size_map;
    std::map<std::string, void*> data_id__data_map;
  public:
    RFPManager(char data_id_t, std::string trans_protocol,
               std::string ib_laddr, std::list<std::string> ib_lport_list,
               std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir,
               boost::shared_ptr<DSDriver> ds_driver_);
    ~RFPManager();
    std::string to_str();
    
    std::string get_laddr();
    std::string get_lport();
    std::string get_tmpfs_dir();
    int get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
    
    int wa_put(std::string laddr, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int wa_get(std::string laddr, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    
    void handle_recv(std::string data_id, int data_size, void* data_);
};

/*******************************************  RIManager  ******************************************/
// App - RI
const std::string GET = "g";
const std::string GET_REPLY = "g_reply";
const std::string BLOCKING_GET = "bg";
const std::string BLOCKING_GET_REPLY = "bg_reply";
const std::string PUT = "p";
const std::string PUT_REPLY = "p_reply";
const std::string DEL = "d";
const std::string DEL_REPLY = "d_reply";
// RI - RI
const std::string RI_TINFO_QUERY = "ri_tiq";
const std::string RI_TINFO_QUERY_REPLY = "ri_tiqr";
const std::string RI_GRIDFTP_PUT = "ri_gp";

const int CL__RIMANAGER_MAX_MSG_SIZE = 1000;

class RIManager {
  struct t_info {
    std::string laddr, lport, tmpfs_dir;
    
    t_info(std::string laddr, std::string lport, std::string tmpfs_dir)
    : laddr(laddr), lport(lport), tmpfs_dir(tmpfs_dir)
    {}
  };
  
  protected:
    //ImpRem: Since handle_ core functions are called by client threads, properties must be thread-safe
    int cl_id, base_client_id, num_client;
    char ds_id;
    std::string data_trans_protocol;
    
    MsgCoder msg_coder;
    
    boost::shared_ptr<SDMSlave> sdm_slave_;
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<BCServer> bc_server_;
    patch_sdm::thread_safe_map<int, boost::shared_ptr<BCClient> > cl_id__bc_client_map; //TODO: prettify
    boost::shared_ptr<RFPManager> rfp_manager_;
    patch_sdm::thread_safe_map<char, boost::shared_ptr<t_info> > ds_id__t_info_map;
    
    patch_sdm::syncer<unsigned int> ri_syncer;
    
    boost::asio::io_service io_service;
    boost::asio::signal_set signals;
  public:
    RIManager(int cl_id, int base_client_id, int num_client,
              char ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
              std::string data_trans_protocol, std::string ib_laddr, std::list<std::string> ib_lport_list,
              std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir,
              char predictor_t, int pbuffer_size, int pexpand_length, COOR_T* lcoor_, COOR_T* ucoor_);
    ~RIManager();
    void close();
    std::string to_str();
    
    void handle_app_req(char* app_req);
    void handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map);
    // int remote_get(char ds_id, std::map<std::string, std::string> r_get_map);
    void handle_put(int p_id, std::map<std::string, std::string> put_map);
    // void handle_del(key_ver_pair kv);
    
    int t_info_query(char to_id);
    void handle_rimsg(std::map<std::string, std::string> rimsg_map);
    void handle_tinfo_query(std::map<std::string, std::string> msg_map);
    void remote_get(std::map<std::string, std::string> msg_map);
    void remote_put(std::map<std::string, std::string> msg_map);
    void handle_tinfo_query_reply(std::map<std::string, std::string> msg_map);
    void handle_gridftp_put(std::map<std::string, std::string> msg_map);
    
    void handle_dm_act(std::map<std::string, std::string> dm_act_map);
    void handle_dm_move(std::map<std::string, std::string> msg_map);
};

#endif //end of _REMOTE_INTERACT_H_