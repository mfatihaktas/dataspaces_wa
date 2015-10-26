#include "tcp_server.h"

unsigned int hash_str(std::string str)
{
  unsigned int h = 31; // Also prime
  const char* s_ = str.c_str();
  while (*s_) {
    h = (h * HASH_PRIME) ^ (s_[0] * HASH_PRIME_2);
    s_++;
  }
  return h; // return h % HASH_PRIME_3;
}

TCPServer::TCPServer(std::string lip, int lport)
: lip(lip), lport(lport), recv_cb(recv_cb),
  io_service_(boost::make_shared<boost::asio::io_service>() ),
  server_name(lip + "_" + boost::lexical_cast<std::string>(lport) ),
  closed(false), inited(false)
{
  // To make sure socket is created after io_service
  acceptor_ = boost::make_shared<boost::asio::ip::tcp::acceptor>(boost::ref(*io_service_) );
  // socket_ =  boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
  // 
  LOG(INFO) << "TCPServer:: server= " << server_name << " constructed; \n" << to_str() << "\n";
}

TCPServer::~TCPServer()
{
  if (!closed)
    close();
  // 
  LOG(INFO) << "TCPServer:: server= " << server_name << " destructed.";
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
    LOG(ERROR) << "close:: server= " << server_name << " already closed; " << to_str();
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
      LOG(ERROR) << "close:: error code=" << ec;
      return 1;
    }
    // TODO
    // causing hangs when closing server
    // commenting next waiting loop will cause exception in init_listen due to unfinished read over the socket
    // for (int i = 0; i < handle_recv_thread_v.size(); i++)
    //   handle_recv_thread_v[i]->join();
    
    // acceptor_->close();
    closed = true;
    LOG(INFO) << "close:: closed.";
    return 0;
  }
  catch (std::exception& ex) {
    LOG(ERROR) << "close:: Exception=" << ex.what();
    return 1;
  }
}

std::string TCPServer::get_lip() { return lip; }
int TCPServer::get_lport() { return lport; }

int TCPServer::init(std::string data_id, data_recv_cb_func recv_cb)
{
  this->recv_cb = recv_cb;
  if (!inited) {
    boost::thread t(&TCPServer::init_listen, this);
    inited = true;
  }
  unsigned int sync_point = hash_str(data_id);
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
      LOG(INFO) << "init_listen:: server= " << server_name << " listening on " << endpoint;
      acceptor_->listen(boost::asio::socket_base::max_connections);
      boost::system::error_code ec;
      
      socket_v.push_back(boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) ) );
      acceptor_->accept(*(socket_v.back() ), ec);
      if (ec) {
        LOG(ERROR) << "init_listen:: error code= " << ec;
        return;
      }
      LOG(INFO) << "init_listen:: server= " << server_name << " got connection from " << socket_v.back()->remote_endpoint().address().to_string();
      // acceptor_->close();
      // init_recv(socket_v.back() );
      boost::thread t(&TCPServer::init_recv, this, socket_v.back() );
    }
    catch(std::exception& ex) {
      LOG(ERROR) << "init_listen:: Exception=" << ex.what();
    }
  }
}

void TCPServer::init_recv(boost::shared_ptr<boost::asio::ip::tcp::socket> socket_)
{
  while (!closed) {
    char* data_id_ = (char*)malloc(MAX_DATA_ID_LENGTH);
    boost::asio::read(*socket_, boost::asio::buffer(data_id_, MAX_DATA_ID_LENGTH) );
    std::string data_id = boost::lexical_cast<std::string>(data_id_);
    free(data_id_);
    
    char* data_size_ = (char*)malloc(MAX_DATA_SIZE_LENGTH);
    boost::asio::read(*socket_, boost::asio::buffer(data_size_, MAX_DATA_ID_LENGTH) );
    int data_size = boost::lexical_cast<int>(data_size_);
    free(data_size_);
    
    int total_to_recv_size = data_size;
    while (!closed) {
      int recved_size;
      void* chunk_;
      try {
        int to_recv_size = (total_to_recv_size < CHUNK_LENGTH) ? total_to_recv_size : CHUNK_LENGTH;
        chunk_ = malloc(to_recv_size);
        recved_size = (socket_->read_some(boost::asio::buffer(chunk_, to_recv_size) ) );
        total_to_recv_size -= recved_size;
      }
      catch(boost::system::system_error& err) {
        // LOG(ERROR) << "init_recv:: err= " << err.what();
        LOG(INFO) << "init_recv:: server= " << server_name << "; client closed the conn.err= " << err.what();
        return;
      }
      
      // handle_recv_thread_v.push_back(boost::make_shared<boost::thread>(&TCPServer::handle_recv, this, chunk_) );
      // recv_cb(data_id, recved_size, chunk_);
      boost::thread t(&TCPServer::handle_recv, this, data_id, recved_size, chunk_);
      if (total_to_recv_size == 0) {
        LOG(INFO) << "init_recv:: server= " << server_name << " completed receiving data_id= " << data_id;
        syncer.notify(hash_str(data_id) );
        break;
      }
    }
  }
}

void TCPServer::handle_recv(std::string data_id, int data_size, void* data_)
{
  recv_cb(data_id, data_size, data_);
}
