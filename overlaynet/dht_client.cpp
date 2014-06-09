#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class DHTClient{
  public:
    DHTClient(char* host, int port);
    void make_channel();
  private:
    char* host;
    int port;
    boost::asio::io_service io_service_;
    tcp::socket socket_(io_service_);
}

void DHTClient::make_channel(){
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(host, port);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  
  boost::system::error_code error = boost::asio::error::host_not_found;
  tcp::resolver::iterator end;
  while(error && endpoint_iterator != end){
    socket_.close();
    socket_.connect(*endpoint_iterator++, error);
  }
  if (error)
    throw boost::system::system_error(error);
  //
}

int DHTClient::send(char* msg){
  boost::asio::write(socket_, boost::asio::buffer(msg), boost::asio::transfer_all(), ignored_error);
  
}

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

DHTClient::DHTClient(char* host, int port){
  this->host = host;
  this->port = port;
}