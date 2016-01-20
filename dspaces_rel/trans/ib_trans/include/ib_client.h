#ifndef _IB_CLIENT_H_
#define _IB_CLIENT_H_

#include <fcntl.h>
#include <libgen.h>
#include <deque>
#include <utility>
#include <iostream>

#include "ib_conn.h"
#include "ib_msg.h"

const char WAIT_INIT = 'I';
const char WAIT_SEND_DATA = 'S';

typedef std::pair<int, void*> size_data_pair;

class IBClient : public IBEnd {
  struct conn_context {
    void* buffer_;
    struct ibv_mr* buffer_mr_;
  
    struct message* msg_;
    struct ibv_mr* msg_mr_;
  
    uint64_t peer_addr;
    uint32_t peer_rkey;
  };
  private:
    const char *s_lip_, *s_lport_;
    
    std::string client_name;
    patch_ib::syncer<char> syncer;
    Connector* connector_;
    
    struct rdma_cm_id* id_;
    patch_ib::BQueue<size_data_pair> size_data_bq;
    
    // std::vector<pthread_t*> pthread_v;
  public:
    IBClient(const char* s_lip_, const char* s_lport_);
    ~IBClient();
    std::string to_str();
    
    void init();
    
    int send_init();
    int send_msg(std::string msg);
    int send_data(std::string data_id, uint64_t data_size, void* data_);
    int make_header(RDMA_DATA_T data_t, std::string data_id, uint64_t data_size,
                    int& header_size, char*& arg_header_);
    int send_next();
    int send_chunk(uint64_t chunk_size, void* chunk_); // chunk_size in bytes
    int write_remote(struct rdma_cm_id* id_, uint64_t size);
    int post_receive(struct rdma_cm_id* id_);
    // State handlers
    int on_pre_conn(struct rdma_cm_id* id_);
    int on_conn(struct rdma_cm_id* id_) {}
    int on_completion(struct ibv_wc* wc_);
    int on_disconn(struct rdma_cm_id* id_) {}
};

struct wrap_IBClient {
  IBClient* ib_client_;
  
  wrap_IBClient(IBClient* ib_client_)
  : ib_client_(ib_client_)
  {}
};

#endif // _IB_CLIENT_H_