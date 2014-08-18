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
//for boost tokenizer
#include <boost/tokenizer.hpp>

#include "ds_drive.h"
#include "dht_node.h"
#include "packet.h"

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
struct RQTable
{
  public:
    RQTable();
    ~RQTable();
    bool get_key(std::string key, char& ds_id, 
                 unsigned int* ver, int* size, int* ndim, 
                 uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int put_key(std::string key, char ds_id, 
                unsigned int ver, int size, int ndim, 
                uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int add_key(std::string key, char ds_id, 
                unsigned int ver, int size, int ndim, 
                uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int update_key(std::string key, char ds_id, 
                   unsigned int ver, int size, int ndim, 
                   uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int del_key(std::string key);
    std::string to_str();
  private:
    std::map<std::string, char> key_dsid_map;
    std::map<std::string, char>::iterator key_dsid_map_it;
    std::map<std::string, std::map<std::string, std::vector<uint64_t> > > key_datainfo_map;
    std::map<std::string, std::map<std::string, std::vector<uint64_t> > >::iterator key_datainfo_map_it;
};

//Remote Interaction Manager
//TODO: a better way for syncing client and server of bccomm
#define WAIT_TIME_FOR_BCCLIENT_DSLOCK 100*1000

#define RI_MAX_MSG_SIZE 1000
#define LI_MAX_MSG_SIZE 1000

struct syncer{
  public:
    syncer();
    ~syncer();
    int add_sync_point(std::string key, int num_peers);
    int del_sync_point(std::string key);
    int wait(std::string key);
    int notify(std::string key);
  private:
    std::map<std::string, boost::shared_ptr<boost::condition_variable> > key_cv_map;
    std::map<std::string, boost::shared_ptr<boost::condition_variable> >::iterator key_cv_map_it;
    std::map<std::string, boost::shared_ptr<boost::mutex> > key_m_map;
    std::map<std::string, boost::shared_ptr<boost::mutex> >::iterator key_m_map_it;
    std::map<std::string, int> key_numpeers_map;
    std::map<std::string, int>::iterator key_numpeers_map_it;
};

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id, 
              char* lip, int lport, char* ipeer_lip, int ipeer_lport);
    ~RIManager();
    std::string to_str();
    
    void handle_ri_req(char* ri_req);
    void handle_r_get(int app_id, std::map<std::string, std::string> r_get_map);
    
    void handle_li_req(char* li_req);
    void handle_l_put(std::map<std::string, std::string> l_put_map);
    
    void handle_wamsg(std::map<std::string, std::string> wamsg_map);
    void handle_r_query(std::map<std::string, std::string> r_query_map);
    void handle_rq_reply(std::map<std::string, std::string> rq_reply_map);
    
    bool remote_query(std::string key);
    int broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map);
    int send_msg(char ds_id, char msg_type, std::map<std::string, std::string> msg_map);
  private:
    char id;
    int num_cnodes, app_id;
    RQTable rq_table;
    IMsgCoder imsg_coder;
    syncer rq_syncer;
    
    boost::shared_ptr<DSpacesDriver> ds_driver_;
    boost::shared_ptr<BCServer> li_bc_server_;
    boost::shared_ptr<BCServer> ri_bc_server_;
    boost::shared_ptr<DHTNode> dht_node_;
    
    std::map<int, boost::shared_ptr<BCClient> > appid_bcclient_map;
};

#endif //end of _DSCLIENT_H_