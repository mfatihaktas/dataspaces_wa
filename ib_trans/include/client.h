#include <fcntl.h>
#include <libgen.h>

#include "common.h"
#include "messages.h"

struct client_context
{
  char *buffer;
  struct ibv_mr *buffer_mr;

  struct message *msg;
  struct ibv_mr *msg_mr;

  uint64_t peer_addr;
  uint32_t peer_rkey;
};

class IBClient{
  public:
    IBClient(const char* s_laddr, const char* s_lport, 
             size_t data_size, char* data_);
    ~IBClient();
    void write_remote(struct rdma_cm_id *id, uint32_t len);
    void post_receive(struct rdma_cm_id *id);
    void send_data(struct rdma_cm_id *id);
    void send_chunk(struct rdma_cm_id *id, size_t chunk_size, char* chunk);
    void init();
    //state handlers
    void on_pre_conn(struct rdma_cm_id *id);
    void on_completion(struct ibv_wc *wc);
    
    std::string to_str();
  private:
    const char *s_laddr, *s_lport;
    size_t data_size;
    char* data_;
    
    int num_srs;
    //
    Connector connector;
};