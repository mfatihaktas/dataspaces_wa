#ifndef _DSCLIENT_H_
#define _DSCLIENT_H_

// #define _GRIDFTP_

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <unistd.h>

#include <string>
#include <cstdarg> // For variable argument lists
#include <csignal> // For wait signal

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
// For boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>

#include "patch_ds.h"
#include "ds_drive.h"
// #include "dht_node.h"
#include "sdm_node.h"
#include "packet.h"

#include "ib_delivery.h"
#ifdef _GRIDFTP_
#include "gftp_delivery.h"
#endif // _GRIDFTP_

#include "prefetch.h"
#include "profiler.h"

class IMsgCoder
{
  public:
    IMsgCoder();
    ~IMsgCoder();
    std::map<std::string, std::string> decode(char* msg);
    std::string encode(std::map<std::string, std::string> msg_map);
    int decode_msg_map(std::map<std::string, std::string> msg_map, 
                       std::string& key, unsigned int& ver, std::string& data_type,
                       int& size, int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);

    int encode_msg_map(std::map<std::string, std::string> &msg_map, 
                       std::string key, unsigned int ver, std::string data_type,
                       int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
};

// Server side of blocking communication channel over dataspaces
// Single server <-> many clients
// used by RIManager
typedef boost::function<void(char*)> function_cb_on_recv;

class BCServer
{
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
class BCClient
{
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
    IMsgCoder imsg_coder;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

typedef std::pair<std::string, unsigned int> key_ver_pair;
struct RQTable //Remote Query Table
{
  struct key_ver_info {
    public:
      int p_id;
      std::string data_type;
      char ds_id;
      int size, ndim;
      uint64_t *gdim_, *lb_, *ub_;
      
      key_ver_info(int p_id, std::string data_type, char ds_id,
                   int size, int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
      : p_id(p_id), data_type(data_type), ds_id(ds_id),
        size(size), ndim(ndim), gdim_(gdim_), lb_(lb_), ub_(ub_) {}
  };
  
  public:
    RQTable();
    ~RQTable();
    int get_key_ver(std::string key, unsigned int ver,
                    int& p_id, std::string &data_type, char &ds_id,
                    int &size, int &ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
    int put_from_map(std::map<std::string, std::string> map);
    int put_key_ver(std::string key, unsigned int ver,
                    int p_id, std::string data_type, char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int add_key_ver(std::string key, unsigned int ver,
                    int p_id, std::string data_type, char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int update_key_ver(std::string key, unsigned int ver,
                       int p_id, std::string data_type, char ds_id, int size, int ndim, 
                       uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int del_key_ver(std::string key, unsigned int ver);
    bool is_feasible_to_get(std::string key, unsigned int ver,
                            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    std::string to_str();
    std::map<std::string, std::string> to_str_str_map();
    
    int mark_all();
    std::map<std::string, std::string> to_unmarked_str_str_map();
  private:
    patch_ds::thread_safe_map<key_ver_pair, boost::shared_ptr<key_ver_info> > kv_info_map;
    patch_ds::thread_safe_map<key_ver_pair, bool> kv_mark_map;
};

struct RSTable //Remote Subscription Table
{
  public:
    RSTable();
    ~RSTable();
    int push_subscriber(std::string key, unsigned int ver, char ds_id);
    int pop_subscriber(std::string key, unsigned int ver, char& ds_id);
  private:
    patch_ds::thread_safe_map<key_ver_pair, std::vector<char> > key_ver__ds_id_vector_map;
};


struct GFTPBTable //Gridftp Bootstrap Table
{
  public:
    GFTPBTable();
    ~GFTPBTable();
    int add(char ds_id, std::string laddr, std::string lport, std::string tmpfs_dir);
    int del(char ds_id);
    int get(char ds_id, std::string &laddr, std::string &lport, std::string &tmpfs_dir);
    bool contains(char ds_id);
  private:
    patch_ds::thread_safe_map<char, std::string> dsid_laddr_map;
    patch_ds::thread_safe_map<char, std::string> dsid_lport_map;
    patch_ds::thread_safe_map<char, std::string> dsid_tmpfsdir_map;
};

const std::string INFINIBAND = "i";
const std::string GRIDFTP = "g";

class RFPManager //Remote Fetch & Place Manager
{
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
class RIManager
{
  private:
    //ImpRem: Since handle_ core functions are called by client threads, properties must be thread-safe
    int app_id, num_cnodes;
    char id;
    std::string wa_trans_protocol, wa_gftp_lintf, wa_laddr, wa_gftp_lport, tmpfs_dir; //RIManager needs these for gftpb process
    IMsgCoder imsg_coder;
    
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCServer> bc_server_;
    boost::shared_ptr<SDMNode> sdm_node_;
    boost::shared_ptr<RFPManager> rfp_manager_;
    boost::shared_ptr<PBuffer> pbuffer_;
    patch_ds::thread_safe_map<int, boost::shared_ptr<BCClient> > appid_bcclient_map; //TODO: prettify
    
    boost::asio::io_service io_service;
    boost::asio::signal_set signals;
    
    RQTable rq_table;
    patch_ds::syncer<key_ver_pair> rq_syncer;
    
    patch_ds::thread_safe_map<key_ver_pair, laddr_lport__tmpfsdir_pair> key_ver___laddr_lport__tmpfsdir_map;
    patch_ds::syncer<key_ver_pair> rp_syncer;
    
    RSTable rs_table;
    patch_ds::syncer<key_ver_pair> handle_rp_syncer;
    
    patch_ds::syncer<key_ver_pair> bget_syncer;
    
    patch_ds::syncer<key_ver_pair> rf_wa_get_syncer;
    patch_ds::syncer<key_ver_pair> rp_wa_get_syncer;
    
    patch_ds::thread_safe_vector<key_ver_pair> key_ver_being_fetched_vector;
    patch_ds::syncer<key_ver_pair> being_fetched_syncer;
    
    TProfiler<key_ver_pair> rget_time_profiler;
    // 
    GFTPBTable gftpb_table;
    patch_ds::syncer<char> gftp_bping_syncer;
  public:
    RIManager(int app_id, int num_cnodes, 
              char id, std::string dht_lip, int dht_lport, std::string ipeer_dht_lip, int ipeer_dht_lport, 
              std::string wa_trans_protocol, std::string wa_laddr, std::string wa_gftp_lintf, std::string wa_gftp_lport, 
              std::string tmpfs_dir, std::list<std::string> wa_ib_lport_list,
              bool with_prefetch, size_t buffer_size, char* alphabet_, size_t alphabet_size, size_t context_size);
    ~RIManager();
    void close();
    std::string to_str();
    
    void handle_app_req(char* app_req);
    void handle_get(bool blocking, int app_id, std::map<std::string, std::string> get_map);
    int remote_get(char ds_id, std::map<std::string, std::string> r_get_map);
    void handle_put(int p_id, std::map<std::string, std::string> put_map);
    void handle_possible_remote_places(std::string key, unsigned int ver);
    void handle_prefetch(char ds_id, key_ver_pair kv);
    void handle_del(key_ver_pair kv);
    
    void handle_rimsg(std::map<std::string, std::string> rimsg_map);
    void handle_rquery(bool subscribe, std::map<std::string, std::string> r_query_map);
    void handle_rq_reply(std::map<std::string, std::string> rq_reply_map);
    void handle_rfetch(std::map<std::string, std::string> r_fetch_map);
    void handle_rqtable(std::map<std::string, std::string> r_rqtable_map);
    void handle_rplace(std::map<std::string, std::string> r_place_map);
    void handle_rp_reply(std::map<std::string, std::string> rp_reply_map);
#ifdef _GRIDFTP_
    void handle_gftpput_done(std::map<std::string, std::string> gftpput_done_map);
    void handle_gftp_bping(std::map<std::string, std::string> gftp_bping_map);
    void handle_gftp_bpong(std::map<std::string, std::string> gftpb_pong_map);
#endif // _GRIDFTP_
    void handle_rcmsg(std::map<std::string, std::string> rcmsg_map);

    int bcast_rq_table();
    int remote_query(bool subscribe, std::string key, unsigned int ver);
    int remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map);
    void handle_wa_get(std::string called_from, std::map<std::string, std::string> str_str_map);
    int remote_place(std::string key, unsigned int ver, char to_id);
    int remote_subscribe(std::string key, unsigned int ver);
    int gftp_bping(char to_id); //blocks until pong is received
};

#endif //end of _DSCLIENT_H_
