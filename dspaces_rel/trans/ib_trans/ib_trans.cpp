#include "ib_trans.h"

IBTrans::IBTrans(std::string s_lip, std::list<std::string> s_lport_list)
: s_lip(s_lip)
{
  for (std::list<std::string>::iterator it = s_lport_list.begin(); it != s_lport_list.end(); it++)
    s_lport_queue.push(*it);
  // 
  LOG(INFO) << "IBTrans:: constructed; \n" << to_str();
}

IBTrans::~IBTrans() { LOG(INFO) << "IBTrans:: destructed."; }

std::string IBTrans::to_str()
{
  std::stringstream ss;
  ss << "s_lip= " << s_lip << "\n"
     << "s_lport_queue= \n" << s_lport_queue.to_str();
  
  return ss.str();
}

std::string IBTrans::get_s_lip() { return s_lip; }
std::string IBTrans::get_s_lport() { return s_lport_queue.pop(); }
void IBTrans::return_s_lport(std::string s_lport) { s_lport_queue.push(s_lport); }

void IBTrans::init_server(std::string data_type, const char* lport,
                          RECV_ID_T recv_id, boost::function<void(RECV_ID_T, int, void*)> data_recv_cb)
{
  if (str_equals(data_type, "int") ) {
    IBServer<int, RECV_ID_T> ib_server(lport, recv_id, data_recv_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "char") ) {
    IBServer<char, RECV_ID_T> ib_server(lport, recv_id, data_recv_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "double") ) {
    IBServer<double, RECV_ID_T> ib_server(lport, recv_id, data_recv_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "float") ) {
    IBServer<float, RECV_ID_T> ib_server(lport, recv_id, data_recv_cb);
    ib_server.init();
  }
  else
    LOG(ERROR) << "init_ib_server:: unknown data_type= " << data_type;
}

void IBTrans::init_client(const char* s_laddr, const char* s_lport,
                          std::string data_type, int data_length, void* data_)
{
  if (str_equals(data_type, "int") ) {
    IBClient<int> ib_client(s_laddr, s_lport, data_length, static_cast<int*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "char") ) {
    IBClient<char> ib_client(s_laddr, s_lport, data_length, static_cast<char*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "double") ) {
    IBClient<double> ib_client(s_laddr, s_lport, data_length, static_cast<double*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "float") ) {
    IBClient<float> ib_client(s_laddr, s_lport, data_length, static_cast<float*>(data_) );
    ib_client.init();
  }
  else
    LOG(ERROR) << "init_ib_client:: unknown data_type= " << data_type;
}

