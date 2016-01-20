#ifndef _IB_TRANS_H_
#define _IB_TRANS_H_

#include <list>

#include "ib_server.h"
#include "ib_client.h"

class IBTrans {
  private:
    std::string s_lip;
  
    patch_ib::BQueue<std::string> s_lport_queue;
  public:
    IBTrans(std::string s_lip, std::list<std::string> ib_lport_list);
    ~IBTrans();
    std::string to_str();
    
    std::string get_s_lip();
    std::string get_s_lport();
    void return_s_lport(std::string s_lport);
    
    void init_server(const char* lport_, ib_data_recv_cb_func data_recv_cb, ib_msg_recv_cb_func msg_recv_cb = 0);
    void init_client(const char* s_lip_, const char* s_lport_,
                     std::string data_id, uint64_t data_size, void* data_);
};

#endif // _IB_TRANS_H_