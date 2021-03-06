#ifndef _COMMON_H_
#define _COMMON_H_

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

#include "patch_ib.h"
// 
#ifndef _TEST_MACROS_
#define _TEST_MACROS_
#define TEST_NZ(x) do { int r=x; if ((r)){ printf("r= %d\n", r); rc_die("error: " #x " failed (returned non-zero)." );} } while (0)
#define TEST_Z(x)  do { if (!(x)) rc_die("error: " #x " failed (returned zero)."); } while (0)
#endif //   _TEST_MACROS_

void rc_die(const char *reason);

const char HEADER_DELIMITER = '#';

const int IB_MAX_CHUNK_ID_LENGTH = 20;
const int IB_CHUNK_LENGTH = 10*1024*1024;

const size_t MAX_QP__CQ_LENGTH = 100;

struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;
};

typedef boost::function<void(struct rdma_cm_id *id)> pre_conn_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> connect_cb_fn;
typedef boost::function<void(struct ibv_wc *wc)> completion_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> disconnect_cb_fn;

class Connector {
  private:
    struct context *s_ctx;
    pre_conn_cb_fn s_on_pre_conn_cb;
    connect_cb_fn s_on_connect_cb;
    completion_cb_fn s_on_completion_cb;
    disconnect_cb_fn s_on_disconnect_cb;
    
    std::vector<boost::thread*> t_v;
  public:
    Connector(pre_conn_cb_fn pc, connect_cb_fn conn, completion_cb_fn comp, disconnect_cb_fn disc);
    ~Connector();
    
    void build_params(struct rdma_conn_param *params);
    void build_connection(struct rdma_cm_id *id);
    
    void build_context(struct ibv_context *verbs);
    void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
    int event_loop(struct rdma_event_channel *ec, int exit_on_disconnect);
    void* poll_cq(void*);
    
    void rc_init(pre_conn_cb_fn, connect_cb_fn, completion_cb_fn, disconnect_cb_fn);
    int rc_client_loop(const char *host, const char *port, void *context);
    void rc_disconnect(struct rdma_cm_id *id);
    struct ibv_pd * rc_get_pd();
    int rc_server_loop(const char *port);
};

#endif // _COMMON_H_