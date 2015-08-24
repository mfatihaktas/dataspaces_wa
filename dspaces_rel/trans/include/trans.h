#ifndef _TRANS_H_
#define _TRANS_H_

#include <boost/make_shared.hpp>
// #define _GRIDFTP_

#include "ib_trans.h"
#ifdef _GRIDFTP_
  #include "gftp_trans.h"
#endif // _GRIDFTP_

#define str_str_equals(x, y) (strcmp(x.c_str(), y.c_str() ) == 0)

const std::string INFINIBAND = "i";
const std::string GRIDFTP = "g";

class TManager { // Transport
  typedef boost::function<void(std::string, int, void*)> data_recv_cb_func;
  
  private:
    std::string trans_protocol;
    std::string ib_laddr, gftp_laddr, gftp_lport;
    
    boost::shared_ptr<IBTManager> ibt_manager_;
  #ifdef _GRIDFTP_
    boost::shared_ptr<GFTPTManager> gftpt_manager_;
  #endif // _GRIDFTP_
  
  public:
    TManager(std::string trans_protocol,
             std::string ib_laddr, std::list<std::string> ib_lport_list, 
             std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir);
    ~TManager();
    std::string to_str();
    
    std::string get_s_laddr();
    std::string get_s_lport();
    
    int init_get(std::string s_lport, std::string data_id, std::string data_type, data_recv_cb_func cb);
    int init_put(std::string s_laddr, std::string s_lport, std::string tmpfs_dir,
                 std::string data_id, std::string data_type, int data_length, void* data_);
};

#endif // _TRANS_H_