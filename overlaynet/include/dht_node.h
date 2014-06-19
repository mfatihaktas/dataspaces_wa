#ifndef DHTNODE_H
#define DHTNODE_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <glog/logging.h>
//for wait_flag
#include <csignal>

#include "dht_server.h"
#include "dht_client.h"
#include "packet.h"

//forward declerations

struct comm_channel {
  char *lip, *peer_lip, *channel_name;
  int lport, peer_lport;
  boost::shared_ptr< DHTServer > server_;
  boost::shared_ptr< DHTClient > client_;
  //
  comm_channel(char* channel_name, char* lip, int lport, char* peer_lip = NULL, int peer_lport = 0)
  : server_( new DHTServer(channel_name, lip, lport) ),
    client_( new DHTClient(channel_name, peer_lip, peer_lport) )
  {
    this->channel_name = channel_name;
    this->lip = lip;
    this->lport = lport;
    this->peer_lip = peer_lip;
    this->peer_lport = peer_lport;
  }
  
  void reinit()
  {
    close();
    client_.reset();
    server_.reset();
    
    boost::shared_ptr< DHTServer > temp_server_( new DHTServer(channel_name, lip, lport) );
    server_ = temp_server_;
    
    //to make sure if first node gets closed by ctrl+c shared_ptr client_ will exist to get closed
    boost::shared_ptr< DHTClient > temp_client_( new DHTClient(channel_name, NULL, 0) );
    client_ = temp_client_;
  }
  
  void reinit_client(char* peer_lip, int peer_lport)
  {
    this->peer_lip = peer_lip;
    this->peer_lport = peer_lport;
    client_.reset();
    boost::shared_ptr< DHTClient > temp_client_( new DHTClient(channel_name, peer_lip, peer_lport) );
    client_ = temp_client_;
  }
  
  void conn_to_peer()
  {
    client_->connect();
  }
  
  int send_to_peer(const Packet& p)
  {
    return client_->send(p.get_packet_size(), p.get_data());
  }
  
  void set_recv_callback(function_recv_callback recv_callback)
  {
    server_->set_recv_callback(recv_callback);
  }
  
  void close()
  {
    client_->close();
    server_->close();
  }
  
  std::string to_str()
  {
    std::stringstream ss;
    ss << "\t channel_name=" << channel_name << "\n";
    ss << "\t lip=" << lip << ", lport=" << boost::lexical_cast<std::string>(lport) << "\n";
    ss << "\t peer_lip=" << peer_lip << ", peer_lport=" << boost::lexical_cast<std::string>(peer_lport) << "\n";
    return ss.str();
  }
};

struct peer_info{
  char id;
  char *lip, *peer_lip, *peer_name;
  int lport, peer_lport;
  //
  peer_info(char id, char* peer_name, char* lip, int lport, char* peer_lip, int peer_lport)
  {
    this->id = id;
    this->peer_name = peer_name;
    this->lip = lip;
    this->lport = lport;
    this->peer_lip = peer_lip;
    this->peer_lport = peer_lport;
  }
  
  std::string to_str() const
  {
    std::stringstream ss;
    ss << "\t id=" << id << " peer_name=" << peer_name << "\n";
    ss << "\t lip=" << lip << ", lport=" << boost::lexical_cast<std::string>(lport) << "\n";
    ss << "\t peer_lip=" << peer_lip << ", peer_lport=" << boost::lexical_cast<std::string>(peer_lport);
    return ss.str();
  }
};

struct peer_table{
  std::vector<char> peer_id_vector;
  std::map<char, boost::shared_ptr<peer_info> > id_pinfo_map;
  std::map<char, boost::shared_ptr<comm_channel> > id_commchannel_map;
  //peer_name is solely for debugging purposes
  int add_peer(char peer_id, char* peer_name, char* lip, int lport, char* peer_lip = NULL, int peer_lport = 0)
  {
    if (id_pinfo_map.count(peer_id) ){
      LOG(ERROR) << "add_peer:: already added; peer_id=" << peer_id;
      return 1;
    }
    boost::shared_ptr<peer_info> temp_( new peer_info(peer_id, peer_name, lip, lport, peer_lip, peer_lport) );
    id_pinfo_map[peer_id] = temp_;
    
    //
    LOG(INFO) << "add_peer:: added peer_id=" << peer_id << "; peer_info=\n" << temp_->to_str();
    peer_id_vector.push_back(peer_id);
    return 0;
  }
  
  int del_peer(char peer_id)
  {
    if (!id_pinfo_map.count(peer_id) ){
      LOG(ERROR) << "del_peer:: non-existing peer_id=" << peer_id;
      return 1;
    }
    
    std::map<char, boost::shared_ptr<peer_info> >::iterator it;
    it=id_pinfo_map.find(peer_id);
    id_pinfo_map.erase (it);
    
    std::map<char, boost::shared_ptr<comm_channel> >::iterator it2;
    it2=id_commchannel_map.find(peer_id);
    id_commchannel_map.erase (it2);
    
    //
    LOG(INFO) << "del_peer:: deleted peer_id=" << peer_id;
    peer_id_vector.erase(std::remove(peer_id_vector.begin(), peer_id_vector.end(), peer_id), peer_id_vector.end());
    return 0;
  }
  
  int add_comm_channel(char peer_id)
  {
    boost::shared_ptr<peer_info> pinfo_ = id_pinfo_map[peer_id];
    
    boost::shared_ptr<comm_channel> cc_(new comm_channel(pinfo_->peer_name, pinfo_->lip, pinfo_->lport, pinfo_->peer_lip, pinfo_->peer_lport) );
    id_commchannel_map[peer_id] = cc_;
    //
    LOG(INFO) << "add_comm_channel:: added comm_channel=\n" << cc_->to_str();
    return 0;
  }
  
  int close_comm_channel(char peer_id)
  {
    std::string tmp_str = (id_commchannel_map[peer_id])->to_str();
    (id_commchannel_map[peer_id])->close();
    //
    LOG(INFO) << "close_comm_channel:: closed comm_channel=\n" << tmp_str;
    return 0;
  }
  
  void set_recv_callback(char peer_id, function_recv_callback recv_callback)
  {
    id_commchannel_map[peer_id]->set_recv_callback(recv_callback);
  }
  
  int conn_to_peer(char peer_id)
  {
    id_commchannel_map[peer_id]->conn_to_peer();
    //
    return 0;
  }
  
  int send_to_peer(char peer_id, const Packet& p)
  {
    id_commchannel_map[peer_id]->send_to_peer(p);
    //
    return 0;
  }
  
  
  std::string to_str() const
  {
    std::stringstream ss;
    for (std::map<char, boost::shared_ptr<peer_info> >::const_iterator it=id_pinfo_map.begin(); it!=id_pinfo_map.end(); ++it){
      ss << "\t peer_id=" << it->first << "\n";
      ss << "\t ninfo=\n" << (it->second)->to_str() << "\n";
    }
    return ss.str();
  }
  
  void close_all_peers()
  {
    LOG(INFO) << "close_all_peers:: started...";
    
    for (std::map<char, boost::shared_ptr<comm_channel> >::const_iterator it=id_commchannel_map.begin(); 
         it!=id_commchannel_map.end(); ++it)
    {
      (it->second)->close();
    }
    //
    LOG(INFO) << "close_all_peers:: done.";
  }
};

class DHTNode{
  struct messenger{
    DHTNode* dhtnode_;
    //
    messenger(DHTNode* dhtnode_)
    {
      this->dhtnode_ = dhtnode_;
    }
    
    boost::shared_ptr< Packet > gen_join_req()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["join_lip"] = dhtnode_->lip;
      msg_map["join_lport"] = boost::lexical_cast<std::string>(dhtnode_->lport);
      msg_map["id"] = dhtnode_->id;
      msg_map["lip"] = dhtnode_->lip;
      msg_map["lport"] = boost::lexical_cast<std::string>(dhtnode_->get_next_lport() );
      
      boost::shared_ptr< Packet > temp_( new Packet(JOIN_REQUEST, msg_map) );
      return temp_;
    }
    
    boost::shared_ptr< Packet > gen_join_reply(bool pos)
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      msg_map["lip"] = dhtnode_->lip;
      msg_map["lport"] = boost::lexical_cast<std::string>(dhtnode_->get_next_lport());
      
      switch(pos)
      {
        case 1:
          msg_map["ack"] = "ok";
          break;
        case 0:
          msg_map["ack"] = "nok";
          break;
      }
      boost::shared_ptr< Packet > temp_ ( new Packet(JOIN_REPLY, msg_map) );
      return temp_;
    }
    
    boost::shared_ptr< Packet > gen_join_ack()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      boost::shared_ptr< Packet > temp_( new Packet(JOIN_ACK, msg_map) );
      return temp_;
    }
    
    boost::shared_ptr< Packet > gen_ping()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      boost::shared_ptr< Packet > temp_( new Packet(PING, msg_map) );
      return temp_;
    }
    
    boost::shared_ptr< Packet > gen_pong()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      boost::shared_ptr< Packet > temp_( new Packet(PONG, msg_map) );
      return temp_;
    }
  };
  //
  public:
    char id, *lip, *joinhost_lip;
    int lport, joinhost_lport;
    
    int next_lport;
    static boost::condition_variable cv;
    //
    DHTNode(char id, char* lip, int lport, 
            char* joinhost_lip = NULL, int joinhost_lport = 0 );
    ~DHTNode();
    
    void ping_peer(char peer_id);
    void test();
    
    void handle_recv(char* type__srlzedmsgmap);
    void handle_join_request(const Packet& p);
    void handle_join_reply(const Packet& p);
    void handle_join_ack(const Packet& p);
    void handle_ping(const Packet& p);
    void handle_pong(const Packet& p);
    
    int get_next_lport();
    void wait_for_flag();
    void close();
  private:
    comm_channel join_channel;
    messenger msger;
    peer_table ptable;
    
    //
    boost::mutex m;
};


#endif //DHTNODE_H
