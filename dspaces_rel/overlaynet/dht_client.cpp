#include "dht_client.h"

using boost::asio::ip::tcp;

DHTClient::DHTClient(char* client_name, char* host_ip, int port)
: client_name(client_name),
  host_ip(host_ip),
  port(port),
  io_service_(new boost::asio::io_service),
	stop_flag(false)
{
  boost::shared_ptr<boost::asio::ip::tcp::socket> t_socket_ (
    new boost::asio::ip::tcp::socket(*io_service_)
  );
  socket_ = t_socket_;
  // 
  LOG(INFO) << "client:" << client_name << " constructed.";
}

DHTClient::~DHTClient()
{
  if (!stop_flag) {
    close();
  }
  // 
  LOG(INFO) << "client:" << client_name << " destructed.";
}

int DHTClient::close()
{
  if (stop_flag) {
    LOG(ERROR) << "close:: client:" << client_name << " is already closed!";
    return 2;
  }
  try
  {
    stop_flag = true;
    // 
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    // socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
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

int DHTClient::send(size_t datasize, char* data)
{
  try {
    socket_->send(boost::asio::buffer(data, datasize));
    // LOG(INFO) << "send:: client:" << client_name << " sent datasize=" << datasize;
    return 0;
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "send:: Exception=" << ex.what();
    return 1;
  }
}

/*
int DHTClient::recv(char** bufp) {
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

