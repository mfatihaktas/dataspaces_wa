#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

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

#include <glog/logging.h>

using boost::asio::ip::tcp;

typedef boost::function<void(std::string, unsigned int, size_t, void*)> function_recv_callback;

template <class data_type>
class TCPServer {
  private:
    std::string key;
    unsigned int ver;
    char *lip;
    int lport;
    size_t recv_length;
    function_recv_callback _recv_callback;
    
    std::string server_name;
    bool stop_flag;
    
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    // std::vector<boost::shared_ptr<boost::thread> > handle_recv_thread_v;
    
  public:
    
    TCPServer(std::string key, unsigned int ver, char* lip, int lport, 
              size_t recv_length, function_recv_callback _recv_callback)
    : key(key),
      ver(ver),
      lip(lip),
      lport(lport),
      recv_length(recv_length),
      _recv_callback(_recv_callback),
      io_service_(new boost::asio::io_service),
      server_name("TCPServer<" + key + ", " + boost::lexical_cast<std::string>(ver) + ">"),
      stop_flag(false)
    {
      // To make sure socket is created after io_service
      boost::shared_ptr<boost::asio::ip::tcp::acceptor> t_acceptor_ (
        new boost::asio::ip::tcp::acceptor(*io_service_)
      );
      acceptor_ = t_acceptor_;
      
      boost::shared_ptr<boost::asio::ip::tcp::socket> t_socket_ (
        new boost::asio::ip::tcp::socket(*io_service_)
      );
      socket_ = t_socket_;
      
      boost::shared_ptr<boost::thread> t_ = boost::make_shared<boost::thread>(&TCPServer::init_listen, this);
      // 
      LOG(INFO) << "TCPServer:: constructed server= \n" << to_str();
    }
    
    ~TCPServer()
    {
      if (!stop_flag) {
        close();
      }
      // 
      LOG(INFO) << "TCPServer:: destructed server_name= " << server_name;
    }
    
    std::string to_str()
    {
      std::stringstream ss;
      
      ss << "\t server_name= " << server_name << "\n";
      ss << "\t lip= " << boost::lexical_cast<std::string>(lip) << "\n";
      ss << "\t lport= " << boost::lexical_cast<std::string>(lport) << "\n";
      
      return ss.str();
    }
    
    int close() {
      if (stop_flag != 0 ) {
        LOG(ERROR) << "close:: server:" << server_name << " is already closed!";
        return 2;
      }
      try {
        stop_flag = true;
         
        boost::system::error_code ec;
        socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        // socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
        socket_->close(ec);
        if (ec) {
          LOG(ERROR) << "close:: error code=" << ec;
          return 1;
        }
        io_service_->stop();
        /*TODO
         *causing hangs when closing server
         *commenting next waiting loop will cause exception in init_listen due to 
         *unfinished read over the socket
        for (int i = 0; i < handle_recv_thread_v.size(); i++) {
          handle_recv_thread_v[i]->join();
        }
        */
        // acceptor_->close();
        // 
        LOG(INFO) << "close:: closed server_name= " << server_name;
        return 0;
      }
      catch( std::exception& ex ) {
        LOG(ERROR) << "close:: Exception=" << ex.what();
        return 1;
      }
    }
    
    void init_listen()
    {
      try
      {
        boost::asio::ip::tcp::resolver resolver(*io_service_);
        boost::asio::ip::tcp::resolver::query query( 
          lip,
          boost::lexical_cast<std::string>(lport)
        );
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
        LOG(INFO) << "init_listen:: server= " << server_name << " listening on " << endpoint;
        // 
        acceptor_->open( endpoint.protocol() );
        acceptor_->set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
        acceptor_->bind(endpoint);
        acceptor_->listen(boost::asio::socket_base::max_connections);
        boost::system::error_code ec;
        acceptor_->accept(*socket_, ec);
        if (ec) {
          LOG(ERROR) << "init_listen:: error code= " << ec;
        }
        LOG(INFO) << "init_listen:: server= " << server_name << " got connection...";
        // acceptor_->close();
        init_recv();
      }
      catch( std::exception& ex )
      {
        LOG(ERROR) << "init_listen:: Exception=" << ex.what();
      }
    }
    
    void init_recv()
    {
      while (!stop_flag) {
        try {
          data_type* chunk_ = (data_type*)malloc(sizeof(data_type)*recv_length);
          size_t data_recved_length;
          
          boost::system::error_code ec;
          try {
            // boost::asio::read(*socket_, boost::asio::buffer(chunk_, recv_length) );
            data_recved_length = (socket_->read_some(boost::asio::buffer(chunk_, recv_length) ) )/sizeof(data_type);
          }
          catch( boost::system::system_error& err) {
            // LOG(ERROR) << "init_recv:: err= " << err.what();
            LOG(INFO) << "init_recv:: server:" << server_name << "; client closed the conn.";
            // close();
            return;
          }
          
          // handle_recv_thread_v.push_back(boost::make_shared<boost::thread>(&TCPServer::handle_recv, this, chunk_) );
          _recv_callback(key, ver, data_recved_length, chunk_);
        }
        catch( std::exception & ex ) {
          LOG(ERROR) << "init_recv:: Exception=" << ex.what();
        }
      }
    }
    
    // void handle_recv(data_type* chunk_)
    // {
    //   _recv_callback(chunk_);
    // }
};

#endif //_TCPSERVER_H_