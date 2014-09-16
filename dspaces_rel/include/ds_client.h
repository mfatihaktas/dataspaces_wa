#ifndef _DSCLIENT_H_
#define _DSCLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <map>
#include <unistd.h>

#include <string>

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
//for boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>

#include "ds_drive.h"
#include "dht_node.h"
#include "packet.h"

#include "ib_delivery.h"

#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) do { int r=x; if (r){ printf("error: " #x " failed (returned non-zero=%d).", r); } } while (0)
#define TEST_Z(x)  do { if (!(x)) printf("error: " #x " failed (returned zero or null)."); } while (0)
#endif
/*******************************************************************************************/
class IMsgCoder
{
  public:
    IMsgCoder();
    ~IMsgCoder();
    std::map<std::string, std::string> decode(char* msg);
    std::string encode(std::map<std::string, std::string> msg_map);
    int decode_i_msg(std::map<std::string, std::string> msg_map, 
                     std::string& key, unsigned int& ver, int& size, 
                     int& ndim, uint64_t* &gdim_, uint64_t* &lb_, uint64_t* &ub_);
  
  
};

//Server side of blocking communication channel over dataspaces
//Single server <-> many clients
//used by RIManager
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

//Client side of blocking communication channel over dataspaces
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

//Remote Query Table
//A key 'k' can only be in a single dataspaces
typedef std::pair<std::string, unsigned int> key_ver_pair;
struct RQTable
{
  public:
    RQTable();
    ~RQTable();
    bool get_key_ver(std::string key, unsigned int ver, 
                     char& ds_id, int* size, int* ndim, 
                     uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int put_from_map(std::map<std::string, std::string> map);
    int put_key_ver(std::string key, unsigned int ver, 
                    char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int add_key_ver(std::string key, unsigned int ver, 
                    char ds_id, int size, int ndim, 
                    uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int update_key_ver(std::string key, unsigned int ver, 
                       char ds_id, int size, int ndim, 
                       uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int del_key_ver(std::string key, unsigned int ver);
    bool is_feasible_to_get(std::string key, unsigned int ver, 
                            int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    std::string to_str();
    std::map<std::string, std::string> to_str_str_map();
  private:
    std::map<key_ver_pair, char> key_ver__dsid_map;
    std::map<key_ver_pair, char>::iterator key_ver__dsid_map_it;
    std::map<key_ver_pair, std::map<std::string, std::vector<uint64_t> > > key_ver__datainfo_map;
    std::map<key_ver_pair, std::map<std::string, std::vector<uint64_t> > >::iterator key_ver__datainfo_map_it;
};

struct syncer{
  public:
    syncer();
    ~syncer();
    int add_sync_point(std::string key, int num_peers);
    int del_sync_point(std::string key);
    int wait(std::string key);
    int notify(std::string key);
  private:
    boost::mutex mutex;
  
    std::map<std::string, boost::shared_ptr<boost::condition_variable> > key_cv_map;
    std::map<std::string, boost::shared_ptr<boost::condition_variable> >::iterator key_cv_map_it;
    std::map<std::string, boost::shared_ptr<boost::mutex> > key_m_map;
    std::map<std::string, boost::shared_ptr<boost::mutex> >::iterator key_m_map_it;
    std::map<std::string, int> key_numpeers_map;
    std::map<std::string, int>::iterator key_numpeers_map_it;
};

class RFManager
{
  public:
    RFManager(std::list<std::string> wa_ib_lport_list, boost::shared_ptr<DSpacesDriver> ds_driver_);
    ~RFManager();
    std::string get_ib_lport();
    
    bool receive_put(std::string ib_laddr, std::string ib_lport,
                     std::string data_type, std::string key, unsigned int ver, int size,
                     int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    void handle_ib_receive(std::string key, size_t data_size, void* data_);
    
    bool get_send(std::string data_type, std::string key, unsigned int ver, int size,
                  int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_,
                  const char* ib_laddr, const char* ib_lport);
    size_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
  private:
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<DDManager> dd_manager_;
    
    std::map<std::string, size_t> key_recvedsize_map;
    std::map<std::string, void*> key_data_map;
};

//Remote Interaction Manager
//TODO: a better way for syncing client and server of bccomm
#define WAIT_TIME_FOR_BCCLIENT_DSLOCK 100*1000

const size_t RI_MAX_MSG_SIZE = 1000;
const size_t LI_MAX_MSG_SIZE = 1000;

const std::string REMOTE_QUERY = "rq";
const std::string REMOTE_QUERY_REPLY = "rq_reply";
const std::string REMOTE_FETCH = "rf";
const std::string REMOTE_GET = "rg";
const std::string REMOTE_GET_REPLY = "rg_reply";
const std::string REMOTE_PUT = "rp";
const std::string REMOTE_PUT_REPLY = "rp_reply";
const std::string LOCAL_GET = "lg";
const std::string LOCAL_GET_REPLY = "lg_reply";
const std::string LOCAL_PUT = "lp";
const std::string LOCAL_PUT_REPLY = "lp_reply";

const std::string REMOTE_RQTABLE = "rrqt";

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id, 
              char* lip, int lport, char* ipeer_lip, int ipeer_lport, 
              char* ib_laddr, std::list<std::string> wa_ib_lport_list);
    ~RIManager();
    std::string to_str();
    
    int bcast_rq_table();
    
    void handle_ri_req(char* ri_req);
    void handle_r_get(int app_id, std::map<std::string, std::string> r_get_map);
    
    void handle_li_req(char* li_req);
    void handle_l_put(std::map<std::string, std::string> l_put_map);
    
    void handle_wamsg(std::map<std::string, std::string> wamsg_map);
    void handle_r_query(std::map<std::string, std::string> r_query_map);
    void handle_rq_reply(std::map<std::string, std::string> rq_reply_map);
    void handle_r_fetch(std::map<std::string, std::string> r_fetch_map);
    void handle_r_rqtable(std::map<std::string, std::string> r_rqtable_map);
    
    bool remote_fetch(char ds_id, std::map<std::string, std::string> r_fetch_map);
    bool remote_query(std::string key, unsigned int ver);
    int broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map);
    int send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map);
  private:
    char id;
    int num_cnodes, app_id;
    RQTable rq_table;
    IMsgCoder imsg_coder;
    syncer rq_syncer;
    char* ib_laddr;
    
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCServer> li_bc_server_;
    boost::shared_ptr<BCServer> ri_bc_server_;
    boost::shared_ptr<DHTNode> dht_node_;
    boost::shared_ptr<RFManager> rf_manager_;
    
    std::map<int, boost::shared_ptr<BCClient> > appid_bcclient_map; //TODO: prettify
};

#endif //end of _DSCLIENT_H