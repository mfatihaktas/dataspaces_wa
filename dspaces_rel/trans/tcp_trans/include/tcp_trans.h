#ifndef _TCP_TRANS_H_
#define _TCP_TRANS_H_

#include "tcp_server.h"
#include "tcp_client.h"

class TCPTrans {
  // typedef std::pair<std::string, int> lip_lport_pair;
  
  private:
    boost::shared_ptr<TCPServer> tcp_server_;
    // std::map<lip_lport_pair, boost::shared_ptr<TCPClient> > lip_lport__tcp_client_map;
    std::vector<boost::shared_ptr<TCPClient> > active_tcp_client_v;
    
    bool closed;
  public:
    TCPTrans(std::string s_lip, int s_lport);
    ~TCPTrans();
    std::string to_str();
    int close();
    
    std::string get_s_lip();
    int get_s_lport();
    
    int init_server(std::string data_id, data_recv_cb_func data_recv_cb);
    int send(std::string s_lip, int s_lport, std::string data_id, int data_size, void* data_);
};

#endif //_TCP_TRANS_H_