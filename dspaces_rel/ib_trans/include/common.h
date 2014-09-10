#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>
//
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>
//
#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) do { int r=x; if ((r)){ printf("r= %d\n", r); rc_die("error: " #x " failed (returned non-zero)." );} } while (0)
#define TEST_Z(x)  do { if (!(x)) rc_die("error: " #x " failed (returned zero)."); } while (0)
#endif

void rc_die(const char *reason);

const size_t BUFFER_LENGTH = 10 * 1024 * 1024;
const size_t MAX_QP__CQ_LENGTH = 100;

struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

typedef boost::function<void(struct rdma_cm_id *id)> pre_conn_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> connect_cb_fn;
typedef boost::function<void(struct ibv_wc *wc)> completion_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> disconnect_cb_fn;

class Connector{
  public:
    Connector();
    ~Connector();
    
    void build_params(struct rdma_conn_param *params);
    void build_connection(struct rdma_cm_id *id);
    
    void build_context(struct ibv_context *verbs);
    void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
    void event_loop(struct rdma_event_channel *ec, int exit_on_disconnect);
    void* poll_cq(void*);
    
    static void* poll_cq_helper(void *context);
    
    void rc_init(pre_conn_cb_fn, connect_cb_fn, completion_cb_fn, disconnect_cb_fn);
    void rc_client_loop(const char *host, const char *port, void *context);
    void rc_disconnect(struct rdma_cm_id *id);
    struct ibv_pd * rc_get_pd();
    void rc_server_loop(const char *port);
  private:
    static struct context *s_ctx;
    pre_conn_cb_fn s_on_pre_conn_cb;
    connect_cb_fn s_on_connect_cb;
    completion_cb_fn s_on_completion_cb;
    disconnect_cb_fn s_on_disconnect_cb;
};

#endif