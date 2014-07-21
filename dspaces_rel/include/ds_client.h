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

class RIMsgCoder
{
  public:
    RIMsgCoder();
    ~RIMsgCoder();
    std::map<std::string, std::string> decode(char* msg);
    std::string encode(std::map<std::string, std::string> msg_map);
    int decode_r_get(std::map<std::string, std::string> msg_map,
                     std::string& var_name, unsigned int& ver, int& size,
                     int& ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
  
  
};

//Server side of blocking communication channel over dataspaces
//Single server <-> many clients
//used by RIManager
typedef boost::function<void(char*)> function_cb_on_recv;

class BCServer
{
  public:
    BCServer(int app_id, int num_clients, int msg_size, 
             std::string base_comm_var_name, function_cb_on_recv f_cb);
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
             std::string base_comm_var_name);
    ~BCClient();
    int send(std::string type, std::map<std::string, std::string> msg_map);
  private:
    int app_id, num_others, max_msg_size;
    std::string base_comm_var_name;
    std::string comm_var_name;
    RIMsgCoder rimsg_coder;
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
                 unsigned int& ver, int& size, int& ndim, 
                 uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int add_key(std::string key, char ds_id, 
                unsigned int ver, int size, int ndim, 
                uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_);
    int update_key(std::string key, char ds_id);
    int del_key(std::string key);
    std::string to_str();
  private:
    std::map<std::string, char> key_dsid_map;
    std::map<std::string, char>::iterator key_dsid_map_it;
    std::map<std::string, std::map<std::string, std::vector<uint64_t> > > key_datainfo_map;
    std::map<std::string, std::map<std::string, std::vector<uint64_t> > >::iterator key_datainfo_map_it;
};

//Remote Interaction Manager
#define RI_MSG_SIZE 100
//typedef boost::function<void(char*)> function_cb_on_ri_req;

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id, 
              char* lip, int lport, char* ipeer_lip, int ipeer_lport);
    ~RIManager();
    std::string to_str();
    void handle_ri_req(char* ri_req);
    void handle_r_get(std::map<std::string, std::string> r_get_map);
  private:
    char id;
    int num_cnodes, app_id;
    RIMsgCoder rimsg_coder;
    boost::shared_ptr<BCServer> bc_server_;
    boost::shared_ptr<DHTNode> dht_node_;
};

/*
class TestClient
{
  public:
    TestClient(int num_cnodes, int app_id);
    ~TestClient();
    float get_avg(int size, int* data);
    void put_test();
    void get_test();
    
    void lock_ri_var(int peer_id);
    void put_ri_msg(std::string type, std::string msg);
    
    void block();
    void unblock();
    void wait();
  private:
    int num_cnodes, app_id;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};
*/

#endif //end of _DSCLIENT_H_