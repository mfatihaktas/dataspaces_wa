#include "ib_delivery.h"

DDManager::DDManager(std::list<std::string> ib_lport_list)
{
  for (std::list<std::string>::iterator it = ib_lport_list.begin(); it != ib_lport_list.end(); it++){
    ib_lport_queue.push(*it);
  }
  //
  LOG(INFO) << "DDManager:: constructed; \n" << to_str();
}

DDManager::~DDManager()
{
  //
  LOG(INFO) << "DDManager:: destructed.";
}

std::string DDManager::to_str()
{
  std::stringstream ss;
  
  ss << "ib_lport_queue=\n" << ib_lport_queue.to_str();
  ss << "\n";
  
  return ss.str();
}

void DDManager::init_ib_server(std::string key, unsigned int ver, std::string data_type, const char* lport, data_recv_cb dr_cb)
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
}

void DDManager::init_ib_client(const char* s_laddr, const char* s_lport,
                               std::string data_type, size_t data_length, void* data_)
{
  if (str_equals(data_type, "int") ){
    IBClient<int> ib_client(s_laddr, s_lport, data_length, static_cast<int*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "char") ){
    IBClient<char> ib_client(s_laddr, s_lport, data_length, static_cast<char*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "double") ){
    IBClient<double> ib_client(s_laddr, s_lport, data_length, static_cast<double*>(data_) );
    ib_client.init();
  }
  else if (str_equals(data_type, "float") ){
    IBClient<float> ib_client(s_laddr, s_lport, data_length, static_cast<float*>(data_) );
    ib_client.init();
  }
}

std::string DDManager::get_next_avail_ib_lport()
{
  return ib_lport_queue.pop();
}

void DDManager::give_ib_lport_back(std::string ib_lport)
{
  ib_lport_queue.push(ib_lport);
}

// void DDManager::init_wa_ib_recv(std::string data_type, data_recv_cb dr_cb)
// {
//   std::string ib_lport = ib_lport_queue.pop();
  
//   init_ib_server(data_type, ib_lport.c_str(), dr_cb);
  
//   ib_lport_queue.push(ib_lport);
//   //
//   LOG(INFO) << "init_wa_ib_recv:: done.";
// }
  