#ifndef SERVER_H
#define SERVER_H
// 
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

typedef boost::function<void(char*)> function_recv_callback;

class SDMServer {
  private:
    std::string host_name, lip;
    int lport;
    // 
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::vector<boost::shared_ptr<boost::asio::ip::tcp::socket> > socket_v;
    bool closed;
    
    function_recv_callback _recv_callback;
  public:
    SDMServer(std::string host_name, std::string lip, int lport,
              function_recv_callback _recv_callback);
    ~SDMServer();
    std::string to_str();
    void set_recv_callback(function_recv_callback _recv_callback);
    
    int close();
    void init_listen();
    void init_recv(boost::shared_ptr<boost::asio::ip::tcp::socket>& socket_);
    void handle_recv(char* msg);
};

#endif //SERVER_H