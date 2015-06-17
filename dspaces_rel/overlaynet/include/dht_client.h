#ifndef _DHTCLIENT_H_
#define _DHTCLIENT_H_
// 
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

class DHTClient {
  public:
    DHTClient(char* client_name, char* s_lip, int s_lport);
    ~DHTClient();
    std::string to_str();
    bool is_alive();
    int connect();
    int close();
    int send(int datasize, char* data);
  private:
    char *s_lip, *client_name;
    int s_lport;
    bool closed;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
};

#endif // _DHTCLIENT_H_
