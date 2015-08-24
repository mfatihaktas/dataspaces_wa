#ifndef _SDM_CONTROL_H_
#define _SDM_CONTROL_H_

#include "sdm_node.h"
#include "sdm.h"

/*****************************  SDMSlave, SDMMaster : SDMCEntity  *********************************/
class SDMCEntity { // SDM Control
  protected:
    SDMNode sdm_node;
    MsgCoder msg_coder;
  public:
    SDMCEntity(char id, std::string type,
               std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb rimsg_recv_cb, func_sdm_cp_recv_cb sdm_cp_recv_cb)
    : sdm_node(id, type, lip, lport, joinhost_lip, joinhost_lport,
               rimsg_recv_cb, sdm_cp_recv_cb)
    {}
    int close() { return sdm_node.close(); }
    
    int broadcast_msg(std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg(SDM_CMSG, msg_map); }
    int broadcast_msg_to_slaves(std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg_to_slaves(SDM_CMSG, msg_map); }
    int send_msg(char to_id, std::map<std::string, std::string> msg_map) {return sdm_node.send_msg(to_id, SDM_CMSG, msg_map); }
    
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

/*******************************************  SDMMaster  ******************************************/
class SDMMaster : public SDMCEntity {
  private:
    char predictor_t;
    
    char data_id_t;
    WASpace wa_space;
    int num_slaves;
    patch_sdm::syncer<unsigned int> sdm_m_syncer;
  public:
    SDMMaster(char predictor_t,
              char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb rimsg_recv_cb,
              int pbuffer_size, int pexpand_length, COOR_T* lcoor_, COOR_T* ucoor_);
    ~SDMMaster();
    
    int sdm_mquery(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    void handle_conn_up(std::map<std::string, std::string> msg_map);
    void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_reg_app(std::map<std::string, std::string> msg_map);
    void handle_sdm_put_notification(std::map<std::string, std::string> msg_map);
    void handle_sdm_squery(std::map<std::string, std::string> msg_map);
    void handle_sdm_move_reply(std::map<std::string, std::string> msg_map);
};

/*******************************************  SDMSlave  *******************************************/
const std::string SDM_MQUERY = "sdm_mq";
const std::string SDM_MQUERY_REPLY = "sdm_mqr";
const std::string SDM_SQUERY = "sdm_sq";
const std::string SDM_SQUERY_REPLY = "sdm_sqr";
const std::string SDM_REG_APP = "sdm_ra";
const std::string SDM_REG_APP_REPLY = "sdm_rar";
const std::string SDM_PUT_NOTIFICATION = "sdm_pn";

const std::string SDM_MOVE = "sdm_m";
const std::string SDM_MOVE_REPLY = "sdm_mr";

typedef boost::function<void(std::map<std::string, std::string> ) > func_dm_act_cb;
class SDMSlave : public SDMCEntity {
  private:
    char data_id_t;
    func_dm_act_cb dm_act_cb;
    
    boost::shared_ptr<QTable<int> > qtable_;
    std::vector<int> app_id_v;
    patch_sdm::syncer<unsigned int> sdm_s_syncer;
    patch_sdm::thread_safe_vector<std::string> data_id_to_bget_v;
  public:
    SDMSlave(char data_id_t, func_dm_act_cb dm_act_cb,
             char ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
             func_rimsg_recv_cb rimsg_recv_cb);
    ~SDMSlave();
    
    int send_msg_to_master(std::map<std::string, std::string> msg_map);
    
    int reg_app(int app_id);
    int put(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id);
    int get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    void handle_conn_up(std::map<std::string, std::string> msg_map);
    void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery(std::map<std::string, std::string> msg_map);
    void handle_sdm_squery_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_reg_app_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_move(std::map<std::string, std::string> msg_map);
};

#endif //_SDM_CONTROL_H_
