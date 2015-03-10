#ifndef IB_CLIENT_H
#define IB_CLIENT_H

#include <fcntl.h>
#include <libgen.h>

#include "common.h"
#include "messages.h"

template <class data_type>
class IBClient{
  struct client_context
  {
    data_type *buffer;
    struct ibv_mr *buffer_mr;
  
    struct message *msg;
    struct ibv_mr *msg_mr;
  
    uint64_t peer_addr;
    uint32_t peer_rkey;
  };
  private:
    const char *s_laddr, *s_lport;
    size_t data_length;
    data_type* data_;
    
    int num_srs;
    // 
    boost::shared_ptr<Connector> connector_;
  public:
    IBClient(const char* s_laddr, const char* s_lport, 
             size_t data_length, data_type* data_)
    : s_laddr(s_laddr),
      s_lport(s_lport),
      data_length(data_length),
      data_(data_),
      num_srs(0),
      connector_( new Connector(
        boost::bind(&IBClient::on_pre_conn, this, _1),
        NULL,
        boost::bind(&IBClient::on_completion, this, _1),
        NULL )
      )
    {
      // 
      LOG(INFO) << "IBClient:: constructed:\n" << to_str();
    };
    ~IBClient()
    {
      // 
      LOG(INFO) << "IBClient:: destructed.";
    };
    
    std::string to_str()
    {
      std::stringstream ss;
    
      ss << "s_laddr= " << s_laddr << "\n";
      ss << "s_lport= " << s_lport << "\n";
      ss << "data_length= " << boost::lexical_cast<std::string>(data_length) << "\n";
      ss << "\n";
      
      return ss.str();
    };
    
    void write_remote(struct rdma_cm_id *id, uint32_t size)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      struct ibv_send_wr wr, *bad_wr = NULL;
      struct ibv_sge sge;
    
      memset(&wr, 0, sizeof(wr));
    
      if (size) {
        sge.addr = (uintptr_t)ctx->buffer;
        sge.length = size;
        sge.lkey = ctx->buffer_mr->lkey;
        
        wr.sg_list = &sge;
        wr.num_sge = 1;
      }
      
      wr.wr_id = (uintptr_t)id;
      wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
      wr.send_flags = IBV_SEND_SIGNALED;
      wr.imm_data = htonl(size);
      wr.wr.rdma.remote_addr = ctx->peer_addr;
      wr.wr.rdma.rkey = ctx->peer_rkey;
    
      TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
      num_srs += 1;
      LOG(INFO) << "write_remote:: size= " << size << " num_srs= " << num_srs;
    };
    
    void post_receive(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      struct ibv_recv_wr wr, *bad_wr = NULL;
      struct ibv_sge sge;
    
      memset(&wr, 0, sizeof(wr));
    
      wr.wr_id = (uintptr_t)id;
      wr.sg_list = &sge;
      wr.num_sge = 1;
    
      sge.addr = (uintptr_t)ctx->msg;
      sge.length = sizeof(*ctx->msg);
      sge.lkey = ctx->msg_mr->lkey;
    
      TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
    };
    
    void send_data(struct rdma_cm_id *id)
    {
      // LOG(INFO) << "send_data:: started.";
      
      data_type* data_t_ = data_;
      size_t data_length_t = data_length;
      
      size_t chunk_length;
      data_type* chunk_ = (data_type*)malloc(BUFFER_LENGTH*sizeof(data_type) );
      while(data_length_t){
        switch (data_length_t >= BUFFER_LENGTH){
          case 1:
            chunk_length = BUFFER_LENGTH;
            break;
          case 0:
            chunk_length = data_length_t;
        }
        memcpy(chunk_, data_t_, chunk_length*sizeof(data_type) );
        send_chunk(id, chunk_length*sizeof(data_type), chunk_);
        
        data_length_t -= chunk_length;
        data_t_ += chunk_length;
      }
      free(chunk_);
      send_eof(id);
      //
      // LOG(INFO) << "send_data:: done.";
    };
    
    void send_chunk(struct rdma_cm_id *id, size_t chunk_size, data_type* chunk) // chunk_size in bytes
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      //ssize_t size = read(ctx->fd, ctx->buffer, BUFFER_LENGTH);
      memcpy(ctx->buffer, chunk, chunk_size );
    
      write_remote(id, chunk_size);
    };
    
    void send_eof(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
      
      size_t eof_size = 3;
      char* eof = (char*)malloc(sizeof(char)*eof_size);
      memcpy(eof, (char*)"EOF", 3);
      data_type* eof_ = reinterpret_cast<data_type*>(eof);
      
      memcpy(ctx->buffer, eof_, eof_size*sizeof(data_type) );
    
      write_remote(id, eof_size);
    };
    
    //state handlers
    void on_pre_conn(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_LENGTH*sizeof(data_type) );
      TEST_Z(ctx->buffer_mr = ibv_reg_mr(connector_->rc_get_pd(), ctx->buffer, BUFFER_LENGTH*sizeof(data_type), IBV_ACCESS_LOCAL_WRITE));
    
      posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
      TEST_Z(ctx->msg_mr = ibv_reg_mr(connector_->rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
    
      post_receive(id);
    };
    
    void on_completion(struct ibv_wc *wc)
    {
      struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
      struct client_context *ctx = (struct client_context *)id->context;
    
      if (wc->opcode & IBV_WC_RECV) {
        if (ctx->msg->id == MSG_MR) {
          ctx->peer_addr = ctx->msg->data.mr.addr;
          ctx->peer_rkey = ctx->msg->data.mr.rkey;
    
          send_data(id);
        }
        else if (ctx->msg->id == MSG_DONE) {
          LOG(INFO) << "on_completion:: received DONE, disconnecting.";
          connector_->rc_disconnect(id);
          return;
        }
    
        post_receive(id);
      }
    };
    
    void init()
    {
      struct client_context ctx;
      
      // connector.rc_init(
      //   boost::bind(&IBClient::on_pre_conn, this, _1),
      //   NULL, // on connect
      //   boost::bind(&IBClient::on_completion, this, _1),
      //   NULL); // on disconnect
    
      connector_->rc_client_loop(s_laddr, s_lport, &ctx);
    };
};

#endif