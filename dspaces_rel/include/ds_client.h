#ifndef _DSCLIENT_H_
#define _DSCLIENT_H_

#include "ds_drive.h"

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
//Remote Query Table
struct RQTable
{
  public:
    RQTable();
    ~RQTable();
    int add_key_dsid_pair(std::string key, char ds_id);
    int update_entry(std::string key, char ds_id);
    int del_key(std::string key);
    bool query(std::string key, char& ds_id);
  private:
    std::map<std::string, char> key_dsid_map;
};

//Server side of blocking communication channel over dataspaces
//Single server <-> many clients
//used by RIManager
typedef boost::function<void(char*)> function_cb_on_recv;

struct BCServer
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
struct BCClient
{
  public:
    BCClient(int app_id, int num_others, int max_msg_size, 
             std::string base_comm_var_name);
    ~BCClient();
    int send(std::string type, std::string msg);
  private:
    int app_id, num_others, max_msg_size;
    std::string base_comm_var_name;
    std::string comm_var_name;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

//Remote Interaction Manager
#define RI_MSG_SIZE 100
//typedef boost::function<void(char*)> function_cb_on_ri_req;

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id);
    ~RIManager();
    void handle_ri_req(char* ri_req);
    void handle_r_get(std::string key);
  private:
    char id;
    int num_cnodes, app_id;
    boost::shared_ptr<BCServer> bc_server_;
};

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

#endif //end of _DSCLIENT_H_