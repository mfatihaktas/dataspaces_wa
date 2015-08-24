#ifndef _DS_CLIENT_H_
#define _DS_CLIENT_H_

// #define _GRIDFTP_
#include "patch_ds.h"
#include "ds_drive.h"
#include "sdm_control.h"
#include "prefetch.h"
#include "profiler.h"
#include "ib_delivery.h"
#ifdef _GRIDFTP_
#include "gftp_delivery.h"
#endif // _GRIDFTP_

// Server side of blocking communication channel over dataspaces
// Single server <-> many clients
typedef boost::function<void(char*)> function_cb_on_recv;
class BCServer {
  public:
    BCServer(int app_id, int num_clients, int msg_size, 
             std::string base_comm_var_name, function_cb_on_recv f_cb,
             boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~BCServer();
    void init_listen_client(int client_id);
    void init_listen_all();
    void reinit_listen_client(int client_id);
  private:
    int app_id, num_clients, msg_size;
    std::string base_comm_var_name;
    function_cb_on_recv f_cb;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

// Client side of blocking communication channel over dataspaces
class BCClient {
  public:
    BCClient(int app_id, int num_others, int max_msg_size, 
             std::string base_comm_var_name,
             boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~BCClient();
    int send(std::map<std::string, std::string> msg_map);
  private:
    int app_id, num_others, max_msg_size;
    std::string base_comm_var_name;
    std::string comm_var_name;
    MsgCoder msg_coder;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

struct RSTable { // Remote Subscription
  public:
    RSTable();
    ~RSTable();
    int push_subscriber(std::string key, unsigned int ver, char ds_id);
    int pop_subscriber(std::string key, unsigned int ver, char& ds_id);
  private:
    patch_sdm::thread_safe_map<key_ver_pair, std::vector<char> > key_ver__ds_id_vector_map;
};

struct GFTPBTable { // Gridftp Bootstrap
  public:
    GFTPBTable();
    ~GFTPBTable();
    int add(char ds_id, std::string laddr, std::string lport, std::string tmpfs_dir);
    int del(char ds_id);
    int get(char ds_id, std::string &laddr, std::string &lport, std::string &tmpfs_dir);
    bool contains(char ds_id);
  private:
    patch_sdm::thread_safe_map<char, std::string> dsid_laddr_map;
    patch_sdm::thread_safe_map<char, std::string> dsid_lport_map;
    patch_sdm::thread_safe_map<char, std::string> dsid_tmpfsdir_map;
};

const std::string INFINIBAND = "i";
const std::string GRIDFTP = "g";

class RFPManager { // Remote Fetch & Place
  public:
    RFPManager(std::string wa_trans_protocol, boost::shared_ptr<DSpacesDriver> ds_driver_,
               std::list<std::string> wa_ib_lport_list, std::string wa_gftp_lintf, 
               std::string wa_gftp_lport, std::string tmpfs_dir);
    ~RFPManager();
    void init_gftp_server();
    
    int wa_get(std::string laddr, std::string lport, std::string tmpfs_dir,
               std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int wa_put(std::string key, unsigned int ver, std::string data_type,
               int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
               std::string laddr, std::string lport, std::string tmpfs_dir);
    
#ifdef _GRIDFTP_
    int gftp_get__ds_put(std::string gftps_laddr, std::string gftps_lport, std::string gftps_tmpfs_dir,
                         std::string key, unsigned int ver,
                         int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int ds_get__gftp_put(std::string key, unsigned int ver,
                         int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                         std::string gftps_laddr, std::string gftps_lport, std::string gftps_tmpfs_dir);
    int gftpfile_read__ds_put(std::string key, unsigned int ver,
                              int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
#endif // _GRIDFTP_
    std::string get_lport();
    int ib_receive__ds_put(std::string ib_laddr, std::string ib_lport,
                           std::string key, unsigned int ver, std::string data_type,
                           int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    void handle_ib_receive(std::string key, unsigned int ver, size_t data_size, void* data_);
    int ds_get__ib_send(std::string key, unsigned int ver, std::string data_type, 
                        int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                        const char* ib_laddr, const char* ib_lport);
    size_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
  private:
    std::string wa_trans_protocol;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<IBDDManager> ibdd_manager_;
#ifdef _GRIDFTP_
    boost::shared_ptr<GFTPDDManager> gftpdd_manager_;
#endif // _GRIDFTP_
    std::map<key_ver_pair, size_t> key_ver__recvedsize_map;
    std::map<key_ver_pair, void*> key_ver__data_map;
};

/*******************************************  RIManager  ******************************************/
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
    
    boost::shared_ptr<DSpacesDriver> ds_driver_;
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

#endif //end of _DS_CLIENT_H_
