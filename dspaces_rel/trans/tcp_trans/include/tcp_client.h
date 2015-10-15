#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

#ifndef TRANS_PARS
#define TRANS_PARS
const int MAX_DATA_ID_LENGTH = 50;
const int MAX_DATA_SIZE_LENGTH = 50;
const int CHUNK_LENGTH = 10*1024*1024;
#endif // TRANS_PARS

class TCPClient {
  private:
    std::string s_lip;
    int s_lport;
    
    std::string client_name;
    bool closed;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  public:
    TCPClient(std::string s_lip, int s_lport);
    ~TCPClient();
     std::string to_str();
    int close();
    int connect();
    int send_control_data(std::string control_data, int max_size);
    int send_chunk(int chunk_size, void* chunk_);
    int send(std::string data_id, int data_size, void* data_);
};

#endif // _TCP_CLIENT_H_
