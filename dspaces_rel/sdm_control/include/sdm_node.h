#ifndef _SDM_NODE_H_
#define _SDM_NODE_H_

#include "patch_pre.h"
#include "sdm_server.h"
#include "sdm_client.h"
#include "packet.h"

/******************************************  peer_info  *******************************************/
struct peer_info {
  std::string name, lip;
  int lport;
  
  peer_info(std::string name, std::string lip, int lport);
  ~peer_info();
  std::string to_str();
};

/********************************************  Commer  ********************************************/
typedef boost::function<void(char*)> func_recv_cb;

class Commer {
  private:
    int id;
    std::string lip;
    int lport;
    func_recv_cb _recv_cb;
    
    SDMServer server;
    patch_all::thread_safe_map<int, boost::shared_ptr<SDMClient> > peer_id__client_map;
    patch_all::thread_safe_map<int, boost::shared_ptr<peer_info> > peer_id__peer_info_map;
  public:
    Commer(int id, std::string lip, int lport,
           func_recv_cb _recv_cb);
    ~Commer();
    int close();
    std::string to_str();
    
    patch_all::thread_safe_map<int, boost::shared_ptr<peer_info> >& get_peer_id__peer_info_map();
    int get_num_peers();
    bool is_peer(int peer_id);
    
    int add_peer(int peer_id, std::string peer_name, std::string peer_lip, int peer_lport);
    int rm_peer(int peer_id);
    int send_to_all_peers(int except_id, const Packet& p);
    int send_to_peer(int peer_id, const Packet& p);
    int connect_send_to(std::string to_lip, int to_lport, const Packet& p);
    int connect(std::string to_lip, int to_lport);
};

/*******************************************  SDMNode  ********************************************/
typedef boost::function<void(std::map<std::string, std::string>)> func_rimsg_recv_cb;
typedef boost::function<void(boost::shared_ptr<Packet>)> func_cmsg_recv_cb;

class SDMNode {
  private:
    std::string type; // For now can be only "m -- sdm_master" or "s -- sdm_slave"
    bool master_slave;
    int id;
    std::string lip, joinhost_lip;
    int lport, joinhost_lport;
    func_rimsg_recv_cb rimsg_recv_cb;
    func_cmsg_recv_cb cmsg_recv_cb;
    // ImpRem: properties must be thread-safe
    Commer commer;
    int sdm_master_id;
    
    patch_all::syncer<int> syncer;
  public:
    SDMNode(std::string type, bool master_slave,
            int id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
            func_rimsg_recv_cb rimsg_recv_cb, func_cmsg_recv_cb cmsg_recv_cb = NULL);
    ~SDMNode();
    int close();
    std::string to_str();
    
    int get_id();
    int get_num_peers();
    
    boost::shared_ptr<Packet> gen_join_req();
    boost::shared_ptr<Packet> gen_join_reply(int peer_id, bool pos);
    boost::shared_ptr<Packet> gen_packet(PACKET_T packet_t, std::map<std::string, std::string> msg_map = std::map<std::string, std::string>() );
    void ping_peer(int peer_id);

    int send_msg_to_master(PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    int broadcast_msg(PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    int broadcast_msg_to_slaves(PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    int send_msg(int to_id, PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    
    void handle_recv(char* type__srlzed_msg_map);
    void handle_join_req(std::map<std::string, std::string> msg_map);
    void handle_join_reply(std::map<std::string, std::string> msg_map);
    void handle_join_nack(std::map<std::string, std::string> msg_map);
    void handle_ping(std::map<std::string, std::string> msg_map);
};

#endif //_SDM_NODE_H_
