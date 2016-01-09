#ifndef _IB_SERVER_H_
#define _IB_SERVER_H_

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

// typedef void (*msg_recv_cb_func_)(int, char*);
// typedef void (*data_recv_cb_func_)(char*, int, void*);

typedef boost::function<void(int, char*)> msg_recv_cb_func;
typedef boost::function<void(char*, int, void*)> data_recv_cb_func;

class IBServer : public IBEnd {
  struct conn_context {
    void* buffer_;
    struct ibv_mr* buffer_mr_;
  
    struct message* msg_;
    struct ibv_mr* msg_mr_;
  };
  private:
    const char* lport_;
    msg_recv_cb_func msg_recv_cb;
    data_recv_cb_func data_recv_cb;
    
    std::string server_name;
    Connector* connector_;
    
    RECV_STATE recv_state;
    RDMA_DATA_T data_t;
    int data_size_to_recv, data_size_recved;
    char* data_id_;
    void* data_to_recv_;
  public:
    IBServer(const char* lport_, msg_recv_cb_func msg_recv_cb, data_recv_cb_func data_recv_cb);
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