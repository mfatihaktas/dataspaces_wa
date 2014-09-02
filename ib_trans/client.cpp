#include "client.h"

IBClient::IBClient(const char* s_laddr, const char* s_lport, 
                   size_t data_size, char* data_)
: s_laddr(s_laddr),
  s_lport(s_lport),
  data_size(data_size),
  data_(data_),
  num_srs(0)
{
  //
  LOG(INFO) << "IBClient:: constructed:\n" << to_str();
}

IBClient::~IBClient()
{
  //
  LOG(INFO) << "IBClient:: destructed.";
}

void IBClient::write_remote(struct rdma_cm_id *id, uint32_t len)
{
  struct client_context *ctx = (struct client_context *)id->context;

  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr));

  if (len) {
    sge.addr = (uintptr_t)ctx->buffer;
    sge.length = len;
    sge.lkey = ctx->buffer_mr->lkey;
    
    wr.sg_list = &sge;
    wr.num_sge = 1;
  }
  
  wr.wr_id = (uintptr_t)id;
  wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.imm_data = htonl(len);
  wr.wr.rdma.remote_addr = ctx->peer_addr;
  wr.wr.rdma.rkey = ctx->peer_rkey;

  TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
  num_srs += 1;
  LOG(INFO) << "write_remote:: len= " << len << " num_srs= " << num_srs;
}

void IBClient::post_receive(struct rdma_cm_id *id)
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
}

void IBClient::send_data(struct rdma_cm_id *id)
{
  LOG(INFO) << "send_data:: started.";
  
  char* data_t_ = data_;
  size_t data_size_t = data_size;
  
  size_t chunk_size;
  char* chunk_ = (char*)malloc(sizeof(char)*BUFFER_SIZE);
  while(data_size_t){
    switch (data_size_t >= BUFFER_SIZE){
      case 1:
        chunk_size = BUFFER_SIZE;
        break;
      case 0:
        chunk_size = data_size_t;
    }
    memcpy(chunk_, data_t_, chunk_size);
    send_chunk(id, chunk_size, chunk_);
    
    data_size_t -= chunk_size;
    data_t_ += chunk_size;
  }
  free(chunk_);
  send_chunk(id, 3, (char*)"EOF");
  //
  LOG(INFO) << "send_data:: done.";
}

void IBClient::send_chunk(struct rdma_cm_id *id, size_t chunk_size, char* chunk)
{
  struct client_context *ctx = (struct client_context *)id->context;

  //ssize_t size = read(ctx->fd, ctx->buffer, BUFFER_SIZE);
  memcpy(ctx->buffer, chunk, chunk_size);

  write_remote(id, chunk_size);
}

/* STATE HANDLERS */
void IBClient::on_pre_conn(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;

  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(connector.rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(connector.rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));

  post_receive(id);
}

void IBClient::on_completion(struct ibv_wc *wc)
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
      connector.rc_disconnect(id);
      return;
    }

    post_receive(id);
  }
}

void IBClient::init()
{
  struct client_context ctx;
  
  connector.rc_init(
    boost::bind(&IBClient::on_pre_conn, this, _1),
    NULL, // on connect
    boost::bind(&IBClient::on_completion, this, _1),
    NULL); // on disconnect

  connector.rc_client_loop(s_laddr, s_lport, &ctx);
}

std::string IBClient::to_str()
{
  std::stringstream ss;

  ss << "s_laddr= " << s_laddr << "\n";
  ss << "s_lport= " << s_lport << "\n";
  ss << "data_size= " << boost::lexical_cast<std::string>(data_size) << "\n";
  ss << "\n";
  
  return ss.str();
}
