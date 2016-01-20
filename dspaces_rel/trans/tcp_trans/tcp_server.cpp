#include "tcp_server.h"

TCPServer::TCPServer(std::string lip, int lport)
: lip(lip), lport(lport),
  io_service_(boost::make_shared<boost::asio::io_service>() ),
  server_name(lip + "_" + boost::lexical_cast<std::string>(lport) ),
  closed(false), inited(false)
{
  // To make sure socket is created after io_service
  acceptor_ = boost::make_shared<boost::asio::ip::tcp::acceptor>(boost::ref(*io_service_) );
  // socket_ =  boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
  // 
  log_(INFO, "server= " << server_name << " constructed; \n" << to_str() << "\n")
}

TCPServer::~TCPServer()
{
  if (!closed)
    close();
  // 
  log_(INFO, "server= " << server_name << " destructed.")
}

std::string TCPServer::to_str()
{
  std::stringstream ss;
  ss << "\t lip= " << lip << "\n"
     << "\t lport= " << lport << "\n";
  
  return ss.str();
}

int TCPServer::close() {
  if (closed) {
    log_(ERROR, "server= " << server_name << " already closed; " << to_str() )
    return 1;
  }
  try {
    boost::system::error_code ec;
    bool err_flag = false;
    for (std::vector<boost::shared_ptr<boost::asio::ip::tcp::socket> >::iterator it = socket_v.begin(); it != socket_v.end(); it++) {
      (*it)->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      // (*it)->shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
      (*it)->close(ec);
      if (ec)
        err_flag = true;
    }
    io_service_->stop();
    
    if (err_flag) {
      log_(ERROR, "error code=" << ec)
      return 1;
    }
    // TODO
    // causing hangs when closing server
    // commenting next waiting loop will cause exception in init_listen due to unfinished read over the socket
    // for (int i = 0; i < handle_recv_thread_v.size(); i++)
    //   handle_recv_thread_v[i]->join();
    
    // acceptor_->close();
    closed = true;
    log_(INFO, "closed.")
    return 0;
  }
  catch (std::exception& ex) {
    log_(ERROR, "Exception=" << ex.what() )
    return 1;
  }
}

std::string TCPServer::get_lip() { return lip; }
int TCPServer::get_lport() { return lport; }

int TCPServer::init(std::string data_id, tcp_data_recv_cb_func data_recv_cb)
{
  this->data_recv_cb = data_recv_cb;
  if (!inited) {
    boost::thread t(&TCPServer::init_listen, this);
    inited = true;
  }
  unsigned int sync_point = patch_tcp::hash_str(data_id);
  syncer.add_sync_point(sync_point, 1);
  syncer.wait(sync_point);
  syncer.del_sync_point(sync_point);
}

void TCPServer::init_listen()
{
  boost::asio::ip::tcp::resolver resolver(*io_service_);
  boost::asio::ip::tcp::resolver::query query(lip.c_str(), boost::lexical_cast<std::string>(lport) );
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  // 
  acceptor_->open(endpoint.protocol() );
  acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true) );
  acceptor_->bind(endpoint);
  while (!closed) {
    try {
      log_(INFO, "server= " << server_name << " listening on " << endpoint)
      acceptor_->listen(boost::asio::socket_base::max_connections);
      boost::system::error_code ec;
      
      boost::shared_ptr<boost::asio::ip::tcp::socket> socket_= boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
      // socket_v.push_back(socket_);
      acceptor_->accept(*socket_, ec);
      if (ec) {
        log_(ERROR, "error code= " << ec)
        return;
      }
      log_(INFO, "server= " << server_name << " got connection from " << socket_->remote_endpoint().address().to_string() )
      // acceptor_->close();
      // init_recv(socket_);
      boost::thread t(&TCPServer::init_recv, this, socket_);
    }
    catch(std::exception& ex) {
      log_(ERROR, "Exception=" << ex.what() )
    }
  }
}

void TCPServer::init_recv(boost::shared_ptr<boost::asio::ip::tcp::socket>& socket_)
{
  while (!closed) {
    char* data_id_ = (char*)malloc(TCP_MAX_DATA_ID_LENGTH);
    boost::asio::read(*socket_, boost::asio::buffer(data_id_, TCP_MAX_DATA_ID_LENGTH) );
    std::string data_id = boost::lexical_cast<std::string>(data_id_);
    
    char* data_size_ = (char*)malloc(TCP_MAX_DATA_SIZE_LENGTH);
    boost::asio::read(*socket_, boost::asio::buffer(data_size_, TCP_MAX_DATA_ID_LENGTH) );
    log_(INFO, "server= " << server_name << ", data_id_= " << data_id_ << ", data_size_= " << data_size_)
    int data_size = boost::lexical_cast<int>(data_size_);
    
    free(data_id_);
    free(data_size_);
    
    int total_to_recv_size = data_size;
    while (!closed) {
      int recved_size;
      void* chunk_;
      try {
        int to_recv_size = (total_to_recv_size < TCP_CHUNK_SIZE) ? total_to_recv_size : TCP_CHUNK_SIZE;
        // int to_recv_size = total_to_recv_size;
        chunk_ = malloc(to_recv_size);
        
        // recved_size = socket_->read_some(boost::asio::buffer(chunk_, to_recv_size) );
        boost::asio::read(*socket_, boost::asio::buffer(chunk_, to_recv_size) );
        recved_size = to_recv_size;
        
        total_to_recv_size -= recved_size;
      }
      catch(boost::system::system_error& err) {
        // log_(ERROR, "err= " << err.what() )
        log_(INFO, "server= " << server_name << ", data_id= " << data_id << "; client closed the conn.err= " << err.what() )
        return;
      }
      
      // handle_recv_thread_v.push_back(boost::make_shared<boost::thread>(&TCPServer::handle_recv, this, chunk_) );
      // data_recv_cb(data_id, recved_size, chunk_);
      boost::thread t(&TCPServer::handle_recv, this, data_id, recved_size, chunk_);
      if (total_to_recv_size == 0) {
        log_(INFO, "server= " << server_name << " completed receiving data_id= " << data_id)
        
        boost::system::error_code ec;
        socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
        socket_->close(ec);
        if (ec)
          log_(ERROR, "socket_->shutdown/close failed with ec= " << ec)
        
        syncer.notify(patch_tcp::hash_str(data_id) );
        // break;
        return;
      }
    }
  }
}

void TCPServer::handle_recv(std::string data_id, int data_size, void* data_)
{
  data_recv_cb(data_id, data_size, data_);
}
