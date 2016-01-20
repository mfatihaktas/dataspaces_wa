#include "sdm_server.h"

#define HEADER_MSGSIZE_LENGTH 4

SDMServer::SDMServer(std::string host_name, std::string lip, int lport, 
                     function_recv_callback _recv_callback)
: host_name(host_name), lip(lip), lport(lport), _recv_callback(_recv_callback),
  io_service_(new boost::asio::io_service),
  closed(false)
{
  acceptor_ = boost::make_shared<boost::asio::ip::tcp::acceptor>(boost::ref(*io_service_) );

  boost::thread t(&SDMServer::init_listen, this);
  
  // io_service_->run();
  // 
  log_(INFO, "constructed; host_name= " << host_name)
}

SDMServer::~SDMServer()
{
  if (!closed)
    close();
  // 
  log_(INFO, "destructed; host_name= " << host_name)
}

std::string SDMServer::to_str()
{
  std::stringstream ss;
  ss << "\t host_name= " << host_name << "\n";
  ss << "\t lip= " << lip << "\n";
  ss << "\t lport= " << lport << "\n";
  
  return ss.str();
}

void SDMServer::set_recv_callback(function_recv_callback _recv_callback)
{
  this->_recv_callback = _recv_callback;
}

int SDMServer::close() {
  if (closed) {
    log_(ERROR, "already closed!; server= \n" << to_str() )
    return 2;
  }
  try {
    for (std::vector<boost::shared_ptr<boost::asio::ip::tcp::socket> >::iterator it = socket_v.begin(); it != socket_v.end(); it++) {
      boost::system::error_code ec;
      (*it)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      // (*it)->shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
      (*it)->close(ec);
      if (ec) {
        log_(ERROR, "ec=" << ec)
        return 1;
      }
    }
    io_service_->stop();
    // acceptor_->close();
    
    closed = true;
    log_(INFO, "closed server= \n" << to_str() )
    return 0;
  }
  catch(std::exception& ex) {
    log_(ERROR, "Exception=" << ex.what() )
    return 1;
  }
}

void SDMServer::init_listen()
{
  boost::asio::ip::tcp::resolver resolver(*io_service_);
  boost::asio::ip::tcp::resolver::query query(lip, boost::lexical_cast<std::string>(lport) );
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  log_(INFO, "server= " << host_name << " listening on " << endpoint)
  // 
  acceptor_->open(endpoint.protocol() );
  acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true) );
  acceptor_->bind(endpoint);
  acceptor_->listen(boost::asio::socket_base::max_connections);
  
  while (1) {
    try {
      boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ = boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
      boost::system::error_code ec;
      acceptor_->accept(*socket_, ec);
      if (ec)
        log_(ERROR, "ec= " << ec)
      socket_v.push_back(socket_);
      log_(INFO, "server= " << host_name << " got connection...")
      // acceptor_->close();
      boost::thread t(&SDMServer::init_recv, this, socket_v.back() );
      // boost::thread t(&SDMServer::init_recv, this, socket);
    }
    catch(std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
    }
  }
}

void SDMServer::init_recv(boost::shared_ptr<boost::asio::ip::tcp::socket>& socket_)
{
  while (!closed) {
    try {
      boost::system::error_code ec;
      char msgsize_buffer[HEADER_MSGSIZE_LENGTH + 1];
      try {
        boost::asio::read(*socket_, boost::asio::buffer(msgsize_buffer, HEADER_MSGSIZE_LENGTH) );
      }
      catch(boost::system::system_error& err) {
        // log_(ERROR, "err=" << err.what() )
        log_(INFO, "server:" << host_name << "; client closed the conn.")
        // close();
        return;
      }
      msgsize_buffer[HEADER_MSGSIZE_LENGTH] = '\0';
      std::string str(msgsize_buffer);
      // log_(INFO, "msgsize_buffer=" << msgsize_buffer)
      int msgsize = boost::lexical_cast<int>(str);
      // log_(INFO, "msgsize=" << msgsize)
      // 
      char* msg = (char*)malloc(msgsize*sizeof(char) );
      boost::asio::read(*socket_, boost::asio::buffer(msg, msgsize) );
      // log_(INFO, "msg=" << msg)
      boost::thread t(&SDMServer::handle_recv, this, msg);
    }
    catch(std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
    }
  }
}

void SDMServer::handle_recv(char* msg)
{
  _recv_callback(msg);
}
