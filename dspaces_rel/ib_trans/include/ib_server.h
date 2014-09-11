#ifndef IB_SERVER_H
#define IB_SERVER_H

#include "messages.h"
#include "common.h"

typedef boost::function<void(std::string, size_t, void*)> data_recv_cb;

template <class data_type>
class IBServer{
  
  struct conn_context
  {
    data_type *buffer;
    struct ibv_mr *buffer_mr;
  
    struct message *msg;
    struct ibv_mr *msg_mr;
  };
  private:
    std::string key;
    const char* lport;
    data_recv_cb dr_cb;
    //
    Connector connector;
  public:
    IBServer(std::string key, const char* lport, data_recv_cb dr_cb)
    : lport(lport),
      dr_cb(dr_cb),
      key(key)
    {
      //
      LOG(INFO) << "IBServer constructed: lport= " << lport;
    };
    ~IBServer()
    {
      //
      LOG(INFO) << "IBServer destructed.";
    };
    
    void post_receive(struct rdma_cm_id *id)
    {
      struct ibv_recv_wr wr, *bad_wr = NULL;
    
      memset(&wr, 0, sizeof(wr));
    
      wr.wr_id = (uintptr_t)id;
      wr.sg_list = NULL;
      wr.num_sge = 0;
    
      TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
    };
    
    void send_message(struct rdma_cm_id *id)
    {
      struct conn_context *ctx = (struct conn_context *)id->context;
    
      struct ibv_send_wr wr, *bad_wr = NULL;
      struct ibv_sge sge;
    
      memset(&wr, 0, sizeof(wr));
    
      wr.wr_id = (uintptr_t)id;
      wr.opcode = IBV_WR_SEND;
      wr.sg_list = &sge;
      wr.num_sge = 1;
      wr.send_flags = IBV_SEND_SIGNALED;
    
      sge.addr = (uintptr_t)ctx->msg;
      sge.length = sizeof(*ctx->msg);
      sge.lkey = ctx->msg_mr->lkey;
    
      TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
    };
    //state handlers
    void on_pre_conn(struct rdma_cm_id *id)
    {
      struct conn_context *ctx = (struct conn_context *)malloc(sizeof(struct conn_context));
    
      id->context = ctx;
    
      posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_LENGTH*sizeof(data_type) );
      TEST_Z(ctx->buffer_mr = ibv_reg_mr(connector.rc_get_pd(), ctx->buffer, BUFFER_LENGTH*sizeof(data_type), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
    
      posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
      TEST_Z(ctx->msg_mr = ibv_reg_mr(connector.rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
    
      post_receive(id);
    };
    
    void on_connection(struct rdma_cm_id *id)
    {
      struct conn_context *ctx = (struct conn_context *)id->context;
      
      ctx->msg->id = MSG_MR;
      ctx->msg->data.mr.addr = (uintptr_t)ctx->buffer_mr->addr;
      ctx->msg->data.mr.rkey = ctx->buffer_mr->rkey;
    
      send_message(id);
    };
    
    void on_disconnect(struct rdma_cm_id *id)
    {
      struct conn_context *ctx = (struct conn_context *)id->context;
    
      ibv_dereg_mr(ctx->buffer_mr);
      ibv_dereg_mr(ctx->msg_mr);
    
      free(ctx->buffer);
      free(ctx->msg);
    
      LOG(INFO) << "on_disconnect:: done.";
    
      free(ctx);
    };
    
    void on_completion(struct ibv_wc *wc)
    {
      struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
      struct conn_context *ctx = (struct conn_context *)id->context;
      
      if (wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
        uint32_t size = ntohl(wc->imm_data);
        
        //print_conn_context(ctx);
        
        if (size == 0) {
          ctx->msg->id = MSG_DONE;
          send_message(id);
          // don't need post_receive() since we're done with this connection
        }
        else {
          if (ctx->msg->id == MSG_DONE){
            printf("MSG_DONE received.\n");
            ctx->msg->id = MSG_DONE;
            send_message(id);
            return;
          }
          else if (size == 3) {
            // LOG(INFO) << "on_completion:: the one recved with size-3= " << (char*)ctx->buffer;
            char is_eof_buf[4];
            memcpy(is_eof_buf, (char*)ctx->buffer, 3);
            is_eof_buf[3] = '\0';
            // LOG(INFO) << "on_completion:: is_eof_buf= " << is_eof_buf;
            if(!strcmp(is_eof_buf, (char*)"EOF") ){
              LOG(INFO) << "on_completion:: EOF received.";
              ctx->msg->id = MSG_DONE;
              send_message(id);
              
              connector.rc_disconnect(id);
              
              return;
            }
          }
          data_type* data_ = (data_type*)malloc(size);
          memcpy(data_, ctx->buffer, size);
          dr_cb(key, size, data_);
          
          post_receive(id);
        }
      }
    };
    
    void init()
    { 
      connector.rc_init(
        boost::bind(&IBServer::on_pre_conn, this, _1),
        boost::bind(&IBServer::on_connection, this, _1),
        boost::bind(&IBServer::on_completion, this, _1),
        boost::bind(&IBServer::on_disconnect, this, _1) );
        
        LOG(INFO) << "init:: waiting for connections. interrupt (^C) to exit.";
        
      connector.rc_server_loop(lport);
    };
};

#endif