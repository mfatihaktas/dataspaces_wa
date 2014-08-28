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

#include <glog/logging.h>
//
#define TEST_NZ(x) do { if ( (x)) rc_die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) rc_die("error: " #x " failed (returned zero/null)."); } while (0)

typedef boost::function<void(struct rdma_cm_id *id)> pre_conn_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> connect_cb_fn;
typedef boost::function<void(struct ibv_wc *wc)> completion_cb_fn;
typedef boost::function<void(struct rdma_cm_id *id)> disconnect_cb_fn;

void rc_init(pre_conn_cb_fn, connect_cb_fn, completion_cb_fn, disconnect_cb_fn);
void rc_client_loop(const char *host, const char *port, void *context);
void rc_disconnect(struct rdma_cm_id *id);
void rc_die(const char *message);
struct ibv_pd * rc_get_pd();
void rc_server_loop(const char *port);

#endif