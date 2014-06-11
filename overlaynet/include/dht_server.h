#ifndef DHTSERVER_H
#define DHTSERVER_H
//
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/function.hpp"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

typedef boost::function<void(char, char*)> read_callback_function;

class DHTServer{
  public:
    DHTServer(char* host, int port, 
              read_callback_function read_callback);
    ~DHTServer();
    int close();
    void init_listen();
    void init_recv();
    void handle_recv(char* msg);
  private:
    char* host;
    int port;
    int stop_flag;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
    //
    read_callback_function _read_callback;
    std::vector< boost::shared_ptr< boost::thread > > handle_recv_thread_v;
};
//
#endif
