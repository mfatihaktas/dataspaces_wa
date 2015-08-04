#ifndef _SDM_CONTROL_H_
#define _SDM_CONTROL_H_

#include "sdm_node.h"
#include "sdm.h"
#include "sfc.h"

/*****************************  SDMSlave, SDMMaster : SDMCEntity  *********************************/
class SDMCEntity { // SDM Control
  protected:
    SDMNode sdm_node;
    MsgCoder msg_coder;
  public:
    SDMCEntity(char id, std::string type,
               std::string lip, int lport,
               std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb _rimsg_recv_cb, func_sdm_cp_recv_cb _sdm_cp_recv_cb)
    : 
    (id, type, lip, lport, joinhost_lip, joinhost_lport,
               _rimsg_recv_cb, _sdm_cp_recv_cb)
    {}
    
    int broadcast_msg(std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg(SDM_CMSG, msg_map); }
    int broadcast_msg_to_slaves(std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg_to_slaves(SDM_CMSG, msg_map); }
    int send_msg(char to_id, std::map<std::string, std::string> msg_map) {return sdm_node.send_msg(to_id, SDM_CMSG, msg_map); }
    
    virtual void handle_recv(boost::shared_ptr<Packet> p_) = 0;
};

class SDMMaster : public SDMCEntity {
  public:
    SDMMaster(char id, std::string type,
              std::string lip, int lport,
              std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb _rimsg_recv_cb)
    : SDMCEntity(id, type, lip, lport, joinhost_lip, joinhost_lport,
                 _rimsg_recv_cb, boost::bind(&SDMMaster::handle_recv, this, _1) )
    {}
    
    void handle_recv(boost::shared_ptr<Packet> p_)
    {
      switch (p_->get_type() ) {
        case SDM_JOIN_ACK:
          handle_conn_up(p_->get_msg_map() );
          break;
        case SDM_CMSG:
          handle_msg_in(p_->get_msg_map() );
          break;
        default:
          LOG(WARNING) << "handle_recv:: unexpected p.get_type= " << p_->get_type();
          break;
      }
    }
    
    virtual void handle_conn_up(std::map<std::string, std::string> msg_map) = 0;
    virtual void handle_msg_in(std::map<std::string, std::string> msg_map) = 0;
};

class SDMSlave : public SDMCEntity {
  public:
    SDMSlave(char id, std::string type,
             std::string lip, int lport,
             std::string joinhost_lip, int joinhost_lport,
             func_rimsg_recv_cb _rimsg_recv_cb)
    : SDMCEntity(id, type, lip, lport, joinhost_lip, joinhost_lport,
                 _rimsg_recv_cb, boost::bind(&SDMSlave::handle_recv, this, _1) )
    {}
    
    int send_msg_to_master(std::map<std::string, std::string> msg_map) { return sdm_node.send_msg_to_master(msg_map); }
    
    void handle_recv(boost::shared_ptr<Packet> p_)
    {
      switch (p_->get_type() ) {
        case SDM_CMSG:
          handle_msg_in(p_->get_msg_map() );
          break;
        default:
          LOG(WARNING) << "handle_recv:: unexpected p.get_type= " << p_->get_type();
          break;
      }
    }
    
    virtual void handle_msg_in(std::map<std::string, std::string> msg_map) = 0;
};

/******************************************  HSDMMaster  ******************************************/
class HSDMMaster : public SDMMaster { // Hilbert
  private:
    WASpace wa_space;
    int num_slaves;
    patch_sdm::syncer<std::string> sdm_mquery_syncer;
    patch_sdm::syncer<std::string> sdm_move_syncer;
  public:
    HSDMMaster(char id, std::string type,
               std::string lip, int lport,
               std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb _rimsg_recv_cb);
    ~HSDMMaster();
    
    int sdm_mquery(COOR_T* lcoor_, COOR_T* ucoor_);
    
    void handle_conn_up(std::map<std::string, std::string> msg_map);
    void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_put_notification(std::map<std::string, std::string> msg_map);
};

/******************************************  HSDMSlave  *******************************************/
const std::string SDM_MQUERY = "sdm_mq";
const std::string SDM_MQUERY_REPLY = "sdm_mqr";
const std::string SDM_SQUERY = "sdm_sq";
const std::string SDM_SQUERY_REPLY = "sdm_sqr";
const std::string SDM_PUT_NOTIFICATION = "sdm_pn";

const std::string SDM_MOVE = "sdm_m";
const std::string SDM_MOVE_REPLY = "sdm_mr";

typedef boost::function<void(std::map<std::string, std::string> ) > function_cb_on_dm_act;
class HSDMSlave : SDMSlave { // Hilbert
  private:
    function_cb_on_dm_act cb_on_dm_act;
    
    RTable<int> rtable;
    patch_sdm::syncer<std::string> sdm_squery_syncer;
  public:
    HSDMSlave(char id, std::string type,
              std::string lip, int lport,
              std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb _rimsg_recv_cb,
              function_cb_on_dm_act cb_on_dm_act);
    ~HSDMSlave();
    
    int put(COOR_T* lcoor_, COOR_T* ucoor_, int p_id);
    int get(COOR_T* lcoor_, COOR_T* ucoor_);
    
    void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery(std::map<std::string, std::string> msg_map);
    void handle_sdm_squery_reply(std::map<std::string, std::string> msg_map);
};

#endif //_SDM_CONTROL_H_
