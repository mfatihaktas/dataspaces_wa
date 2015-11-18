#include "tcp_trans.h"

TCPTrans::TCPTrans(std::string s_lip, int s_lport)
: tcp_server_(boost::make_shared<TCPServer>(s_lip, s_lport) ),
  closed(false)
{
  // 
  LOG(INFO) << "TCPTrans:: constructed; \n" << to_str();
}

TCPTrans::~TCPTrans()
{
  if (!closed)
    close();
  // 
  closed = true;
  LOG(INFO) << "TCPTrans:: destructed.";
}

std::string TCPTrans::to_str()
{
  std::stringstream ss;
  ss << "tcp_server= \n" << tcp_server_->to_str() << "\n";
  
  return ss.str();
}

int TCPTrans::close()
{
  if (closed) {
    LOG(ERROR) << "close:: already closed!";
    return 1;
  }
  
  tcp_server_->close();
  // for (std::map<lip_lport_pair, boost::shared_ptr<TCPClient> >::iterator it = lip_lport__tcp_client_map.begin(); it != lip_lport__tcp_client_map.end(); it++)
  //   (it->second)->close();
  for (std::vector<boost::shared_ptr<TCPClient> >::iterator it = active_tcp_client_v.begin(); it != active_tcp_client_v.begin(); it++)
    (*it)->close();
  
  LOG(INFO) << "close:: done.";
  return 0;
}

std::string TCPTrans::get_s_lip() { return tcp_server_->get_lip(); }
int TCPTrans::get_s_lport() { return tcp_server_->get_lport(); }

int TCPTrans::init_server(std::string data_id, data_recv_cb_func data_recv_cb) { return tcp_server_->init(data_id, data_recv_cb); }

int TCPTrans::send(std::string s_lip, int s_lport, std::string data_id, int data_size, void* data_)
{
  // TODO: This obviously fails when multiple coupling data streams over the same tcp_client
  // lip_lport_pair ll_pair = std::make_pair(s_lip, s_lport);
  // if (lip_lport__tcp_client_map.count(ll_pair) == 0)
  //   lip_lport__tcp_client_map[ll_pair] = boost::make_shared<TCPClient>(s_lip, s_lport);
  
  // return lip_lport__tcp_client_map[ll_pair]->send(data_id, data_size, data_);
  
  boost::shared_ptr<TCPClient> tcp_client_ = boost::make_shared<TCPClient>(s_lip, s_lport);
  active_tcp_client_v.push_back(tcp_client_);
  return tcp_client_->send(data_id, data_size, data_);
}
