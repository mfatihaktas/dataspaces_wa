#ifndef DHTCLIENT_H
#define DHTCLIENT_H
//
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

class DHTClient{
  public:
    DHTClient(char* host, int port);
    ~DHTClient();
    int connect();
    int send(size_t datasize, char* msg);
  private:
    char* host;
    int port;
    int stop_flag;
    //
    boost::shared_ptr< boost::asio::io_service > io_service_;
    boost::shared_ptr< boost::asio::ip::tcp::socket > socket_;
};
//
#endif
