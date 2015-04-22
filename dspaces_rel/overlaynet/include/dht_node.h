#ifndef _DHTNODE_H_
#define _DHTNODE_H_

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
// for wait_flag
#include <csignal>

#include "dht_server.h"
#include "dht_client.h"
#include "packet.h"

namespace patch_dht{
  template <typename Tk, typename Tv>
  struct thread_safe_map
  {
    private:
      boost::mutex mutex;
      typename std::map<Tk, Tv> map;
      typename std::map<Tk, Tv>::iterator map_it;
    public:
      thread_safe_map() {};
      ~thread_safe_map() {};
      
      Tv& operator[](Tk k) {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map[k];
      };
      
      int del(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        map_it = map.find(k);
        map.erase(map_it);
        return 0;
      };
      
      bool contains(Tk k)
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return !(map.count(k) == 0);
      };
      
      typename std::map<Tk, Tv>::iterator begin()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.begin();
      };
      
      typename std::map<Tk, Tv>::iterator end()
      {
        boost::lock_guard<boost::mutex> guard(this->mutex);
        return map.end();
      };
  };
}

//********************************  comm_channel  **********************************//
struct comm_channel {
  char *lip, *peer_lip, *channel_name;
  int lport, peer_lport;
  boost::shared_ptr<DHTServer> server_;
  boost::shared_ptr<DHTClient> client_;
  // 
  comm_channel(char* channel_name, char* lip, int lport, char* peer_lip = NULL, int peer_lport = 0)
  : channel_name(channel_name),
    lip(lip),
    lport(lport),
    peer_lip(peer_lip),
    peer_lport(peer_lport),
    server_(boost::make_shared<DHTServer>(channel_name, lip, lport) ),
    client_(boost::make_shared<DHTClient>(channel_name, peer_lip, peer_lport) ) 
    { }
  
  void reinit()
  {
    // LOG(INFO) << "reinit:: started.";
    
    close();
    client_.reset();
    server_.reset();
    
    boost::shared_ptr<DHTServer> temp_server_ = boost::make_shared<DHTServer>(channel_name, lip, lport);
    server_ = temp_server_;
    
    // To make sure if first node gets closed by ctrl+c shared_ptr client_ will exist to get closed
    boost::shared_ptr<DHTClient> temp_client_ = boost::make_shared<DHTClient>(channel_name, (char*)"", 0);
    client_ = temp_client_;
    
    // LOG(INFO) << "reinit:: done.";
  }
  
  void reinit_client(char* peer_lip, int peer_lport)
  {
    this->peer_lip = peer_lip;
    this->peer_lport = peer_lport;
    client_.reset();
    
    client_ = boost::make_shared<DHTClient>(channel_name, peer_lip, peer_lport);
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

//********************************  peer_info  **********************************//
struct peer_info{
  char id;
  char *lip, *peer_name, *peer_lip, *peer_join_lip;
  int lport, peer_lport, peer_join_lport;
  // 
  peer_info(char id, char* peer_name, char* lip, int lport, char* peer_lip, int peer_lport, 
            char* peer_join_lip = NULL, int peer_join_lport = 0)
  : id(id),
    peer_name(peer_name),
    lip(lip),
    lport(lport),
    peer_lip(peer_lip),
    peer_lport(peer_lport),
    peer_join_lip(peer_join_lip),
    peer_join_lport(peer_join_lport)
  { }
  
  std::string to_str() const
  {
    std::stringstream ss;
    ss << "\t id=" << id << " peer_name=" << peer_name << "\n";
    ss << "\t lip=" << lip << ", lport=" << boost::lexical_cast<std::string>(lport) << "\n";
    ss << "\t peer_lip=" << peer_lip << ", peer_lport=" << boost::lexical_cast<std::string>(peer_lport) << "\n";
    ss << "\t peer_join_lip=" << peer_join_lip << ", peer_join_lport=" << boost::lexical_cast<std::string>(peer_join_lport);
    
    return ss.str();
  }
};
//********************************  peer_table  **********************************//
struct peer_table{
  boost::mutex mutex;
  std::vector<char> peer_id_vector;
  patch_dht::thread_safe_map<char, boost::shared_ptr<peer_info> > id_pinfo_map;
  patch_dht::thread_safe_map<char, boost::shared_ptr<comm_channel> > id_commchannel_map;
  // 
  bool is_peer(char id)
  {
    if (std::find(peer_id_vector.begin(), peer_id_vector.end(), id) != peer_id_vector.end() ) {
      return true;
    }
    
    return false;
  }
  
  // peer_name is solely for debugging purposes
  int add_peer(char peer_id, char* peer_name, char* lip, int lport, 
               char* peer_lip = NULL, int peer_lport = 0,
               char* peer_join_lip = NULL, int peer_join_lport = 0)
  {
    if (id_pinfo_map.contains(peer_id) ) {
      LOG(ERROR) << "add_peer:: already added peer_id=" << peer_id;
      return 1;
    }
    boost::shared_ptr<peer_info> temp_ = boost::make_shared<peer_info>(peer_id, peer_name, lip, lport, peer_lip, peer_lport, peer_join_lip, peer_join_lport);
    id_pinfo_map[peer_id] = temp_;
    // 
    LOG(INFO) << "add_peer:: added peer_id=" << peer_id << "; peer_info=\n" << temp_->to_str();
    peer_id_vector.push_back(peer_id);
    return 0;
  }
  
  int del_peer(char peer_id)
  {
    if (!id_pinfo_map.contains(peer_id) ) {
      LOG(ERROR) << "del_peer:: non-existing peer_id=" << peer_id;
      return 1;
    }
    
    id_pinfo_map.del(peer_id);
    id_commchannel_map.del(peer_id);
    // 
    LOG(INFO) << "del_peer:: deleted peer_id=" << peer_id;
    peer_id_vector.erase(std::remove(peer_id_vector.begin(), peer_id_vector.end(), peer_id), peer_id_vector.end() );
    return 0;
  }
  
  int add_comm_channel(char peer_id)
  {
    boost::shared_ptr<peer_info> pinfo_ = id_pinfo_map[peer_id];
    
    boost::shared_ptr<comm_channel> cc_ = boost::make_shared<comm_channel>(pinfo_->peer_name, pinfo_->lip, pinfo_->lport, pinfo_->peer_lip, pinfo_->peer_lport);
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
  
  std::string to_str()
  {
    std::stringstream ss;
    for (std::map<char, boost::shared_ptr<peer_info> >::iterator it = id_pinfo_map.begin(); 
         it != id_pinfo_map.end(); it++) {
      ss << "\t peer_id=" << it->first << "\n";
      ss << "\t peer_info=\n" << (it->second)->to_str() << "\n";
      ss << "\n";
    }
    return ss.str();
  }
  
  void close_all_peers()
  {
    LOG(INFO) << "close_all_peers:: started...";
    
    for (std::map<char, boost::shared_ptr<comm_channel> >::iterator it = id_commchannel_map.begin(); 
         it != id_commchannel_map.end(); it++) {
      (it->second)->close();
    }
    // 
    LOG(INFO) << "close_all_peers:: done.";
  }
};

//********************************  prospective_peer_info  **********************************//
struct prospective_peer_info
{
  char* join_lip;
  int join_lport;
  // 
  prospective_peer_info(char* join_lip, int join_lport)
  {
    this->join_lip = join_lip;
    this->join_lport = join_lport;
  }
  
  std::string to_str()
  {
    std::stringstream ss;
    ss << "\t join_lip=" << join_lip << "\n";
    ss << "\t join_lport=" << boost::lexical_cast<std::string>(join_lport) << "\n";
    // 
    return ss.str();
  }
};

//********************************  prospective_peer_table  **********************************//
struct prospective_peer_table
{
  boost::mutex mutex;
  std::vector< boost::shared_ptr<prospective_peer_info> > ppeer_vector;
  
  int push(char* join_lip, int join_lport)
  {
    boost::lock_guard<boost::mutex> guard(this->mutex);
    boost::shared_ptr<prospective_peer_info> ppinfo_ = boost::make_shared<prospective_peer_info>(join_lip, join_lport);
    ppeer_vector.push_back(ppinfo_);
    // 
    LOG(INFO) << "push:: pushed prospective_peer_info=\n" << ppinfo_->to_str() << "\n";
    return 0;
  }
  
  boost::shared_ptr<prospective_peer_info> pop()
  {
    boost::lock_guard<boost::mutex> guard(this->mutex);
    
    if (ppeer_vector.empty()){
      boost::shared_ptr<prospective_peer_info> ppinfo_;
      return ppinfo_;
    }
    // 
    boost::shared_ptr<prospective_peer_info> ppinfo_= ppeer_vector.back();
    ppeer_vector.pop_back();
    // 
    LOG(INFO) << "pop:: popped prospective_peer_info=\n" << ppinfo_->to_str() << "\n";
    return ppinfo_;
  }
};

//********************************  DHTNode  **********************************//
typedef boost::function<void(std::map<std::string, std::string>)> func_rimsg_recv_cb;

class DHTNode {
  struct messenger {
    DHTNode* dhtnode_;
    // 
    messenger(DHTNode* dhtnode_)
    {
      this->dhtnode_ = dhtnode_;
    }
    
    boost::shared_ptr<Packet> gen_join_req()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["join_lip"] = dhtnode_->lip;
      msg_map["join_lport"] = boost::lexical_cast<std::string>(dhtnode_->lport);
      msg_map["id"] = dhtnode_->id;
      msg_map["lip"] = dhtnode_->lip;
      msg_map["lport"] = boost::lexical_cast<std::string>(dhtnode_->get_next_lport() );
      
      return boost::make_shared<Packet>(JOIN_REQUEST, msg_map);
    }
    
    boost::shared_ptr<Packet> gen_join_reply(char peer_id, bool pos)
    {
      std::map<std::string, std::string> msg_map;
      msg_map["join_lip"] = dhtnode_->lip;
      msg_map["join_lport"] = boost::lexical_cast<std::string>(dhtnode_->lport);
      msg_map["id"] = dhtnode_->id;
      msg_map["lip"] = dhtnode_->lip;
      msg_map["lport"] = boost::lexical_cast<std::string>(dhtnode_->next_lport);
      
      switch(pos)
      {
        case 1:
          msg_map["ack"] = "ok";
          break;
        case 0:
          msg_map["ack"] = "nok";
          break;
      }
      if (pos){
        // Add other peers new peer should connect to
        int count = 0;
        for (std::map<char, boost::shared_ptr<peer_info> >::const_iterator it = (dhtnode_->ptable.id_pinfo_map).begin();
            it != dhtnode_->ptable.id_pinfo_map.end(); it++)
        {
          if ( !(it->first == peer_id) ){
            peer_info pinfo = *(it->second);
            
            ++count;
            std::string key_tail_str = boost::lexical_cast<std::string>(count);
            std::string join_id_key_str = "join_id_" + key_tail_str;
            msg_map[join_id_key_str] = boost::lexical_cast<std::string>(pinfo.id);
            std::string join_lip_key_str = "join_lip_" + key_tail_str;
            msg_map[join_lip_key_str] = pinfo.peer_join_lip;
            std::string join_lport_key_str = "join_lport_" + key_tail_str;
            msg_map[join_lport_key_str] = boost::lexical_cast<std::string>(pinfo.peer_join_lport);
          }
        }
      }
      // 
      return boost::make_shared<Packet>(JOIN_REPLY, msg_map);
    }
    
    boost::shared_ptr<Packet> gen_join_ack()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      return boost::make_shared<Packet>(JOIN_ACK, msg_map);
    }
    
    boost::shared_ptr<Packet> gen_ping()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      return boost::make_shared<Packet>(PING, msg_map);
    }
    
    boost::shared_ptr<Packet> gen_pong()
    {
      std::map<std::string, std::string> msg_map;
      msg_map["id"] = dhtnode_->id;
      
      return boost::make_shared<Packet>(PONG, msg_map);
    }
  };
  // 
  public:
    char id, *lip, *joinhost_lip;
    int lport, joinhost_lport;
    
    int next_lport;
    static boost::condition_variable cv;
    // 
    DHTNode(char id, func_rimsg_recv_cb _rimsg_recv_cb,
            char* lip, int lport, 
            char* joinhost_lip, int joinhost_lport );
    ~DHTNode();
    int get_num_peers();
    
    void ping_peer(char peer_id);
    void test();
    int send_msg(char to_id, char msg_type, std::map<std::string, std::string> msg_map);
    int broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map);
    int send_to_allpeernodes(const Packet& p);
    int send_to_node(char id, const Packet& p);
    
    void fill_pptable(std::map<std::string, std::string> msg_map);
    void handle_next_ppeer();
    
    void handle_recv(char* type__srlzedmsgmap);
    void handle_join_request(const Packet& p);
    void handle_join_reply(const Packet& p);
    void handle_join_ack(const Packet& p);
    void handle_ping(const Packet& p);
    void handle_pong(const Packet& p);
    void handle_rimsg(const Packet& p);
    
    int get_next_lport();
    void wait_for_flag();
    void close();
    std::string to_str();
  private:
    // ImpRem: properties must be thread-safe
    boost::mutex mutex;
    
    comm_channel join_channel;
    messenger msger;
    peer_table ptable;
    prospective_peer_table pptable;
    // 
    boost::mutex m;
    
    func_rimsg_recv_cb _rimsg_recv_cb;
};


#endif //_DHTNODE_H_
