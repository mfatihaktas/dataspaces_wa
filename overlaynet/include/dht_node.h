#ifndef DHTNODE_H
#define DHTNODE_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
//for wait_flag
#include <csignal>
//#include <mutex>
//#include <condition_variable>

#include "dht_server.h"

typedef boost::function<void(char*)> handle_recv_function;

class DHTNode{
  public:
    static boost::condition_variable cv;
    //
    DHTNode(char* lip, int lport);
    ~DHTNode();
    
    void handle_recv(char* type__srlzedmsgmap);
    void wait_for_flag();
  private:
    boost::shared_ptr< DHTServer > server_;
    handle_recv_function fp_handle_recv;
    //
    boost::mutex m;
};

#endif //DHTNODE_H
