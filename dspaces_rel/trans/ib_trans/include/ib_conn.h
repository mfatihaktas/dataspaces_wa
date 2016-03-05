#ifndef _IB_CONN_H_
#define _IB_CONN_H_

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>

#include "patch_ib.h"

// Used for differentiating msg from data in comm between client-server
enum RDMA_DATA_T {
  RDMA_INIT = 'I',
  RDMA_MSG = 'M',
  RDMA_DATA = 'D',
  RDMA_DONE = 'O',
};
// For client-server comm protocol
const char HEADER_DELIMITER = '#';

const int IB_MAX_DATA_T_LENGTH = 1;
const int IB_MAX_DATA_ID_LENGTH = 20;
const int IB_MAX_DATA_SIZE_LENGTH = 15;

const int IB_MAX_MSG_LENGTH = 100;
// 
const int TIMEOUT_IN_MS = 500; 
const uint64_t BUFFER_SIZE = 1024*1024*1024; // 10*1024*1024;
const uint64_t MAX_QP__CQ_LENGTH = 100;

struct context {
  struct ibv_context* ctx_;
  struct ibv_pd* pd_;
  struct ibv_cq* cq_;
  struct ibv_comp_channel* comp_channel_;
};

class IBEnd {
  public:
    virtual int on_pre_conn(struct rdma_cm_id *id) = 0;
    virtual int on_conn(struct rdma_cm_id *id) = 0;
    virtual int on_completion(struct ibv_wc *wc) = 0;
    virtual int on_disconn(struct rdma_cm_id *id) = 0;
};

class Connector {
  private:
    IBEnd* ib_end_;
    struct context* s_ctx_;
    
    std::vector<pthread_t*> pthread_v;
  public:
    Connector(IBEnd* ib_end_);
    ~Connector();
    struct ibv_pd* get_pd_();
    
    void build_param(struct rdma_conn_param* param_);
    void build_conn(struct rdma_cm_id* id_);
    
    void build_context(struct ibv_context* verb_);
    void build_qp_attr(struct ibv_qp_init_attr* qp_attr_);
    void event_loop(struct rdma_event_channel* ec_, bool exit_on_disconn);
    
    void* poll_cq(void* ctx_);
    
    void rc_client_loop(const char* host_, const char* port_, void* ctx_);
    void rc_disconnect(struct rdma_cm_id* id_);
    void rc_server_loop(const char* port_);
};

struct wrap_Connector {
  struct context* s_ctx_;
  Connector* connector_;
  
  wrap_Connector(Connector* connector_, struct context* s_ctx_)
  : connector_(connector_), s_ctx_(s_ctx_)
  {}
};

#endif // _IB_CONN_H_
