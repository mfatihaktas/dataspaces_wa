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
//for wait_flag
#include <csignal>

#include "dht_server.h"
#include "dht_client.h"
#include "packet.h"

//forward declerations
//'n': neighbor


struct comm_channel {
  char *lip, *neigh_lip;
  int lport, neigh_lport;
  boost::shared_ptr< DHTServer > server_;
  boost::shared_ptr< DHTClient > client_;
  //
  comm_channel(char* lip, int lport, char* neigh_lip = NULL, int neigh_lport = 0)
  : server_( new DHTServer(lip, lport, NULL) ),
    client_( new DHTClient(neigh_lip, neigh_lport) )
  {
    this->lip = lip;
    this->lport = lport;
    this->neigh_lip = neigh_lip;
    this->neigh_lport = neigh_lport;
  }
  
  void reinit_client(char* neigh_lip, int neigh_lport)
  {
    this->neigh_lip = neigh_lip;
    this->neigh_lport = neigh_lport;
    client_.reset();
    boost::shared_ptr< DHTClient > temp_client_(
      new DHTClient(neigh_lip, neigh_lport) 
    );
    client_ = temp_client_;
  }
  
  void conn_to_neigh()
  {
    client_->connect();
  }
  
  int send_to_neigh(const Packet& p)
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
};

struct node_info{
  char id;
  char* lip;
  int lport;
  //
  node_info(char id, char* lip, int lport)
  {
    this->id = id;
    this->lip = lip;
    this->lport = lport;
  }
};

struct neigh_table{
  std::map<char, boost::shared_ptr<node_info> > id_ninfo_map;
  //
  void add_n(char id, char* lip, int lport)
  {
    boost::shared_ptr<node_info> temp_(
      new node_info(id, lip, lport)
    );
    id_ninfo_map[id] = temp_;
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
      node_info ni = dhtnode_->ninfo;
      msg_map["id"] = ni.id;
      msg_map["lip"] = ni.lip;
      msg_map["lport"] = boost::lexical_cast<std::string>(ni.lport);
      
      boost::shared_ptr< Packet > temp_(
        new Packet(JOIN_REQUEST, msg_map)
      );
      return temp_;
    }
  };
  //
  public:
    node_info ninfo;
    static boost::condition_variable cv;
    //
    DHTNode(char id, char* lip, int lport, 
            char* ipeer_lip = NULL, int ipeer_lport = 0 );
    ~DHTNode();
    
    void handle_recv(char* type__srlzedmsgmap);
    void handle_join_request(const Packet& p);
    
    void wait_for_flag();
  private:
    comm_channel join_channel;
    messenger msger;
    //
    boost::mutex m;
};


#endif //DHTNODE_H
