#ifndef DHTSERVER_H
#define DHTSERVER_H
//
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

class DHTServer{
  public:
    DHTServer(char* host, int port);
    ~DHTServer();
    void init_listen();
  private:
    char* host;
    int port;
    int stop_flag;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
};
//
#endif
