#include "dht_server.h"
#include <glog/logging.h>

#define HEADER_MSGSIZE_LENGTH 4

DHTServer::DHTServer(char* host_name, char* host, int port, 
                     function_recv_callback recv_callback)
: io_service_(new boost::asio::io_service),
	acceptor_(new boost::asio::ip::tcp::acceptor( *io_service_ )),
	socket_(new boost::asio::ip::tcp::socket( *io_service_ )),
	stop_flag(0)
{
  this->host_name = host_name;
  this->host = host;
  this->port = port;
  //
  _recv_callback = recv_callback;
  //
  boost::shared_ptr< boost::thread > t_(
    new boost::thread(&DHTServer::init_listen, this)
  );
  //
  LOG(INFO) << "server:" << host_name << " constructed.";
}

DHTServer::~DHTServer()
{
  if (!stop_flag){
    close();
  }
  //
  LOG(INFO) << "server:" << host_name << " destructed.";
}

bool DHTServer::is_alive()
{
  return (stop_flag == 0);
}

void DHTServer::set_recv_callback(function_recv_callback recv_callback)
{
  _recv_callback = recv_callback;
}

int DHTServer::close(){
  if (!is_alive()){
    LOG(ERROR) << "close:: server:" << host_name << " is already closed!";
    return 2;
  }
  try
  {
    stop_flag = 1;
    //
    boost::system::error_code ec;
    //socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
    socket_->close(ec);
    if (ec){
      LOG(ERROR) << "close:: ec=" << ec;
      return 1;
    }
    io_service_->stop();
    /*TODO
     *causing hangs when closing server
     *commenting next waiting loop will cause exception in init_listen due to 
     *unfinished read over the socket
    for (int i = 0; i < handle_recv_thread_v.size(); i++){
      handle_recv_thread_v[i]->join();
    }
    */
    //
    LOG(INFO) << "close:: server:" << host_name << " closed.";
    return 0;
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "init_listen:: Exception=" << ex.what();
    return 1;
  }
}

void DHTServer::init_listen()
{
  try
  {
	  boost::asio::ip::tcp::resolver resolver( *io_service_ );
	  boost::asio::ip::tcp::resolver::query query( 
		  host,
		  boost::lexical_cast< std::string >( port )
	  );
	  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
	  LOG(INFO) << "init_listen:: server:" << host_name << " listening on " << endpoint;
	  //
	  acceptor_->open( endpoint.protocol() );
	  acceptor_->set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
	  acceptor_->bind( endpoint );
	  acceptor_->listen( boost::asio::socket_base::max_connections );
	  boost::system::error_code ec;
	  acceptor_->accept( *socket_, ec );
    if (ec){
      LOG(ERROR) << "init_listen:: ec=" << ec;
    }
    LOG(INFO) << "init_listen:: server:" << host_name << " got connection...";
    acceptor_->close();
    init_recv();
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "init_listen:: Exception=" << ex.what();
  }
}

void DHTServer::init_recv()
{
  while (!stop_flag){
    try
    {
      boost::system::error_code ec;
      char msgsize_buffer[HEADER_MSGSIZE_LENGTH+1];
      try{
        boost::asio::read(*socket_, boost::asio::buffer( msgsize_buffer, HEADER_MSGSIZE_LENGTH ) );
      }
      catch( boost::system::system_error& err)
      {
        //LOG(ERROR) << "init_recv:: err=" << err.what();
        LOG(INFO) << "init_recv:: server:" << host_name << "; client closed the conn.";
        close();
        return;
      }
      msgsize_buffer[HEADER_MSGSIZE_LENGTH] = '\0';
      std::string str(msgsize_buffer);
      //LOG(INFO) << "init_recv:: msgsize_buffer=" << msgsize_buffer;
      int msgsize = boost::lexical_cast<int>(str);
      //LOG(INFO) << "init_recv:: msgsize=" << msgsize;
      //
      char* msg = new char[msgsize];
      boost::asio::read(*socket_, boost::asio::buffer( msg, msgsize ) );
      //LOG(INFO) << "init_recv:: msg=" << msg;
      //
      boost::shared_ptr< boost::thread > t_(
        new boost::thread(&DHTServer::handle_recv, this, msg)
      );
      handle_recv_thread_v.push_back( t_ );
    }
    catch( std::exception & ex )
    {
      LOG(ERROR) << "init_recv:: Exception=" << ex.what();
    }
  }
}

void DHTServer::handle_recv(char* msg)
{
  _recv_callback(msg);
}