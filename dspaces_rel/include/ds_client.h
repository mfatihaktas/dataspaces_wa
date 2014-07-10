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

//Remote Interaction Manager
#define RI_MSG_SIZE 100

typedef boost::function<void(char*)> function_cb_on_ri_req;

class RIManager
{
  public:
    RIManager(char id, int num_cnodes, int app_id);
    ~RIManager();
    void handle_ri_req(char* ri_req);
    void init_listen_ri_req(int peer_id);
  private:
    char id;
    int num_cnodes, app_id;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
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
    void put_ri_msg(std::string ri_msg);
    
    void block();
    void unblock();
    void wait();
  private:
    int num_cnodes, app_id;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

#endif //end of _DSCLIENT_H_