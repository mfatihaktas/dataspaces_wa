#include "dht_server.h"
#include <glog/logging.h>

#define HEADER_MSGSIZE_LENGTH 4

DHTServer::DHTServer(char* host, int port, char** overlay_buffer_, 
                     boost::function<void(void)> read_callback )
: io_service_(new boost::asio::io_service),
	acceptor_(new boost::asio::ip::tcp::acceptor( *io_service_ )),
	socket_(new boost::asio::ip::tcp::socket( *io_service_ )),
	stop_flag(0)
{
  this->host = host;
  this->port = port;
  this->overlay_buffer_ = overlay_buffer_;
  //
  google::InitGoogleLogging("DHTServer");
  //
  _read_callback = read_callback;
  //_read_callback();
  //
  LOG(INFO) << "constructed.";
}

DHTServer::~DHTServer()
{
  LOG(INFO) << "destructed.";
}

int DHTServer::close(){
  boost::system::error_code ec;
  socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  socket_->close(ec);
  if (ec){
    LOG(ERROR) << "ec=" << ec;
    return 1;
  }
  io_service_->stop();
  
  return 0;
}

/*
void DHTServer::handle_read_wrapper(const boost::system::error_code& error,
                                    std::size_t bytes_recved)
{
  if (error){
    LOG(ERROR) << "error=" << error;
    return;
  }
  LOG(INFO) << "bytes_recved=" << bytes_recved;
  _read_callback();
  //
  init_recv();
}
*/

void DHTServer::init_recv()
{
  while (!stop_flag){
    char msgsize_buffer[HEADER_MSGSIZE_LENGTH];
    boost::asio::read(*socket_, boost::asio::buffer( msgsize_buffer, HEADER_MSGSIZE_LENGTH ) );
    std::string str(msgsize_buffer);
    LOG(INFO) << "msgsize_buffer=" << msgsize_buffer;
    int msgsize = boost::lexical_cast<int>(str);
    LOG(INFO) << "msgsize=" << msgsize;
    //
    char* msg = new char[msgsize];
    boost::asio::read(*socket_, boost::asio::buffer( msg, msgsize ) );
    LOG(INFO) << "msg=" << msg;
    //
    *overlay_buffer_ = msg;
  	_read_callback();
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
	  LOG(INFO) << "listening on " << endpoint;
	  //
	  acceptor_->open( endpoint.protocol() );
	  acceptor_->set_option( boost::asio::ip::tcp::acceptor::reuse_address( false ) );
	  acceptor_->bind( endpoint );
	  acceptor_->listen( boost::asio::socket_base::max_connections );
	  boost::system::error_code ec;
	  acceptor_->accept( *socket_, ec );
    if (ec){
      LOG(ERROR) << "ec=" << ec;
    }
    LOG(INFO) << "got connection...";
    acceptor_->close();
    init_recv();
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "Exception=" << ex.what();
  }
}

