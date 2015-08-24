#ifndef _REMOTE_INTERACT_H_
#define _REMOTE_INTERACT_H_

/******************************************  RFPManager  ******************************************/
const std::string INFINIBAND = "i";
const std::string GRIDFTP = "g";

typedef boost::function<void(RECV_ID_T, int, void*)> data_recv_cb_func;

class RFPManager { // Remote Fetch & Place
  private:
    char data_id_t;
    std::string trans_protocol;
    
    boost::shared_ptr<TManager> t_manager_;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    
    std::map<std::string, int> data_id__recved_size_map;
    std::map<std::string, void*> data_id__data_map;
  public:
    RFPManager(char data_id_t, std::string trans_protocol,
               std::string ib_laddr, std::list<std::string> ib_lport_list,
               std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir,
               boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~RFPManager();
    
    int wa_put(std::string laddr, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int wa_get(std::string laddr, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    
    void handle_recv(std::string data_id, size_t data_size, void* data_);
    // std::string get_lport();
};

/*******************************************  RIManager  ******************************************/
const size_t CL__RIMANAGER_MAX_MSG_SIZE = 1000;

class RIManager {
  protected:
    //ImpRem: Since handle_ core functions are called by client threads, properties must be thread-safe
    int cl_id, num_client;
    char ds_id;
    std::string trans_protocol, data_laddr, data_lport;
    
    MsgCoder msg_coder;
    
    boost::shared_ptr<SDMSlave> sdm_slave_;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCServer> bc_server_;
    patch_sdm::thread_safe_map<int, boost::shared_ptr<BCClient> > cl_id__bc_client_map; //TODO: prettify
    boost::shared_ptr<RFPManager> rfp_manager_;
    
    boost::asio::io_service io_service;
    boost::asio::signal_set signals;
    
    patch_sdm::syncer<unsigned int> ri_man_syncer;
    
  public:
    RIManager(int cl_id, int num_client, char data_id_t,
              char ds_id, std::string control_lip, int control_lport, std::string join_control_lip, int join_control_lport,
              std::string data_trans_protocol, std::string ib_laddr, std::list<std::string> ib_lport_list,
              std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir);
    ~RIManager();
    int close();
    std::string to_str();
    
    void handle_app_req(char* app_req);
    void handle_get(bool blocking, int cl_id, std::map<std::string, std::string> get_map);
    // int remote_get(char ds_id, std::map<std::string, std::string> r_get_map);
    void handle_put(int p_id, std::map<std::string, std::string> put_map);
    // void handle_del(key_ver_pair kv);
    
    void handle_ri_msg(std::map<std::string, std::string> ri_msg_map);
    // void handle_rfetch(std::map<std::string, std::string> r_fetch_map);
    // void handle_rplace(std::map<std::string, std::string> r_place_map);
    // void handle_rp_reply(std::map<std::string, std::string> rp_reply_map);
    
    // int remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map);
    // void handle_wa_get(std::string called_from, std::map<std::string, std::string> str_str_map);
    // int remote_place(std::string key, unsigned int ver, char to_id);
    
    void handle_dm_act(std::map<std::string, std::string> dm_act_map);
};



#endif //end of _REMOTE_INTERACT_H_