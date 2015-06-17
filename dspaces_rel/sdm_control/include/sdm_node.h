#ifndef _SDM_NODE_H_
#define _SDM_NODE_H_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <glog/logging.h>

#include "sdm_server.h"
#include "sdm_client.h"
#include "packet.h"
#include "patch_sdm.h"

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
    char id;
    std::string lip;
    int lport;
    func_recv_cb _recv_cb;
    
    SDMServer server;
    patch_sdm::thread_safe_map<char, boost::shared_ptr<SDMClient> > peer_id__client_map;
    patch_sdm::thread_safe_map<char, boost::shared_ptr<peer_info> > peer_id__peer_info_map;
  public:
    Commer(char id, std::string lip, int lport,
           func_recv_cb _recv_cb);
    ~Commer();
    int close();
    std::string to_str();
    
    patch_sdm::thread_safe_map<char, boost::shared_ptr<peer_info> >& get_peer_id__peer_info_map();
    int get_num_peers();
    bool is_peer(char peer_id);
    
    int add_peer(char peer_id, std::string peer_name, std::string peer_lip, int peer_lport);
    int rm_peer(char peer_id);
    int send_to_all_peers(const Packet& p);
    int send_to_peer(char peer_id, const Packet& p);
    int connect_send_to(std::string to_lip, int to_lport, const Packet& p);
};

/*******************************************  SDMNode  ********************************************/
typedef boost::function<void(std::map<std::string, std::string>)> func_rimsg_recv_cb;

class SDMNode {
  private:
    char id;
    std::string lip, joinhost_lip;
    int lport, joinhost_lport;
    func_rimsg_recv_cb _rimsg_recv_cb;
    // ImpRem: properties must be thread-safe
    Commer commer;
  public:
    SDMNode(char id, std::string lip, int lport,
            std::string joinhost_lip, int joinhost_lport,
            func_rimsg_recv_cb _rimsg_recv_cb);
    ~SDMNode();
    void close();
    std::string to_str();
    
    int get_num_peers();
    
    boost::shared_ptr<Packet> gen_join_req();
    boost::shared_ptr<Packet> gen_join_reply(char peer_id, bool pos);
    boost::shared_ptr<Packet> gen_packet(PACKET_T packet_t, std::map<std::string, std::string> msg_map = std::map<std::string, std::string>() );
    void ping_peer(char peer_id);
    // void test();
    int broadcast_msg(PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    int send_msg(char to_id, PACKET_T packet_t, std::map<std::string, std::string> msg_map);
    
    void handle_recv(char* type__srlzed_msg_map);
    void handle_join_req(const Packet& p);
    void handle_join_reply(const Packet& p);
    void handle_join_nack(const Packet& p);
    void handle_ping(const Packet& p);
    void handle_pong(const Packet& p);
};

#endif //_SDM_NODE_H_
