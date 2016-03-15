#include "ib_trans.h"

IBTrans::IBTrans(std::string s_lip, std::list<std::string> s_lport_list)
: s_lip(s_lip)
{
  for (std::list<std::string>::iterator it = s_lport_list.begin(); it != s_lport_list.end(); it++)
    s_lport_queue.push(*it);
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

IBTrans::~IBTrans() { log_(INFO, "destructed.") }

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

void IBTrans::init_server(std::string data_type, const char* lport_,
                          std::string recv_id, ib_data_recv_cb_func data_recv_cb, ib_msg_recv_cb_func msg_recv_cb)
{
  IBServer ib_server(lport_, data_recv_cb, msg_recv_cb);
  // if (str_cstr_equals(data_type, "int") ) {
  //   IBServer<int, std::string> ib_server(lport_, recv_id, data_recv_cb);
  //   ib_server.init();
  // }
  // else if (str_cstr_equals(data_type, "char") ) {
  //   IBServer<char, std::string> ib_server(lport_, recv_id, data_recv_cb);
  //   ib_server.init();
  // }
  // else if (str_cstr_equals(data_type, "double") ) {
  //   IBServer<double, std::string> ib_server(lport_, recv_id, data_recv_cb);
  //   ib_server.init();
  // }
  // else if (str_cstr_equals(data_type, "float") ) {
  //   IBServer<float, std::string> ib_server(lport_, recv_id, data_recv_cb);
  //   ib_server.init();
  // }
  // else {
  //   log_(ERROR, "unknown data_type= " << data_type)
  // }
}

int IBTrans::init_client(const char* s_lip_, const char* s_lport_,
                         std::string data_type, std::string data_id, uint64_t data_length, void* data_)
{
  int err;
  
  uint64_t data_size = data_length;
  if (data_type == "char")
    data_size *= sizeof(char);
  else if (data_type == "int")
    data_size *= sizeof(int);
  else if (data_type == "float")
    data_size *= sizeof(float);
  else if (data_type == "double")
    data_size *= sizeof(double);
  else {
    log_(ERROR, "unknown data_type= " << data_type)
    return 1;
  }
  IBClient ib_client(s_lip_, s_lport_);
  ib_client.send_data(data_id, data_size, data_);
  // if (str_cstr_equals(data_type, "int") ) {
  //   IBClient<int> ib_client(s_lip_, s_lport_, data_length, static_cast<int*>(data_) );
  //   return_if_err(ib_client.init(), err)
  // }
  // else if (str_cstr_equals(data_type, "char") ) {
  //   IBClient<char> ib_client(s_lip_, s_lport_, data_length, static_cast<char*>(data_) );
  //   ib_client.init();
  // }
  // else if (str_cstr_equals(data_type, "double") ) {
  //   IBClient<double> ib_client(s_lip_, s_lport_, data_length, static_cast<double*>(data_) );
  //   ib_client.init();
  // }
  // else if (str_cstr_equals(data_type, "float") ) {
  //   IBClient<float> ib_client(s_lip_, s_lport_, data_length, static_cast<float*>(data_) );
  //   ib_client.init();
  // }
  // else {
  //   log_(ERROR, "unknown data_type= " << data_type)
  // }
  
  return 0;
}
