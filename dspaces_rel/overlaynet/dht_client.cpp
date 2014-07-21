#include "dht_client.h"
#include <glog/logging.h>

using boost::asio::ip::tcp;

DHTClient::DHTClient(char* client_name, char* host_ip, int port)
: io_service_(new boost::asio::io_service),
	socket_(new boost::asio::ip::tcp::socket( *io_service_ )),
	stop_flag(0)
{
  this->client_name = client_name;
  this->host_ip = host_ip;
  this->port = port;
  //
  LOG(INFO) << "client:" << client_name << " constructed.";
}

DHTClient::~DHTClient()
{
  if (!stop_flag){
    close();
  }
  //
  LOG(INFO) << "client:" << client_name << " destructed.";
}

bool DHTClient::is_alive()
{
  return (stop_flag == 0);
}

int DHTClient::close()
{
  if (!is_alive()){
    LOG(ERROR) << "close:: client:" << client_name << " is already closed!";
    return 2;
  }
  try
  {
    stop_flag = 1;
    //
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    //socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    socket_->close(ec);
    if (ec){
      LOG(ERROR) << "close:: ec=" << ec;
    }
    io_service_->stop();
    //
    LOG(INFO) << "close:: client:" << client_name << " closed.";
    return 0;
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "close:: Exception=" << ex.what();
    return 1;
  }
}

int DHTClient::connect()
{
  try
  {
    tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query( 
	    host_ip,
	    boost::lexical_cast< std::string >( port )
    );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );
    LOG(INFO) << "connect:: client:" << client_name << " connecting to " << endpoint << "...";
    socket_->connect(endpoint);
    LOG(INFO) << "connect:: client:" << client_name << " connected.";
    return 0;
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "connect:: Exception=" << ex.what();
    return 1;
  }
}

int DHTClient::send(size_t datasize, char* data){
  try
  {
    socket_->send(boost::asio::buffer(data, datasize));
    LOG(INFO) << "send:: client:" << client_name << " sent datasize=" << datasize;
    return 0;
  }
  catch( std::exception & ex )
  {
    LOG(ERROR) << "send:: Exception=" << ex.what();
    return 1;
  }
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

