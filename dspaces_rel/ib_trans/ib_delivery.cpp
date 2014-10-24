#include "ib_delivery.h"

IBDDManager::IBDDManager(std::list<std::string> ib_lport_list)
{
  for (std::list<std::string>::iterator it = ib_lport_list.begin(); it != ib_lport_list.end(); it++){
    ib_lport_queue.push(*it);
  }
  //
  LOG(INFO) << "IBDDManager:: constructed; \n" << to_str();
}

IBDDManager::~IBDDManager()
{
  //
  LOG(INFO) << "IBDDManager:: destructed.";
}

std::string IBDDManager::to_str()
{
  std::stringstream ss;
  
  ss << "ib_lport_queue=\n" << ib_lport_queue.to_str();
  ss << "\n";
  
  return ss.str();
}

void IBDDManager::init_ib_server(std::string key, unsigned int ver, std::string data_type, const char* lport, data_recv_cb dr_cb)
{
  if (str_equals(data_type, "int") ){
    IBServer<int> ib_server(key, ver, lport, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "char") ){
    IBServer<char> ib_server(key, ver, lport, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "double") ){
    IBServer<double> ib_server(key, ver, lport, dr_cb);
    ib_server.init();
  }
  else if (str_equals(data_type, "float") ){
    IBServer<float> ib_server(key, ver, lport, dr_cb);
    ib_server.init();
  }
  else {
    LOG(ERROR) << "init_ib_server:: unknown data_type= " << data_type;
  }
}

void IBDDManager::init_ib_client(const char* s_laddr, const char* s_lport,
                               std::string data_type, size_t data_length, void* data_)
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
  else {
    LOG(ERROR) << "init_ib_server:: unknown data_type= " << data_type;
  }
}

std::string IBDDManager::get_next_avail_ib_lport()
{
  return ib_lport_queue.pop();
}

void IBDDManager::give_ib_lport_back(std::string ib_lport)
{
  ib_lport_queue.push(ib_lport);
}
