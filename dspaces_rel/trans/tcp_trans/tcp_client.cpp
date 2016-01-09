#include "tcp_client.h"

TCPClient::TCPClient(std::string s_lip, int s_lport)
: s_lip(s_lip), s_lport(s_lport),
  io_service_(boost::make_shared<boost::asio::io_service>() ),
  client_name(s_lip + "_" + boost::lexical_cast<std::string>(s_lport) ),
  closed(false)
{
  socket_ = boost::make_shared<boost::asio::ip::tcp::socket>(boost::ref(*io_service_) );
  // 
  LOG(INFO) << "TCPClient:: constructed client= \n" << to_str();
  
  if (connect() ) {
    LOG(ERROR) << "TCPClient:: client_name= " << client_name << " connect failed!";
  }
}

TCPClient::~TCPClient()
{
  if (!closed)
    close();
  // 
  LOG(INFO) << "TCPClient:: destructed." << to_str();
}

std::string TCPClient::to_str()
{
  std::stringstream ss;
  ss << "\t s_lip= " << s_lip << "\n"
     << "\t s_lport= " << s_lport << "\n";
  
  return ss.str();
}

int TCPClient::close()
{
  if (closed) {
    LOG(ERROR) << "close:: client client_name= " << client_name << " is already closed!";
    return 1;
  }
  try {
    boost::system::error_code ec;
    // socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    socket_->close(ec);
    if (ec) {
      LOG(ERROR) << "close:: ec= " << ec;
      return 1;
    }
    io_service_->stop();
    // 
    closed = true;
    LOG(INFO) << "close:: client_name= " << client_name << " closed.";
    return 0;
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "close:: Exception= " << ex.what();
    return 1;
  }
}

int TCPClient::connect()
{
  try {
    boost::asio::ip::tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::resolver::query query(s_lip.c_str(), boost::lexical_cast<std::string>(s_lport) );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    LOG(INFO) << "connect:: client_name= " << client_name << " connecting to " << endpoint << "...";
    socket_->connect(endpoint);
    LOG(INFO) << "connect:: connected.";
    return 0;
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "connect:: Exception=" << ex.what();
    close();
    return 1;
  }
}

int TCPClient::send_control_data(std::string control_data, int max_size)
{
  if (control_data.size() > max_size) {
    LOG(INFO) << "send_control_data:: client_name= " << client_name << " control_data.size= " << control_data.size() << " > max_size= " << max_size;
    return 1;
  }
  char* control_data_ = (char*)malloc(max_size*sizeof(char) );
  memcpy(control_data_, control_data.c_str(), control_data.size() );
  if (control_data.size() < max_size)
    control_data_[control_data.size() ] = '\0';
  
  int num_B = boost::asio::write(*socket_, boost::asio::buffer(control_data_, max_size) );
  if (num_B != max_size) {
    LOG(ERROR) << "send_control_data:: boost::asio::write wrote num_B= " << num_B << " != max_size= " << max_size;
    free(control_data_);
    return 1;
  }
  free(control_data_);
  
  return 0;
}

int TCPClient::send_chunk(int chunk_size, void* chunk_)
{
  try {
    return (int)socket_->send(boost::asio::buffer(chunk_, chunk_size) );
  }
  catch(std::exception& ex) {
    LOG(ERROR) << "send:: Exception=" << ex.what();
    close();
    return -1;
  }
}

int TCPClient::send(std::string data_id, int data_size, void* data_)
{
  send_control_data(boost::lexical_cast<std::string>(data_id), MAX_DATA_ID_LENGTH);
  send_control_data(boost::lexical_cast<std::string>(data_size), MAX_DATA_SIZE_LENGTH);
  
  void* _data_ = data_;
  int data_size_to_send = data_size;
  while(data_size_to_send > 0) {
    int chunk_size = (data_size_to_send > CHUNK_SIZE) ? CHUNK_SIZE : data_size_to_send;
    int data_size_sent = send_chunk(chunk_size, data_);
    if (data_size_sent == -1) {
      LOG(ERROR) << "init:: client_name= " << client_name << " send_chunk failed!";
      return 1;
    }
    
    data_size_to_send -= data_size_sent;
    // data_ += data_size_sent;
    data_ = static_cast<char*>(data_) + data_size_sent;
  }
  // Note: To see if read_some: Bad address can be solved by this
  // usleep(1000000);
  // free(_data_);
  
  return 0;
}

/*
int TCPClient::recv(char** bufp) {
  boost::system::error_code error;
  try{
    int len = socket_.read_some(boost::asio::buffer(*bufp), error);
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