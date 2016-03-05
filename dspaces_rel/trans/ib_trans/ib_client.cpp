#include "ib_client.h"

extern "C" void* call_init_w_wrap(void* wrap_)
{
  wrap_IBClient* w_(static_cast<wrap_IBClient*>(wrap_) );
  w_->ib_client_->init();
  
  delete w_;
  return 0;
}

extern "C" void* call_send_next_w_wrap(void* wrap_)
{
  wrap_IBClient* w_(static_cast<wrap_IBClient*>(wrap_) );
  TEST_NZ(w_->ib_client_->send_next() );
  
  delete w_;
  return 0;
}

IBClient::IBClient(const char* s_lip_, const char* s_lport_)
: s_lip_(s_lip_), s_lport_(s_lport_),
  client_name(patch::to_str<const char*>(s_lip_) + ":" + patch::to_str<const char*>(s_lport_) ),
  connector_(new Connector(this) )
{
  log_(INFO, "constructed client= \n" << to_str() )
  // 
  pthread_t t;
  wrap_IBClient* wrap_ = new wrap_IBClient(this);
  TEST_NZ(pthread_create(&t, NULL, call_init_w_wrap, wrap_) );
  
  log_(INFO, "client_name= " << client_name << " waiting for init...")
  syncer.add_sync_point(WAIT_INIT, 1);
  syncer.wait(WAIT_INIT);
  syncer.del_sync_point(WAIT_INIT);
  log_(INFO, "client_name= " << client_name << " done waiting for init.")
  
  // To initiate with server so server can start sending MSG_READY_TO_RECV
  pthread_t t2;
  wrap_IBClient* wrap_2_ = new wrap_IBClient(this);
  TEST_NZ(pthread_create(&t2, NULL, call_send_next_w_wrap, wrap_2_) );
  
  TEST_NZ(send_init() );
}

IBClient::~IBClient()
{
  syncer.close();
  delete connector_;
  // delete wrap_;
  // 
  log_(INFO, "destructed.")
}

std::string IBClient::to_str()
{
  std::stringstream ss;
  ss << "s_lip_= " << s_lip_ << "\n"
     << "s_lport_= " << s_lport_ << "\n"
     << "client_name= " << client_name << "\n";
  
  return ss.str();
}

void IBClient::init()
{
  struct conn_context ctx;
  connector_->rc_client_loop(s_lip_, s_lport_, &ctx);
  syncer.notify(WAIT_SEND_DATA);
}

int IBClient::send_init()
{
  int err;
  int header_size;
  char* header_;
  return_if_err(make_header(RDMA_INIT, "", 0, header_size, header_), err)
  size_data_bq.push(std::make_pair(header_size, (void*)header_) );
  
  return 0;
}

int IBClient::send_msg(std::string msg)
{
  int err;
  int header_size;
  char* header_;
  return_if_err(make_header(RDMA_MSG, "", msg.size(), header_size, header_), err)
  size_data_bq.push(std::make_pair(header_size, (void*)header_) );
  
  int msg_length = msg.size() + 1;
  
  char* msg_ = (char*)malloc(msg_length*sizeof(char) );
  memcpy(msg_, msg.c_str(), msg.size() );
  msg_[msg.size() ] = '\0';
  size_data_bq.push(std::make_pair(msg_length, msg_) );
  
  return 0;
}

int IBClient::send_data(std::string data_id, uint64_t data_size, void* data_)
{
  log_(INFO, "started; data_size= " << data_size)
  int err;
  int header_size;
  char* header_;
  return_if_err(make_header(RDMA_DATA, data_id, data_size, header_size, header_), err)
  size_data_bq.push(std::make_pair(header_size, (void*)header_) );
  
  void* data_t_ = data_;
  uint64_t data_size_t = data_size;
  
  uint64_t chunk_size;
  // void* chunk_ = malloc(BUFFER_SIZE);
  while (data_size_t) {
    chunk_size = (data_size_t > BUFFER_SIZE) ? BUFFER_SIZE : data_size_t;
    // memcpy(chunk_, data_t_, chunk_size);
    size_data_bq.push(std::make_pair(chunk_size, data_t_) );
    
    data_size_t -= chunk_size;
    data_t_ = static_cast<char*>(data_t_) + chunk_size;
  }
  // free(data_);
  
  char* end_indicator_ = (char*)malloc(4*sizeof(char) );
  memcpy(end_indicator_, (char*)"EOF", 4);
  end_indicator_[3] = '\0';
  size_data_bq.push(std::make_pair(4, end_indicator_) );
  
  syncer.add_sync_point(WAIT_SEND_DATA, 1);
  syncer.wait(WAIT_SEND_DATA);
  syncer.del_sync_point(WAIT_SEND_DATA);
  log_(INFO, "done; data_size= " << data_size)
  
  // TODO: to make ib_server to go ahead and destruct after finished recving
  // struct conn_context* ctx_ = (struct conn_context*)id_->context;
  // ctx_->msg_->id = MSG_DONE;
  // return_if_err(write_remote(id_, 0), err)
  // sleep(1);
  // connector_->rc_disconnect(id_);
  
  return 0;
}

int IBClient::make_header(RDMA_DATA_T data_t, std::string data_id, uint64_t data_size,
                          int& header_size, char*& arg_header_)
{
  if (data_t == RDMA_INIT || data_t == RDMA_DONE)
    header_size = IB_MAX_DATA_T_LENGTH + 1;
  else if (data_t == RDMA_MSG)
    header_size = IB_MAX_DATA_T_LENGTH + IB_MAX_DATA_SIZE_LENGTH + 1;
  else if (data_t == RDMA_DATA)
    header_size = IB_MAX_DATA_T_LENGTH + IB_MAX_DATA_ID_LENGTH + IB_MAX_DATA_SIZE_LENGTH + 1;
  else {
    log_(ERROR, "unknown data_t= " << (char)data_t)
    return 1;
  }
  // Add data_t
  char* header_ = (char*)malloc(header_size*sizeof(char) );
  arg_header_ = header_;
  *header_ = (char)data_t;
  header_ += IB_MAX_DATA_T_LENGTH;
  // Add data_id
  if (data_id.size() > 0) { // Can be <= 0 for RDMA_INIT/MSG
    if (data_id.size() > IB_MAX_DATA_ID_LENGTH) {
      log_(ERROR, "data_id.size= " << data_id.size() << " > IB_MAX_DATA_ID_LENGTH= " << IB_MAX_DATA_ID_LENGTH)
      return 1;
    }
    char* data_id_ = (char*)malloc(IB_MAX_DATA_ID_LENGTH*sizeof(char) );
    memcpy(data_id_, data_id.c_str(), data_id.size() );
    for (int i = data_id.size(); i < IB_MAX_DATA_ID_LENGTH; i++)
      data_id_[i] = HEADER_DELIMITER;
    memcpy(header_, data_id_, IB_MAX_DATA_ID_LENGTH);
    header_ += IB_MAX_DATA_ID_LENGTH;
  }
  // Add data_size
  if (data_size > 0) { // Can be <= 0 for RDMA_INIT
    char* data_size_ = (char*)malloc(IB_MAX_DATA_SIZE_LENGTH*sizeof(char) );
    sprintf(data_size_, "%d", data_size);
    if (data_size != atoi(data_size_) ) { // Double-check
      log_(ERROR, "sprintf failed; data_size= " << data_size << ", data_size_= " << data_size_)
      return 1;
    }
    char* null_char_;
    null_char_ = strchr(data_size_, '\0');
    for (int i = null_char_ - data_size_; i < IB_MAX_DATA_SIZE_LENGTH; i++)
      data_size_[i] = HEADER_DELIMITER;
    memcpy(header_, data_size_, IB_MAX_DATA_SIZE_LENGTH);
    header_ += IB_MAX_DATA_SIZE_LENGTH;
  }
  *header_ = '\0';
  
  return 0;
}

int IBClient::send_next()
{
  int err;
  size_data_pair size_data = size_data_bq.pop();
  log_(INFO, "popped size= " << size_data.first)
  if (size_data.first < 200)
    std::cout << "data_= " << (char*)size_data.second << "\n";
  if (size_data.first == 4 && cstr_cstr_equals("EOF", (const char*)size_data.second) ) {
    // TODO: to make ib_server to go ahead and destruct after finished recving
    // log_(INFO, "sending RDMA_DONE...")
    // int header_size;
    // char* header_;
    // return_if_err(make_header(RDMA_DONE, "", 0, header_size, header_), err)
    // return_if_err(send_chunk(header_size, (void*)header_), err)
    
    // syncer.notify(WAIT_SEND_DATA);
    return 0;
  }
  
  return_if_err(send_chunk(size_data.first, size_data.second), err, free(size_data.second);)
  // return_if_err(send_chunk(size_data.first, size_data.second), err, free(size_data.second);)
  // free(size_data.second); // This causes segmentation fault in memcpy(ctx_->buffer_, chunk_, chunk_size);
  
  return 0;
}

int IBClient::send_chunk(uint64_t chunk_size, void* chunk_)
{
  int err;
  // sleep(2);
  struct conn_context* ctx_ = (struct conn_context*)id_->context;
  
  log_(INFO, "chunk_size= " << chunk_size)
  memcpy(ctx_->buffer_, chunk_, chunk_size);
  
  return_if_err(write_remote(id_, chunk_size), err)
  return 0;
}

int IBClient::write_remote(struct rdma_cm_id* id_, uint64_t size)
{
  int err;
  struct conn_context* ctx_ = (struct conn_context*)id_->context;

  struct ibv_send_wr wr, *bad_wr_ = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr) );

  if (size) {
    sge.addr = (uintptr_t)ctx_->buffer_;
    sge.length = size;
    sge.lkey = ctx_->buffer_mr_->lkey;
    
    wr.sg_list = &sge;
    wr.num_sge = 1;
  }
  
  wr.wr_id = (uintptr_t)id_;
  wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.imm_data = htonl(size);
  wr.wr.rdma.remote_addr = ctx_->peer_addr;
  wr.wr.rdma.rkey = ctx_->peer_rkey;
  
  return_if_err(ibv_post_send(id_->qp, &wr, &bad_wr_), err)
  log_(INFO, "wrote size= " << size)
  
  return 0;
}

int IBClient::post_receive(struct rdma_cm_id* id_)
{
  int err;
  struct conn_context* ctx_ = (struct conn_context*)id_->context;
  
  struct ibv_recv_wr wr, *bad_wr_ = NULL;
  struct ibv_sge sge;
  
  memset(&wr, 0, sizeof(wr) );
  
  wr.wr_id = (uintptr_t)id_;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  
  sge.addr = (uintptr_t)ctx_->msg_;
  sge.length = sizeof(*ctx_->msg_);
  sge.lkey = ctx_->msg_mr_->lkey;
  
  return_if_err(ibv_post_recv(id_->qp, &wr, &bad_wr_), err)
  
  return 0;
}

//----------------------------------------  State handlers  --------------------------------------//
int IBClient::on_pre_conn(struct rdma_cm_id* id_)
{
  int err;
  struct conn_context* ctx_ = (struct conn_context*)id_->context;
  ctx_->buffer_ = malloc(BUFFER_SIZE);
  
  posix_memalign((void**)&ctx_->buffer_, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx_->buffer_mr_ = ibv_reg_mr(connector_->get_pd_(), ctx_->buffer_, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE) );

  posix_memalign((void**)&ctx_->msg_, sysconf(_SC_PAGESIZE), sizeof(*ctx_->msg_) );
  TEST_Z(ctx_->msg_mr_ = ibv_reg_mr(connector_->get_pd_(), ctx_->msg_, sizeof(*ctx_->msg_), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE) );

  return_if_err(post_receive(id_), err)
  
  return 0;
}

int IBClient::on_completion(struct ibv_wc* wc_)
{
  int err;
  struct rdma_cm_id* id_ = (struct rdma_cm_id*)(uintptr_t)(wc_->wr_id);
  struct conn_context* ctx_ = (struct conn_context*)id_->context;

  if (wc_->opcode & IBV_WC_RECV) {
    if (ctx_->msg_->id == MSG_MR) {
      ctx_->peer_addr = ctx_->msg_->data.mr.addr;
      ctx_->peer_rkey = ctx_->msg_->data.mr.rkey;

      this->id_ = id_;
      syncer.notify(WAIT_INIT);
    }
    else if (ctx_->msg_->id == MSG_READY_TO_RECV) {
      log_(INFO, "recved MSG_READY_TO_RECV.")
      
      pthread_t t;
      wrap_IBClient* wrap_ = new wrap_IBClient(this);
      TEST_NZ(pthread_create(&t, NULL, call_send_next_w_wrap, wrap_) );
    }
    else if (ctx_->msg_->id == MSG_DONE) {
      log_(INFO, "received DONE, disconnecting.")
      connector_->rc_disconnect(id_);
      return 1;
    }
    return_if_err(post_receive(id_), err)
    
    return 0;
  }
  // else if (wc_->opcode & IBV_WC_SEND) {
  //   log_(INFO, "IBV_WC_SEND success.")
  //   free(ctx_->buffer_);
  // }
  // else {
  //   log_(ERROR, "unknown wc_->opcode= " << wc_->opcode)
  //   return wc_->opcode;
  // }
  
  return 0;
}
