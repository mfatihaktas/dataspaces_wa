#include "messages.h"
#include "common.h"

struct conn_context
{
  char *buffer;
  struct ibv_mr *buffer_mr;

  struct message *msg;
  struct ibv_mr *msg_mr;
};

typedef boost::function<void(size_t, char*)> data_recv_cb;

class IBServer{
  public:
    IBServer(const char* lport, data_recv_cb dr_cb);
    ~IBServer();
    
    void post_receive(struct rdma_cm_id *id);
    void send_message(struct rdma_cm_id *id);
    //state handlers
    void on_pre_conn(struct rdma_cm_id *id);
    void on_connection(struct rdma_cm_id *id);
    void on_disconnect(struct rdma_cm_id *id);
    void on_completion(struct ibv_wc *wc);
    
    void init();
    
  private:
    const char* lport;
    data_recv_cb dr_cb;
    //
    Connector connector;
};