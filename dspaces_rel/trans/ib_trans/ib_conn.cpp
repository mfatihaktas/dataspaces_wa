#include "ib_conn.h"

extern "C" void* call_poll_cq_w_wrap(void* wrap_)
{
  wrap_Connector* w_(static_cast<wrap_Connector*>(wrap_) );
  w_->connector_->poll_cq(w_->s_ctx_);
  
  delete w_;
  return 0;
}

Connector::Connector(IBEnd* ib_end_)
: ib_end_(ib_end_)
{
  s_ctx_ = NULL;
  // 
  log_(INFO, "constructed.")
}

Connector::~Connector() { log_(INFO, "destructed.") }

struct ibv_pd* Connector::get_pd_() { return s_ctx_->pd_; }

void Connector::build_param(struct rdma_conn_param* param_)
{
  memset(param_, 0, sizeof(*param_) );

  param_->initiator_depth = param_->responder_resources = 1;
  param_->rnr_retry_count = 7; /* infinite retry */
}

void Connector::build_conn(struct rdma_cm_id* id_)
{
  struct ibv_qp_init_attr qp_attr;

  build_context(id_->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id_, s_ctx_->pd_, &qp_attr) );
}

void Connector::build_context(struct ibv_context* verb_)
{
  if (s_ctx_ && s_ctx_->ctx_ != verb_) {
    log_(ERROR, "cannot handle events in more than one context.")
    exit(EXIT_FAILURE);
  }

  s_ctx_ = (struct context*)malloc(sizeof(struct context) );

  s_ctx_->ctx_ = verb_;

  TEST_Z(s_ctx_->pd_ = ibv_alloc_pd(s_ctx_->ctx_) );
  TEST_Z(s_ctx_->comp_channel_ = ibv_create_comp_channel(s_ctx_->ctx_) );
  TEST_Z(s_ctx_->cq_ = ibv_create_cq(s_ctx_->ctx_, MAX_QP__CQ_LENGTH, NULL, s_ctx_->comp_channel_, 0) );
  TEST_NZ(ibv_req_notify_cq(s_ctx_->cq_, 0) );
  // TODO
  // TEST_NZ(pthread_create(pthread_v.back(), NULL, &Connector::bst_poll_cq, (void*)(this) ) );
  pthread_v.push_back(new pthread_t() );
  wrap_Connector* wrap_ = new wrap_Connector(this, s_ctx_);
  TEST_NZ(pthread_create(pthread_v.back(), NULL, call_poll_cq_w_wrap, wrap_) );
}

void Connector::build_qp_attr(struct ibv_qp_init_attr* qp_attr_)
{
  memset(qp_attr_, 0, sizeof(*qp_attr_) );

  qp_attr_->send_cq = s_ctx_->cq_;
  qp_attr_->recv_cq = s_ctx_->cq_;
  qp_attr_->qp_type = IBV_QPT_RC;
  
  qp_attr_->cap.max_send_wr = MAX_QP__CQ_LENGTH;
  qp_attr_->cap.max_recv_wr = MAX_QP__CQ_LENGTH;
  qp_attr_->cap.max_send_sge = 1;
  qp_attr_->cap.max_recv_sge = 1;
}

void Connector::event_loop(struct rdma_event_channel* ec_, bool exit_on_disconn)
{
  struct rdma_cm_event* event_ = NULL;
  struct rdma_conn_param cm_param;

  build_param(&cm_param);
  while (rdma_get_cm_event(ec_, &event_) == 0) {
    struct rdma_cm_event event_copy;
    
    memcpy(&event_copy, event_, sizeof(*event_) );
    rdma_ack_cm_event(event_);
    if (event_copy.event == RDMA_CM_EVENT_ADDR_RESOLVED) {
      build_conn(event_copy.id);
      
      ib_end_->on_pre_conn(event_copy.id);
      
      TEST_NZ(rdma_resolve_route(event_copy.id, TIMEOUT_IN_MS) );
    } 
    else if (event_copy.event == RDMA_CM_EVENT_ROUTE_RESOLVED) {
      TEST_NZ(rdma_connect(event_copy.id, &cm_param) );
    }
    else if (event_copy.event == RDMA_CM_EVENT_CONNECT_REQUEST) {
      build_conn(event_copy.id);
      
      ib_end_->on_pre_conn(event_copy.id);

      TEST_NZ(rdma_accept(event_copy.id, &cm_param) );
    } 
    else if (event_copy.event == RDMA_CM_EVENT_ESTABLISHED) {
      ib_end_->on_conn(event_copy.id);
    } 
    else if (event_copy.event == RDMA_CM_EVENT_DISCONNECTED) {
      rdma_destroy_qp(event_copy.id);
      
      ib_end_->on_disconn(event_copy.id);
      
      rdma_destroy_id(event_copy.id);
      if (exit_on_disconn)
        break;
    } 
    else {
      log_(ERROR, "Unknown event= " << event_copy.event)
      exit(EXIT_FAILURE);
    }
  }
}

void* Connector::poll_cq(void* ctx_)
{
  struct ibv_cq* cq_;
  struct ibv_wc wc;

  while (1) {
    // log_(INFO, "before ibv_get_cq_event.")
    TEST_NZ(ibv_get_cq_event(s_ctx_->comp_channel_, &cq_, &ctx_) );
    ibv_ack_cq_events(cq_, 1);
    TEST_NZ(ibv_req_notify_cq(cq_, 0) );
    // log_(INFO, "before ibv_poll_cq.")
    while (ibv_poll_cq(cq_, 1, &wc) ) {
      if (wc.status == IBV_WC_SUCCESS) {
        // log_(INFO, "calling completion_cb...")
        try {
          ib_end_->on_completion(&wc);
        }
        catch(const std::exception& ex) {
          log_(ERROR, "FATAL exception ex= " << ex.what() )
          exit(EXIT_FAILURE);
        }
      }
      else {
        log_(ERROR, "status is not IBV_WC_SUCCESS")
        exit(EXIT_FAILURE);
      }
    }
  }

  return NULL;
}

void Connector::rc_client_loop(const char* host_, const char* port_, void* ctx_)
{
  struct addrinfo* addr_;
  struct rdma_cm_id* conn_ = NULL;
  struct rdma_event_channel* ec_ = NULL;
  struct rdma_conn_param cm_param;

  TEST_NZ(getaddrinfo(host_, port_, NULL, &addr_) );

  TEST_Z(ec_ = rdma_create_event_channel() );
  TEST_NZ(rdma_create_id(ec_, &conn_, NULL, RDMA_PS_TCP) );
  TEST_NZ(rdma_resolve_addr(conn_, NULL, addr_->ai_addr, TIMEOUT_IN_MS) );

  freeaddrinfo(addr_);

  conn_->context = ctx_;

  build_param(&cm_param);

  event_loop(ec_, true); // exit on disconnect

  rdma_destroy_event_channel(ec_);
}

void Connector::rc_server_loop(const char* port_)
{
  struct sockaddr_in addr;
  struct rdma_cm_id* listener_ = NULL;
  struct rdma_event_channel* ec_ = NULL;

  memset(&addr, 0, sizeof(addr) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port_) );

  TEST_Z(ec_ = rdma_create_event_channel() );
  TEST_NZ(rdma_create_id(ec_, &listener_, NULL, RDMA_PS_TCP) );
  TEST_NZ(rdma_bind_addr(listener_, (struct sockaddr *)&addr) );
  TEST_NZ(rdma_listen(listener_, 10) ); /* backlog=10 is arbitrary */

  event_loop(ec_, false); // don't exit on disconnect

  rdma_destroy_id(listener_);
  rdma_destroy_event_channel(ec_);
}

void Connector::rc_disconnect(struct rdma_cm_id* id_) { rdma_disconnect(id_); }
