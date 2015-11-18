#ifndef _SDM_CLIENT_H_
#define _SDM_CLIENT_H_
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

class SDMClient {
  private:
    std::string s_lip, client_name;
    int s_lport;
    bool closed;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  public:
    SDMClient(std::string client_name, std::string s_lip, int s_lport);
    ~SDMClient();
    std::string to_str();
    
    int connect();
    int close();
    int send(int datasize, char* data);
};

#endif // _SDM_CLIENT_H_
