#include "dht_server.h"

#include <glog/logging.h>

DHTServer::DHTServer(char* host, int port)
: io_service_(new boost::asio::io_service),
	acceptor_(new boost::asio::ip::tcp::acceptor( *io_service_ )),
	socket_(new boost::asio::ip::tcp::socket( *io_service_ )),
	stop_flag(0)
{
  this->host = host;
  this->port = port;
  //
  google::InitGoogleLogging("DHTServer");
  
  LOG(INFO) << "info\n";
  LOG(ERROR) << "error\n";
}

DHTServer::~DHTServer(){
  boost::system::error_code ec;
  socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  socket_->close(ec);
  if (ec){
    std::cout << "DHTServer::~DHTServer: ec=" << ec << std::endl;
  }
  io_service_->stop();
}

void DHTServer::init_listen(){
  try
  {
	  boost::asio::ip::tcp::resolver resolver( *io_service_ );
	  boost::asio::ip::tcp::resolver::query query( 
		  host,
		  boost::lexical_cast< std::string >( port )
	  );
	  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
	  std::cout << "DHTServer::init_listen: listening on " << endpoint << std::endl;
	  //
	  acceptor_->open( endpoint.protocol() );
	  acceptor_->set_option( boost::asio::ip::tcp::acceptor::reuse_address( false ) );
	  acceptor_->bind( endpoint );
	  acceptor_->listen( boost::asio::socket_base::max_connections );
	  boost::system::error_code accept_ec;
	  acceptor_->accept( *socket_, accept_ec );
    if (accept_ec){
      std::cout << "DHTServer::init_listen: accept_ec=" << accept_ec << std::endl;
    }
    
    acceptor_->close();
  }
  catch( std::exception & ex )
  {
	  std::cout << "DHTServer::init_listen: Exception=" << ex.what() << std::endl;
  }
}

/*
void DHTServer::init_comm(){
  
}

*/
