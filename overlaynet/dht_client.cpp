#include "dht_client.h"
#include <glog/logging.h>

using boost::asio::ip::tcp;

DHTClient::DHTClient(char* host, int port)
: io_service_(new boost::asio::io_service),
	socket_(new boost::asio::ip::tcp::socket( *io_service_ ))
{
  this->host = host;
  this->port = port;
  //
  google::InitGoogleLogging("DHTClient");
  //
  LOG(INFO) << "constructed.";
}

DHTClient::~DHTClient()
{
  boost::system::error_code ec;
  socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  socket_->close(ec);
  if (ec){
    LOG(ERROR) << "ec=" << ec;
  }
  io_service_->stop();
  //
  LOG(INFO) << "destructed.";
}

int DHTClient::connect()
{
  try
  {
    tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query( 
	    host,
	    boost::lexical_cast< std::string >( port )
    );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
    LOG(INFO) << "connecting to " << endpoint << " ...";
    socket_->connect(endpoint);
    LOG(INFO) << "connected.";
    return 0;
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "Exception=" << ex.what();
    return 1;
  }
}

int DHTClient::send(size_t datasize, char* data){
  socket_->send(boost::asio::buffer(data, datasize));
  LOG(INFO) << "sent datasize=" << datasize;
  LOG(INFO) << "sent data=" << data;
  return 1;
}

/*
int DHTClient::recv(char** bufp){
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

