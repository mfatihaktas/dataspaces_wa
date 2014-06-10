#ifndef DHTSERVER_H
#define DHTSERVER_H
//
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/function.hpp"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

class DHTServer{
  public:
    DHTServer(char* host, int port, char** overlay_buffer_,  
              boost::function<void(void)> read_callback);
    ~DHTServer();
    int close();
    void init_listen();
    void init_recv();
    //void handle_read_wrapper(const boost::system::error_code& error, std::size_t bytes_recved);
  private:
    char* host;
    int port;
    int stop_flag;
    //
    char** overlay_buffer_;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
    //
    boost::function<void(void)> _read_callback;
};
//
#endif
