#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

#include "patch_tcp.h"

#define HASH_PRIME 54059
#define HASH_PRIME_2 76963
#define HASH_PRIME_3 86969
unsigned int hash_str(std::string str);

#ifndef TRANS_PARS
#define TRANS_PARS
const int MAX_DATA_ID_LENGTH = 50;
const int MAX_DATA_SIZE_LENGTH = 50;
const int CHUNK_LENGTH = 10*1024*1024;
#endif // TRANS_PARS

typedef boost::function<void(std::string, int, void*)> data_recv_cb_func;

class TCPServer {
  private:
    std::string lip;
    int lport;
    data_recv_cb_func recv_cb;
    
    patch_tcp::syncer<unsigned int> syncer;
    std::string server_name;
    bool closed, inited;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    // boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    std::vector<boost::shared_ptr<boost::asio::ip::tcp::socket> > socket_v;
    // std::vector<boost::shared_ptr<boost::thread> > handle_recv_thread_v;
  public:
    TCPServer(std::string lip, int lport);
    ~TCPServer();
    std::string to_str();
    int close();
    
    std::string get_lip();
    int get_lport();
    
    int init(std::string data_id, data_recv_cb_func recv_cb);
    void init_listen();
    void init_recv(boost::shared_ptr<boost::asio::ip::tcp::socket>& socket_);
    void handle_recv(std::string data_id, int data_size, void* data_);
};

#endif //_TCP_SERVER_H_