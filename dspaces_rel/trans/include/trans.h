#ifndef _TRANS_H_
#define _TRANS_H_

#include <boost/make_shared.hpp>
// #define _GRIDFTP_

#include "ib_trans.h"
#include "tcp_trans.h"
#ifdef _GRIDFTP_
  #include "gftp_trans.h"
#endif // _GRIDFTP_

#define str_str_equals(x, y) (strcmp(x.c_str(), y.c_str() ) == 0)
#define str_cstr_equals(x, y) (strcmp(x.c_str(), y) == 0)

const std::string INFINIBAND = "i";
const std::string TCP = "t";
const std::string GRIDFTP = "g";

class Trans { // Transport
  typedef boost::function<void(std::string, int, void*)> data_recv_cb_func;
  
  private:
    std::string trans_protocol;
    
    boost::shared_ptr<IBTrans> ib_trans_;
    boost::shared_ptr<TCPTrans> tcp_trans_;
  #ifdef _GRIDFTP_
    boost::shared_ptr<GFTPTrans> gftp_trans_;
  #endif // _GRIDFTP_
  
  public:
    Trans(std::string trans_protocol,
          std::string ib_lip, std::list<std::string> ib_lport_list,
          std::string tcp_lip, int tcp_lport,
          std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir);
    ~Trans();
    std::string to_str();
    
    std::string get_s_lip();
    std::string get_s_lport();
    std::string get_tmpfs_dir();
    
    int init_get(std::string data_type, std::string s_lport, std::string data_id, data_recv_cb_func recv_cb);
    int init_put(std::string s_lip, std::string s_lport, std::string tmpfs_dir,
                 std::string data_type, std::string data_id, int data_length, void* data_);
};

#endif // _TRANS_H_