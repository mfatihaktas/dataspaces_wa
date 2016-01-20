#ifndef _IB_SERVER_H_
#define _IB_SERVER_H_

#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include "ib_msg.h"
#include "ib_conn.h"

enum RECV_STATE {
  HEADER_RECV = 'H',
  DATA_RECV = 'D',
};

// Header struct:
// msg_header: DATA_T + DATA_SIZE
// msg_header: DATA_T + DATA_ID + DATA_SIZE

// typedef void (*ib_msg_recv_cb_func_)(int, char*);
// typedef void (*ib_data_recv_cb_func_)(char*, int, void*);
typedef boost::function<void(uint64_t, char*)> ib_msg_recv_cb_func;
typedef boost::function<void(std::string, uint64_t, void*)> ib_data_recv_cb_func;

class IBServer : public IBEnd {
  struct conn_context {
    void* buffer_;
    struct ibv_mr* buffer_mr_;
  
    struct message* msg_;
    struct ibv_mr* msg_mr_;
  };
  private:
    const char* lport_;
    ib_data_recv_cb_func data_recv_cb;
    ib_msg_recv_cb_func msg_recv_cb;
    
    std::string server_name;
    Connector* connector_;
    
    RECV_STATE recv_state;
    RDMA_DATA_T data_t;
    uint64_t data_size_to_recv, data_size_recved;
    char* data_id_;
    void* data_to_recv_;
  public:
    IBServer(const char* lport_, ib_data_recv_cb_func data_recv_cb, ib_msg_recv_cb_func msg_recv_cb = 0);
    ~IBServer();
    std::string to_str();
    
    void init();
    
    int send_message(struct rdma_cm_id* id_);
    int post_receive(struct rdma_cm_id* id_);
    // State handlers
    int on_pre_conn(struct rdma_cm_id* id_);
    int on_conn(struct rdma_cm_id* id_);
    int on_disconn(struct rdma_cm_id* id_);
    int on_completion(struct ibv_wc* wc_);
    int proc_header(char* header_);
};

#endif // _IB_SERVER_H_