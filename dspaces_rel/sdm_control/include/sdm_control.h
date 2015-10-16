#ifndef _SDM_CONTROL_H_
#define _SDM_CONTROL_H_

#include "prefetch.h"
#include "patch_sdm.h"
#include "sdm_node.h"

/******************************************  SDMCEntity  ******************************************/
class SDMCEntity { // SDM Control
  protected:
    SDMNode sdm_node;
    patch_sdm::MsgCoder msg_coder;
  public:
    SDMCEntity(std::string type,
               int id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb rimsg_recv_cb)
    : sdm_node(type, false,
               id, lip, lport, joinhost_lip, joinhost_lport,
               rimsg_recv_cb, boost::bind(&SDMCEntity::handle_cmsg, this, _1) )
    {}
    
    virtual int close() { return sdm_node.close(); }
    
    virtual std::string to_str()
    {
      std::stringstream ss;
      ss << "sdm_node= \n" << sdm_node.to_str() << "\n";
      return ss.str();
    }
    
    int get_id() { return sdm_node.get_id(); }
    
    int send_cmsg_to_master(std::map<std::string, std::string> msg_map) { return sdm_node.send_msg_to_master(PACKET_CMSG, msg_map); }
    // int broadcast_msg(PACKET_T packet_t, std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg(packet_t, msg_map); }
    int broadcast_cmsg_to_slaves(std::map<std::string, std::string> msg_map) { return sdm_node.broadcast_msg_to_slaves(PACKET_CMSG, msg_map); }
    int send_cmsg(int to_id, std::map<std::string, std::string> msg_map) { return sdm_node.send_msg(to_id, PACKET_CMSG, msg_map); }
    int send_rimsg(int to_id, std::map<std::string, std::string> msg_map) { return sdm_node.send_msg(to_id, PACKET_RIMSG, msg_map); }
    
    void handle_cmsg(boost::shared_ptr<Packet> p_)
    {
      // LOG(INFO) << "handle_cmsg:: p= " << p_->to_str();
      switch (p_->get_type() ) {
        case PACKET_JOIN_ACK:
          handle_conn_up(p_->get_msg_map() );
          break;
        case PACKET_CMSG:
          handle_msg_in(p_->get_msg_map() );
          break;
        default:
          LOG(WARNING) << "handle_cmsg:: unexpected p.get_type= " << p_->get_type();
          break;
      }
    }
    
    virtual void handle_conn_up(std::map<std::string, std::string> msg_map) = 0;
    virtual void handle_msg_in(std::map<std::string, std::string> msg_map) = 0;
};

/***********************************  SDMSlave : SDMCEntity  **************************************/
const std::string SDM_MQUERY = "sdm_mq";
const std::string SDM_MQUERY_REPLY = "sdm_mqr";
const std::string SDM_SQUERY = "sdm_sq";
const std::string SDM_SQUERY_REPLY = "sdm_sqr";
const std::string SDM_REG_APP = "sdm_ra";
const std::string SDM_REG_APP_REPLY = "sdm_rar";
const std::string SDM_PUT_NOTIFICATION = "sdm_pn";

const std::string SDM_MOVE = "sdm_m";
const std::string SDM_MOVE_REPLY = "sdm_mr";
const std::string SDM_DEL = "sdm_d";
const std::string SDM_DEL_REPLY = "sdm_dr";

const int REMOTE_P_ID = -1;

typedef boost::function<void(std::map<std::string, std::string> ) > func_dm_act_cb;
class SDMSlave : public SDMCEntity {
  protected:
    DATA_ID_T data_id_t;
    func_dm_act_cb dm_act_cb;
    std::string type;
    
    boost::shared_ptr<QTable<int> > qtable_;
    
    std::vector<int> app_id_v;
    patch_all::syncer<unsigned int> sdm_s_syncer;
    patch_all::thread_safe_vector<std::string> data_id_to_bget_v;
  public:
    SDMSlave(DATA_ID_T data_id_t, std::string type,
             int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
             func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
             boost::shared_ptr<QTable<int> > qtable_);
    // SDMSlave(const SDMSlave&);
    ~SDMSlave();
    virtual int close();
    virtual std::string to_str();
    
    virtual int reg_app(int app_id);
    virtual int put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id);
    virtual int get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    virtual void handle_conn_up(std::map<std::string, std::string> msg_map);
    virtual void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery(std::map<std::string, std::string> msg_map);
    void handle_sdm_squery_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_reg_app_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_move(std::map<std::string, std::string> msg_map);
    void handle_sdm_del(std::map<std::string, std::string> msg_map);
};

/*************************************  MSDMSlave : SDMSlave  *************************************/
class MSDMSlave : public SDMSlave { // Markov
  public:
    MSDMSlave(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb);
    ~MSDMSlave();
};

/*************************************  SSDMSlave : SDMSlave  *************************************/
class SSDMSlave : public SDMSlave { // Spatial
  public:
    SSDMSlave(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb);
    ~SSDMSlave();
};

/*************************************  SDMMaster : SDMSlave  *************************************/
class SDMMaster : public SDMSlave {
  protected:
    boost::shared_ptr<WASpace> wa_space_;
    
    int num_slaves;
    patch_all::syncer<unsigned int> sdm_m_syncer;
  public:
    SDMMaster(DATA_ID_T data_id_t,
              int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
              func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
              boost::shared_ptr<QTable<int> > qtable_, boost::shared_ptr<WASpace> wa_space_);
    ~SDMMaster();
    int close();
    virtual std::string to_str();
    
    int sdm_mquery(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    int reg_app(int app_id);
    int put(bool notify, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int p_id);
    int get(bool blocking, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    void handle_conn_up(std::map<std::string, std::string> msg_map);
    void handle_msg_in(std::map<std::string, std::string> msg_map);
    void handle_sdm_mquery_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_reg_app(std::map<std::string, std::string> msg_map);
    void handle_sdm_put_notification(std::map<std::string, std::string> msg_map);
    void handle_sdm_squery(std::map<std::string, std::string> msg_map);
    void handle_sdm_move_reply(std::map<std::string, std::string> msg_map);
    void handle_sdm_del_reply(std::map<std::string, std::string> msg_map);
    
    void handle_wa_space_data_act(PREFETCH_DATA_ACT_T data_act_t, int to_id, key_ver_pair kv, lcoor_ucoor_pair lucoor_);
};

/************************************  MSDMMaster : SDMMaster  ************************************/
class MSDMMaster : public SDMMaster { // Markov
  public:
    MSDMMaster(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
               MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch);
    ~MSDMMaster();
    std::string to_str();
};

/************************************  SSDMMaster : SDMMaster  ************************************/
class SSDMMaster : public SDMMaster { // Spatial
  public:
    SSDMMaster(int ds_id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
               func_rimsg_recv_cb rimsg_recv_cb, func_dm_act_cb dm_act_cb,
               SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch);
    ~SSDMMaster();
    std::string to_str();
};

#endif //_SDM_CONTROL_H_
