#include "ib_server.h"

IBServer::IBServer(const char* lport_, ib_data_recv_cb_func data_recv_cb, ib_msg_recv_cb_func msg_recv_cb)
: lport_(lport_), data_recv_cb(data_recv_cb), msg_recv_cb(msg_recv_cb),
  server_name(patch::to_str<const char*>(lport_) ),
  connector_(new Connector(this) ),
  recv_state(HEADER_RECV)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
  
  init();
}

IBServer::~IBServer()
{
  delete connector_;
  // 
  log_(INFO, "destructed.")
};

std::string IBServer::to_str()
{
  std::stringstream ss;
  ss << "server_name= " << server_name << "\n"
     << "lport_= " << lport_ << "\n";
  return ss.str();
}

void IBServer::init()
{
  log_(INFO, "server_name= " << server_name << "; waiting for connections...")
  connector_->rc_server_loop(lport_);
}

int IBServer::send_message(struct rdma_cm_id* id_)
{
  struct conn_context* ctx_ = (struct conn_context*)id_->context;

  struct ibv_send_wr wr, *bad_wr_ = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr) );

  wr.wr_id = (uintptr_t)id_;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;

  sge.addr = (uintptr_t)ctx_->msg_;
  sge.length = sizeof(*ctx_->msg_);
  sge.lkey = ctx_->msg_mr_->lkey;

  return ibv_post_send(id_->qp, &wr, &bad_wr_);
}

int IBServer::post_receive(struct rdma_cm_id* id_)
{
  struct ibv_recv_wr wr, *bad_wr_ = NULL;

  memset(&wr, 0, sizeof(wr) );

  wr.wr_id = (uintptr_t)id_;
  wr.sg_list = NULL;
  wr.num_sge = 0;

  return ibv_post_recv(id_->qp, &wr, &bad_wr_);
}

//----------------------------------------  State handlers  --------------------------------------//
int IBServer::on_pre_conn(struct rdma_cm_id* id_)
{
  int err;
  struct conn_context* ctx_ = (struct conn_context*)malloc(sizeof(struct conn_context) );

  id_->context = ctx_;

  posix_memalign((void **)&ctx_->buffer_, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx_->buffer_mr_ = ibv_reg_mr(connector_->get_pd_(), ctx_->buffer_, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE) );

  posix_memalign((void **)&ctx_->msg_, sysconf(_SC_PAGESIZE), sizeof(*ctx_->msg_) );
  TEST_Z(ctx_->msg_mr_ = ibv_reg_mr(connector_->get_pd_(), ctx_->msg_, sizeof(*ctx_->msg_), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE) );
  
  return_if_err(post_receive(id_), err)
  return 0;
}

int IBServer::on_conn(struct rdma_cm_id* id_)
{
  int err;
  struct conn_context* ctx_ = (struct conn_context*)id_->context;
  
  ctx_->msg_->id = MSG_MR;
  ctx_->msg_->data.mr.addr = (uintptr_t)ctx_->buffer_mr_->addr;
  ctx_->msg_->data.mr.rkey = ctx_->buffer_mr_->rkey;
  
  return_if_err(send_message(id_), err)
  return 0;
}

int IBServer::on_disconn(struct rdma_cm_id* id_)
{
  struct conn_context* ctx_ = (struct conn_context*)id_->context;

  ibv_dereg_mr(ctx_->buffer_mr_);
  ibv_dereg_mr(ctx_->msg_mr_);

  free(ctx_->buffer_); ctx_->buffer_ = NULL;
  free(ctx_->msg_); ctx_->msg_ = NULL;
  free(ctx_); ctx_ = NULL;

  log_(INFO, "done.")
  return 0;
}

int IBServer::on_completion(struct ibv_wc* wc_)
{
  int err;
  struct rdma_cm_id* id_ = (struct rdma_cm_id*)(uintptr_t)wc_->wr_id;
  struct conn_context* ctx_ = (struct conn_context*)id_->context;
  
  if (wc_->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
    uint64_t size = ntohl(wc_->imm_data);
    // print_conn_context(ctx);
    
    if (size == 0) {
      log_(INFO, "recved size= " << size)
      ctx_->msg_->id = MSG_DONE;
      return_if_err(send_message(id_), err)
      return 0;
    }
    else if (ctx_->msg_->id == MSG_DONE) {
      log_(INFO, "recved MSG_DONE.")
      ctx_->msg_->id = MSG_DONE;
      return_if_err(send_message(id_), err)
      return 0;
    }
    
    if (recv_state == DATA_RECV) {
      if (data_t == RDMA_MSG) {
        // TODO: thread
        char* msg_ = (char*)malloc(size*sizeof(char) );
        memcpy(msg_, (char*)ctx_->buffer_, size);
        if (msg_recv_cb)
          msg_recv_cb(size, msg_);
        
        free(msg_); msg_ = NULL;
        recv_state = HEADER_RECV;
      }
      else if (data_t == RDMA_DATA) {
        if (data_size_recved + size > data_size_to_recv) {
          log_(ERROR, "recving more than advertised; data_size_recved + size= " << data_size_recved + size << " while data_size_to_recv= " << data_size_to_recv)
          return 1;
        }
        memcpy(static_cast<char*>(data_to_recv_) + data_size_recved, ctx_->buffer_, size);
        data_size_recved += size;
        
        if (data_size_recved == data_size_to_recv) {
          log_(INFO, "finished recving; data_id= " << data_id_ << ", data_size= " << data_size_recved)
          // TODO: thread
          if (data_recv_cb)
            data_recv_cb(boost::lexical_cast<std::string>(data_id_), data_size_recved, data_to_recv_);
          
          // delete connector_;
          ctx_->msg_->id = MSG_DONE;
          return_if_err(send_message(id_), err)
          // TODO: for now to end ib_server, return 1 will terminate Connector::poll_cq loop
          return 1;
          // connector_->rc_disconnect(id_); // this causes double free corruption error on ib_client
          
          recv_state = HEADER_RECV;
          free(data_id_); data_id_ = NULL;
          // free(data_to_recv_); data_to_recv_ = NULL; // done in data_recv_cb
          free(data_size_); data_size_ = NULL;
        }
      }
    }
    else if (recv_state == HEADER_RECV) {
      if (proc_header((char*)ctx_->buffer_) ) {
        log_(ERROR, "proc_header failed; header_= " << (char*)ctx_->buffer_)
        return 1;
      }
      
      if (data_t == RDMA_MSG || data_t == RDMA_DATA)
        recv_state = DATA_RECV;
      // else if (data_t == RDMA_DONE) { // TODO: to terminate ib_server
      //   log_(INFO, "recved RDMA_DONE.")
      //   return 1;
      // }
    }
    else {
      log_(ERROR, "recv_state_err; data_t= " << (char)data_t << ", recv_state= " << (char)recv_state << ", recved size= " << size)
      return 1;
    }
    
    TEST_NZ(post_receive(id_) )
    
    ctx_->msg_->id = MSG_READY_TO_RECV;
    return_if_err(send_message(id_), err)
    return 0;
  }
}

int IBServer::proc_header(char* header_)
{
  log_(INFO, "header_= " << header_)
  // Decode data_t
  data_t = (RDMA_DATA_T)header_[0];
  header_ += IB_MAX_DATA_T_LENGTH;
  // Decode data_t
  if (data_t == RDMA_DATA) {
    data_id_ = (char*)malloc(IB_MAX_DATA_ID_LENGTH*sizeof(char) );
    memcpy(data_id_, header_, IB_MAX_DATA_ID_LENGTH);
    *(strchr(data_id_, HEADER_DELIMITER) ) = '\0';
    header_ += IB_MAX_DATA_ID_LENGTH;
  }
  if (data_t == RDMA_MSG || data_t == RDMA_DATA) {
    char* data_size_ = (char*)malloc(IB_MAX_DATA_SIZE_LENGTH*sizeof(char) );
    memcpy(data_size_, header_, IB_MAX_DATA_SIZE_LENGTH);
    *(strchr(data_size_, HEADER_DELIMITER) ) = '\0';
    
    data_size_recved = 0;
    data_size_to_recv = atoi(data_size_);
    free(data_size_); data_size_ = NULL;
    data_to_recv_ = malloc(data_size_to_recv);
  }
  
  return 0;
}
