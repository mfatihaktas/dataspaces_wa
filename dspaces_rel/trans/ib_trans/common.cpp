#include "common.h"

const int TIMEOUT_IN_MS = 500;

void rc_die(const char *reason)
{
  fprintf(stderr, "%s\n", reason);
  exit(EXIT_FAILURE);
}

Connector::Connector(pre_conn_cb_fn pc, connect_cb_fn conn, completion_cb_fn comp, disconnect_cb_fn disc)
{
  rc_init(pc, conn, comp, disc);
  s_ctx = NULL;
  // 
  LOG(INFO) << "Connector:: constructed.";
}

Connector::~Connector() { LOG(INFO) << "Connector:: destructed."; }

void Connector::build_connection(struct rdma_cm_id *id)
{
  struct ibv_qp_init_attr qp_attr;

  build_context(id->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr) );
}

void Connector::build_context(struct ibv_context *verbs)
{
  if (s_ctx) {
    if (s_ctx->ctx != verbs)
      rc_die("cannot handle events in more than one context.");

    return;
  }

  s_ctx = (struct context *)malloc(sizeof(struct context) );

  s_ctx->ctx = verbs;

  TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx) );
  TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx) );
  TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, MAX_QP__CQ_LENGTH, NULL, s_ctx->comp_channel, 0) );
  TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0) );

  boost::thread t(boost::bind(&Connector::poll_cq, this, s_ctx) );
}

void Connector::build_params(struct rdma_conn_param *params)
{
  memset(params, 0, sizeof(*params) );

  params->initiator_depth = params->responder_resources = 1;
  params->rnr_retry_count = 7; /* infinite retry */
}

void Connector::build_qp_attr(struct ibv_qp_init_attr *qp_attr)
{
  memset(qp_attr, 0, sizeof(*qp_attr) );

  qp_attr->send_cq = s_ctx->cq;
  qp_attr->recv_cq = s_ctx->cq;
  qp_attr->qp_type = IBV_QPT_RC;

  qp_attr->cap.max_send_wr = MAX_QP__CQ_LENGTH;
  qp_attr->cap.max_recv_wr = MAX_QP__CQ_LENGTH;
  qp_attr->cap.max_send_sge = 1;
  qp_attr->cap.max_recv_sge = 1;
}

void Connector::event_loop(struct rdma_event_channel *ec, int exit_on_disconnect)
{
  struct rdma_cm_event *event = NULL;
  struct rdma_conn_param cm_params;

  build_params(&cm_params);

  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event) );
    rdma_ack_cm_event(event);

    if (event_copy.event == RDMA_CM_EVENT_ADDR_RESOLVED) {
      build_connection(event_copy.id);

      if (s_on_pre_conn_cb)
        s_on_pre_conn_cb(event_copy.id);

      TEST_NZ(rdma_resolve_route(event_copy.id, TIMEOUT_IN_MS) );

    } 
    else if (event_copy.event == RDMA_CM_EVENT_ROUTE_RESOLVED) {
      TEST_NZ(rdma_connect(event_copy.id, &cm_params) );
    } 
    else if (event_copy.event == RDMA_CM_EVENT_CONNECT_REQUEST) {
      build_connection(event_copy.id);

      if (s_on_pre_conn_cb)
        s_on_pre_conn_cb(event_copy.id);

      TEST_NZ(rdma_accept(event_copy.id, &cm_params) );
    } 
    else if (event_copy.event == RDMA_CM_EVENT_ESTABLISHED) {
      if (s_on_connect_cb)
        s_on_connect_cb(event_copy.id);
    } 
    else if (event_copy.event == RDMA_CM_EVENT_DISCONNECTED) {
      rdma_destroy_qp(event_copy.id);

      if (s_on_disconnect_cb)
        s_on_disconnect_cb(event_copy.id);

      rdma_destroy_id(event_copy.id);

      if (exit_on_disconnect)
        break;
    } 
    else {
      rc_die("event_loop:: Unknown event \n");
    }
  }
}

void* Connector::poll_cq(void *ctx)
{
  // struct ibv_cq *cq = ((struct context*)ctx)->cq;
  struct ibv_cq *cq;
  struct ibv_wc wc;

  while (1) {
    // LOG(INFO) << "poll_cq:: before ibv_get_cq_event.";
    TEST_NZ(ibv_get_cq_event(s_ctx->comp_channel, &cq, &ctx) );
    ibv_ack_cq_events(cq, 1);
    TEST_NZ(ibv_req_notify_cq(cq, 0) );
    // LOG(INFO) << "poll_cq:: before ibv_poll_cq.";
    while (ibv_poll_cq(cq, 1, &wc) ) {
      if (wc.status == IBV_WC_SUCCESS) {
        // LOG(INFO) << "poll_cq:: calling s_on_completion_cb...";
        try {
          s_on_completion_cb(&wc);
        }
        catch(boost::bad_function_call) {
          LOG(ERROR) << "poll_cq:: how can s_on_completion_cb be empty!!!";
          rc_die("poll_cq:: status is not IBV_WC_SUCCESS");
        }
      }
      else {
        rc_die("poll_cq:: status is not IBV_WC_SUCCESS");
      }
    }
  }

  return NULL;
}

void Connector::rc_init(pre_conn_cb_fn pc, connect_cb_fn conn, completion_cb_fn comp, disconnect_cb_fn disc)
{
  s_on_pre_conn_cb = pc;
  s_on_connect_cb = conn;
  s_on_completion_cb = comp;
  s_on_disconnect_cb = disc;
}

void Connector::rc_client_loop(const char *host, const char *port, void *context)
{
  struct addrinfo *addr;
  struct rdma_cm_id *conn = NULL;
  struct rdma_event_channel *ec = NULL;
  struct rdma_conn_param cm_params;

  TEST_NZ(getaddrinfo(host, port, NULL, &addr) );

  TEST_Z(ec = rdma_create_event_channel() );
  TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP) );
  TEST_NZ(rdma_resolve_addr(conn, NULL, addr->ai_addr, TIMEOUT_IN_MS) );

  freeaddrinfo(addr);

  conn->context = context;

  build_params(&cm_params);

  event_loop(ec, 1); // exit on disconnect

  rdma_destroy_event_channel(ec);
}

void Connector::rc_server_loop(const char *port)
{
  struct sockaddr_in addr;
  struct rdma_cm_id *listener = NULL;
  struct rdma_event_channel *ec = NULL;

  memset(&addr, 0, sizeof(addr) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port) );

  TEST_Z(ec = rdma_create_event_channel() );
  TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP) );
  TEST_NZ(rdma_bind_addr(listener, (struct sockaddr *)&addr) );
  TEST_NZ(rdma_listen(listener, 10) ); /* backlog=10 is arbitrary */

  event_loop(ec, 1); // don't exit on disconnect

  rdma_destroy_id(listener);
  rdma_destroy_event_channel(ec);
}

void Connector::rc_disconnect(struct rdma_cm_id *id)
{
  rdma_disconnect(id);
}

struct ibv_pd* Connector::rc_get_pd()
{
  return s_ctx->pd;
}