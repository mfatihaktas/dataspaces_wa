#ifndef _DS_CLIENT_H_
#define _DS_CLIENT_H_

#include "ds_drive.h"
#include "sdm.h"

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
    MsgCoder msg_coder;
    boost::shared_ptr<DSDriver> ds_driver_;
  public:
    BCClient(int app_id, int max_msg_size, 
             std::string base_comm_var_name,
             boost::shared_ptr<DSDriver> ds_driver_);
    ~BCClient();
    // int close();
    
    int send(std::map<std::string, std::string> msg_map);
};

/*******************************************  RIManager  ******************************************/
/*
const size_t APP_RIMANAGER_MAX_MSG_SIZE = 1000;

const std::string GET = "g";
const std::string GET_REPLY = "g_reply";
const std::string BLOCKING_GET = "bg";
const std::string BLOCKING_GET_REPLY = "bg_reply";
const std::string PUT = "p";
const std::string PUT_REPLY = "p_reply";

const std::string RI_RQUERY = "rq";
const std::string RI_RBQUERY = "rbq";
const std::string RI_RQUERY_REPLY = "rq_reply";
const std::string RI_RFETCH = "rf";
const std::string RI_RQTABLE = "rrqt";
const std::string RI_RPLACE = "rp";
const std::string RI_RPLACE_REPLY = "rp_reply";
#ifdef _GRIDFTP_
const std::string RI_GFTPPUT_DONE = "gp_done";
const std::string RI_GFTP_BPING = "gb_ping";
const std::string RI_GFTP_BPONG = "gb_pong";
#endif // _GRIDFTP_

typedef std::pair<std::string, std::string> laddr_lport_pair;
typedef std::pair<laddr_lport_pair, std::string> laddr_lport__tmpfsdir_pair;
class RIManager {
  private:
    //ImpRem: Since handle_ core functions are called by client threads, properties must be thread-safe
    int app_id, num_cnodes;
    char id;
    std::string wa_trans_protocol, wa_gftp_lintf, wa_laddr, wa_gftp_lport, tmpfs_dir; //RIManager needs these for gftpb process
    MsgCoder msg_coder;
    
    boost::shared_ptr<DSDriver> ds_driver_;
    boost::shared_ptr<BCServer> bc_server_;
    boost::shared_ptr<SDMNode> sdm_node_;
    boost::shared_ptr<RFPManager> rfp_manager_;
    boost::shared_ptr<PBuffer> pbuffer_;
    patch_sdm::thread_safe_map<int, boost::shared_ptr<BCClient> > appid_bcclient_map; //TODO: prettify
    
    boost::asio::io_service io_service;
    boost::asio::signal_set signals;
    
    KVTable rq_table;
    patch_sdm::syncer<key_ver_pair> rq_syncer;
    
    patch_sdm::thread_safe_map<key_ver_pair, laddr_lport__tmpfsdir_pair> key_ver___laddr_lport__tmpfsdir_map;
    patch_sdm::syncer<key_ver_pair> rp_syncer;
    
    RSTable rs_table;
    patch_sdm::syncer<key_ver_pair> handle_rp_syncer;
    
    patch_sdm::syncer<key_ver_pair> bget_syncer;
    
    patch_sdm::syncer<key_ver_pair> rf_wa_get_syncer;
    patch_sdm::syncer<key_ver_pair> rp_wa_get_syncer;
    
    patch_sdm::thread_safe_vector<key_ver_pair> key_ver_being_fetched_vector;
    patch_sdm::syncer<key_ver_pair> being_fetched_syncer;
    
    TProfiler<key_ver_pair> rget_time_profiler;
    // 
    GFTPBTable gftpb_table;
    patch_sdm::syncer<char> gftp_bping_syncer;
  public:
    
};
*/

#endif //end of _DS_CLIENT_H_
