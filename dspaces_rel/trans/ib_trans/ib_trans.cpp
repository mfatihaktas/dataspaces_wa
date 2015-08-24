#include "ib_trans.h"

IBTManager::IBTManager(std::list<std::string> ib_lport_list)
{
  for (std::list<std::string>::iterator it = ib_lport_list.begin(); it != ib_lport_list.end(); it++)
    ib_lport_queue.push(*it);
  // 
  LOG(INFO) << "IBTManager:: constructed; \n" << to_str();
}

IBTManager::~IBTManager() { LOG(INFO) << "IBTManager:: destructed."; }

std::string IBTManager::to_str()
{
  std::stringstream ss;
  ss << "ib_lport_queue= \n" << ib_lport_queue.to_str();
  return ss.str();
}

std::string IBTManager::get_next_avail_ib_lport() { return ib_lport_queue.pop(); }
void IBTManager::give_ib_lport_back(std::string ib_lport) { ib_lport_queue.push(ib_lport); }

void IBTManager::init_ib_server(const char* lport, boost::function<void(RECV_ID_T, int, void*)> dr_cb,
                                std::string data_type, RECV_ID_T recv_id)
{
  if (str_equals(data_type, "int") ) {
    IBServer<int, RECV_ID_T> ib_server(lport, recv_id, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "char") ) {
    IBServer<char, RECV_ID_T> ib_server(lport, recv_id, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "double") ) {
    IBServer<double, RECV_ID_T> ib_server(lport, recv_id, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "float") ) {
    IBServer<float, RECV_ID_T> ib_server(lport, recv_id, dr_cb);
    ib_server.init();
  }
  else
    LOG(ERROR) << "init_ib_server:: unknown data_type= " << data_type;
}

void IBTManager::init_ib_client(const char* s_laddr, const char* s_lport,
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

