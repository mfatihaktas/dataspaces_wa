#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

using boost::asio::ip::tcp;

#define CHUNK_LENGTH 10*1024*1024

template <class data_type>
class TCPClient {
  private:
    char *client_name, *s_lip;
    int s_lport;
    size_t data_length;
    data_type* data_;
    bool stop_flag;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    
  public:
    TCPClient(char* client_name, char* s_lip, int s_lport,
              size_t data_length, data_type* data_)
    : client_name(client_name),
      s_lip(s_lip),
      s_lport(s_lport),
      data_length(data_length),
      data_(data_),
      io_service_(new boost::asio::io_service),
      stop_flag(false)
    {
      boost::shared_ptr<boost::asio::ip::tcp::socket> t_socket_ (
        new boost::asio::ip::tcp::socket(*io_service_)
      );
      socket_ = t_socket_;
      // 
      LOG(INFO) << "TCPClient:: constructed client= \n" << to_str();
    }
    
    ~TCPClient()
    {
      if (!stop_flag) {
        close();
      }
      // 
      LOG(INFO) << "TCPClient:: destructed." << to_str();
    }
    
    std::string to_str()
    {
      std::stringstream ss;
    
      ss << "\t s_lip= " << s_lip << "\n";
      ss << "\t s_lport= " << s_lport << "\n";
      ss << "\t data_length= " << boost::lexical_cast<std::string>(data_length) << "\n";
      
      return ss.str();
    }
    
    int close()
    {
      if (stop_flag) {
        LOG(ERROR) << "close:: client= " << client_name << " is already closed!";
        return 2;
      }
      try {
        stop_flag = true;
        // 
        boost::system::error_code ec;
        socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        // socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        socket_->close(ec);
        if (ec) {
          LOG(ERROR) << "close:: ec= " << ec;
        }
        io_service_->stop();
        // 
        LOG(INFO) << "close:: client= " << client_name << " closed.";
        return 0;
      }
      catch(std::exception& ex) {
        LOG(ERROR) << "close:: Exception= " << ex.what();
        return 1;
      }
    }
    
    int connect()
    {
      try {
        tcp::resolver resolver(*io_service_);
        boost::asio::ip::tcp::resolver::query query( 
          s_lip,
          boost::lexical_cast<std::string>( s_lport )
        );
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
        LOG(INFO) << "connect:: client= " << client_name << " connecting to " << endpoint << "...";
        socket_->connect(endpoint);
        LOG(INFO) << "connect:: client= " << client_name << " connected.";
        return 0;
      }
      catch(std::exception& ex ) {
        LOG(ERROR) << "connect:: Exception=" << ex.what();
        return 1;
      }
    }
    
    size_t send_chunk(size_t chunk_length, data_type* chunk_)
    {
      try {
        return ( (size_t)socket_->send(boost::asio::buffer(chunk_, chunk_length) ) ) / sizeof(data_type);
        // LOG(INFO) << "send:: client:" << client_name << " sent data_length=" << data_length;
      }
      catch(std::exception& ex) {
        LOG(ERROR) << "send:: Exception=" << ex.what();
        return 0;
      }
    }
    
    int init()
    {
      if (connect() ) {
        LOG(ERROR) << "init:: connect failed!";
        return 1;
      }
      
      size_t data_length_to_send = data_length;
      while(data_length_to_send > 0) {
        size_t chunk_length = (data_length_to_send > CHUNK_LENGTH) ? CHUNK_LENGTH : data_length_to_send;
        size_t data_length_sent = send_chunk(chunk_length, data_);
        if (!data_length_sent) {
          LOG(ERROR) << "init:: send_chunk failed!";
          return 1;
        }
        
        data_length_to_send -= data_length_sent;
        data_ += data_length_sent;
      }
      
      return 0;
    }
    
    /*
    int recv(char** bufp) {
      boost::system::error_code error;
      try{
        size_t len = socket_.read_some(boost::asio::buffer(*bufp), error);
        if (error == boost::asio::error::eof)
          std::cout << "conn is closed by peer\n";
        else if (error)
          throw boost::system::system_error(error);
      }
      catch (std::exception& e){
        std::cerr << e.what() << std::endl;
      }
    }
    */
};

#endif // _TCP_CLIENT_H_
