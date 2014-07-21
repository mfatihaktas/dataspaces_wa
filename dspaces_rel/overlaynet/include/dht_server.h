#ifndef DHTSERVER_H
#define DHTSERVER_H
//
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

typedef boost::function<void(char*)> function_recv_callback;

class DHTServer{
  public:
    DHTServer(char* host_name, char* host, int port, 
              function_recv_callback recv_callback = NULL);
    ~DHTServer();
    void set_recv_callback(function_recv_callback recv_callback);
    
    int close();
    void init_listen();
    void init_recv();
    void handle_recv(char* msg);
    bool is_alive();
  private:
    char *host, *host_name;
    int port;
    int stop_flag;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
    //
    function_recv_callback _recv_callback;
    std::vector< boost::shared_ptr< boost::thread > > handle_recv_thread_v;
};
//
#endif
