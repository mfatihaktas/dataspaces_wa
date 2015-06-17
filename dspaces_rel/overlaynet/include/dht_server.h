#ifndef DHTSERVER_H
#define DHTSERVER_H
// 
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

typedef boost::function<void(char*)> function_recv_callback;

class DHTServer {
  public:
    DHTServer(char* host_name, char* lip, int lport,
              function_recv_callback _recv_callback = NULL);
    ~DHTServer();
    std::string to_str();
    void set_recv_callback(function_recv_callback _recv_callback);
    
    int close();
    void init_listen();
    void init_recv();
    void handle_recv(char* msg);
  private:
    char *lip, *host_name;
    int lport;
    bool closed;
    // 
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
    // 
    function_recv_callback _recv_callback;
    std::vector< boost::shared_ptr< boost::thread > > handle_recv_thread_v;
};
// 
#endif //DHTSERVER_H
