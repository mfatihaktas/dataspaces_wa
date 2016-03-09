#ifndef _IB_CLIENT_H_
#define _IB_CLIENT_H_

// #include <fcntl.h>
// #include <libgen.h>
// #include <deque>
// #include <utility>
// #include <iostream>

// #include "ib_conn.h"
// #include "ib_msg.h"

// const char WAIT_INIT = 'I';
// const char WAIT_SEND_DATA = 'S';

// typedef std::pair<int, void*> size_data_pair;

// class IBClient : public IBEnd {
//   struct conn_context {
//     void* buffer_;
//     struct ibv_mr* buffer_mr_;
  
//     struct message* msg_;
//     struct ibv_mr* msg_mr_;
  
//     uint64_t peer_addr;
//     uint32_t peer_rkey;
//   };
//   private:
//     const char *s_lip_, *s_lport_;
    
//     std::string client_name;
//     patch::syncer<char> syncer;
//     Connector* connector_;
    
//     struct rdma_cm_id* id_;
//     patch::BQueue<size_data_pair> size_data_bq;
    
//     // std::vector<pthread_t*> pthread_v;
//   public:
//     IBClient(const char* s_lip_, const char* s_lport_);
//     ~IBClient();
//     std::string to_str();
    
//     void init();
    
//     int send_init();
//     int send_msg(std::string msg);
//     int send_data(std::string data_id, uint64_t data_size, void* data_);
//     int make_header(RDMA_DATA_T data_t, std::string data_id, uint64_t data_size,
//                     int& header_size, char*& arg_header_);
//     int send_next();
//     int send_chunk(uint64_t chunk_size, void* chunk_); // chunk_size in bytes
//     int write_remote(struct rdma_cm_id* id_, uint64_t size);
//     int post_receive(struct rdma_cm_id* id_);
//     // State handlers
//     int on_pre_conn(struct rdma_cm_id* id_);
//     int on_conn(struct rdma_cm_id* id_) {}
//     int on_completion(struct ibv_wc* wc_);
//     int on_disconn(struct rdma_cm_id* id_) {}
// };

// struct wrap_IBClient {
//   IBClient* ib_client_;
  
//   wrap_IBClient(IBClient* ib_client_)
//   : ib_client_(ib_client_)
//   {}
// };

#include <fcntl.h>
#include <libgen.h>

#include "common.h"
#include "messages.h"

template <typename DATA_T>
class IBClient {
  struct client_context {
    DATA_T *buffer;
    struct ibv_mr *buffer_mr;
  
    struct message *msg;
    struct ibv_mr *msg_mr;
  
    uint64_t peer_addr;
    uint32_t peer_rkey;
  };
  private:
    const char *s_lip, *s_lport;
    int data_length;
    DATA_T* data_;
    
    int num_srs;
    // 
    boost::shared_ptr<Connector> connector_;
  public:
    IBClient(const char* s_lip, const char* s_lport, 
             int data_length, DATA_T* data_)
    : s_lip(s_lip), s_lport(s_lport),
      data_length(data_length), data_(data_),
      num_srs(0),
      connector_(
        new Connector (
          boost::bind(&IBClient::on_pre_conn, this, _1),
          NULL,
          boost::bind(&IBClient::on_completion, this, _1),
          boost::bind(&IBClient::on_disconn, this, _1) ) )
    {
      // 
      LOG(INFO) << "IBClient:: constructed client= \n" << to_str();
    };
    
    ~IBClient() { LOG(INFO) << "IBClient:: destructed."; };
    
    std::string to_str()
    {
      std::stringstream ss;
    
      ss << "s_lip= " << s_lip << "\n";
      ss << "s_lport= " << s_lport << "\n";
      ss << "data_length= " << boost::lexical_cast<std::string>(data_length) << "\n";
      
      return ss.str();
    };
    
    void write_remote(struct rdma_cm_id *id, uint32_t size)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      struct ibv_send_wr wr, *bad_wr = NULL;
      struct ibv_sge sge;
    
      memset(&wr, 0, sizeof(wr) );
    
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
    
      TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr) );
      num_srs += 1;
      LOG(INFO) << "write_remote:: size= " << size << "B num_srs= " << num_srs;
    };
    
    void post_receive(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      struct ibv_recv_wr wr, *bad_wr = NULL;
      struct ibv_sge sge;
    
      memset(&wr, 0, sizeof(wr) );
    
      wr.wr_id = (uintptr_t)id;
      wr.sg_list = &sge;
      wr.num_sge = 1;
    
      sge.addr = (uintptr_t)ctx->msg;
      sge.length = sizeof(*ctx->msg);
      sge.lkey = ctx->msg_mr->lkey;
    
      TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr) );
    };
    
    void send_data(struct rdma_cm_id *id)
    {
      DATA_T* data_t_ = data_;
      int data_length_t = data_length;
      
      int chunk_length;
      DATA_T* chunk_ = (DATA_T*)malloc(BUFFER_LENGTH*sizeof(DATA_T) );
      while (data_length_t) {
        switch (data_length_t >= BUFFER_LENGTH) {
          case 1:
            chunk_length = BUFFER_LENGTH;
            break;
          case 0:
            chunk_length = data_length_t;
        }
        memcpy(chunk_, data_t_, chunk_length*sizeof(DATA_T) );
        send_chunk(id, chunk_length*sizeof(DATA_T), chunk_);
        
        data_length_t -= chunk_length;
        data_t_ += chunk_length;
      }
      free(chunk_);
      send_eof(id);
    };
    
    void send_chunk(struct rdma_cm_id *id, int chunk_size, DATA_T* chunk) // chunk_size in bytes
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      // sint size = read(ctx->fd, ctx->buffer, BUFFER_LENGTH);
      memcpy(ctx->buffer, chunk, chunk_size );
    
      write_remote(id, chunk_size);
    };
    
    void send_eof(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
      
      int eof_size = 3;
      char* eof = (char*)malloc(sizeof(char)*eof_size);
      memcpy(eof, (char*)"EOF", 3);
      DATA_T* eof_ = reinterpret_cast<DATA_T*>(eof);
      
      memcpy(ctx->buffer, eof_, eof_size*sizeof(DATA_T) );
    
      write_remote(id, eof_size);
      free(eof);
    };
    
    // State handlers
    void on_pre_conn(struct rdma_cm_id *id)
    {
      struct client_context *ctx = (struct client_context *)id->context;
    
      posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_LENGTH*sizeof(DATA_T) );
      TEST_Z(ctx->buffer_mr = ibv_reg_mr(connector_->rc_get_pd(), ctx->buffer, BUFFER_LENGTH*sizeof(DATA_T), IBV_ACCESS_LOCAL_WRITE) );
    
      posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg) );
      TEST_Z(ctx->msg_mr = ibv_reg_mr(connector_->rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE) );
    
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
    
    void on_disconn(struct rdma_cm_id* id_)
    {
      int err;
      struct client_context* ctx_ = (struct client_context*)id_->context;
      
      return_err_if_ret_cond_flag(ibv_dereg_mr(ctx_->buffer_mr), err, !=, 0,)
      return_err_if_ret_cond_flag(ibv_dereg_mr(ctx_->msg_mr), err, !=, 0,)
      log_(INFO, "done.")
    };
    
    int init()
    {
      int err;
      struct client_context ctx;
      return_if_err(connector_->rc_client_loop(s_lip, s_lport, &ctx), err)
      // err = 1;
      // while (err) {
      //   err = connector_->rc_client_loop(s_lip, s_lport, &ctx);
      //   if (err) {
      //     log_(ERROR, "connector_->rc_client_loop failed; will try again...")
      //     // sleep(1);
      //   }
      // }
      
      return 0;
    };
};

#endif // _IB_CLIENT_H_