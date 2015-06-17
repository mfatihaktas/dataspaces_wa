#include "sdm_client.h"

SDMClient::SDMClient(std::string client_name, std::string s_lip, int s_lport)
: client_name(client_name), s_lip(s_lip), s_lport(s_lport),
  io_service_(new boost::asio::io_service),
  closed(false)
{
  socket_ = boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
  // 
  LOG(INFO) << "client= " << client_name << " constructed.";
}

SDMClient::~SDMClient()
{
  if (!closed)
    close();
  // 
  LOG(INFO) << "client= " << client_name << " destructed.";
}

std::string SDMClient::to_str()
{
  std::stringstream ss;
  ss << "\t client_name= " << boost::lexical_cast<std::string>(client_name) << "\n";
  ss << "\t s_lip= " << boost::lexical_cast<std::string>(s_lip) << "\n";
  ss << "\t s_lport= " << boost::lexical_cast<std::string>(s_lport) << "\n";
  
  return ss.str();
}

int SDMClient::close()
{
  if (closed) {
    LOG(ERROR) << "close:: client= " << client_name << " is already closed!";
    return 2;
  }
  try {
    closed = true;
    // 
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    // socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    socket_->close(ec);
    if (ec)
      LOG(ERROR) << "close:: ec= " << ec;

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

int SDMClient::connect()
{
  try {
    boost::asio::ip::tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query(s_lip, boost::lexical_cast<std::string>(s_lport) );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    LOG(INFO) << "connect:: client= " << client_name << " connecting to " << endpoint << "...";
    socket_->connect(endpoint);
    LOG(INFO) << "connect:: client= " << client_name << " connected.";
    return 0;
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "connect:: Exception= " << ex.what();
    return 1;
  }
}

int SDMClient::send(int datasize, char* data)
{
  try {
    socket_->send(boost::asio::buffer(data, datasize) );
    // LOG(INFO) << "send:: client:" << client_name << " sent datasize=" << datasize;
    return 0;
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "send:: Exception= " << ex.what();
    return 1;
  }
}
