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

void IBTrans::init_server(const char* lport_, msg_recv_cb_func msg_recv_cb, data_recv_cb_func data_recv_cb)
{
  IBServer ib_server(lport_, msg_recv_cb, data_recv_cb);
}

void IBTrans::init_client(const char* s_lip_, const char* s_lport_,
                          std::string data_id, int data_size, void* data_)
{
  IBClient ib_client(s_lip_, s_lport_);
  ib_client.send_data(data_id, data_size, data_);
}

