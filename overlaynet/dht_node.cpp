#include "dht_node.h"

boost::condition_variable DHTNode::cv;

char* string_to_char_(std::string str)
{
  char* temp_ = new char[str.length()];
  strcpy(temp_, str.c_str());
  
  return temp_;
}

void handle_signal(int signum)
{
  LOG(INFO) << "handle_signal:: recved signum=" << signum;
  DHTNode::cv.notify_one();
}

DHTNode::DHTNode(char id, char* lip, int lport,
                 char* joinhost_lip, int joinhost_lport )
: join_channel((char*)"join", lip, lport, joinhost_lip, joinhost_lport),
  msger( this ),
  next_lport(lport)
{
  this->id = id;
  this->lip = lip;
  this->lport = lport;
  this->joinhost_lip = joinhost_lip;
  this->joinhost_lport = joinhost_lport;
  //
  function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
  join_channel.set_recv_callback(fp_handle_recv);
  
  signal(SIGINT, handle_signal);
  //
  LOG(INFO) << "constructed.";
  //
  if ( joinhost_lip == NULL ){ //first node
    LOG(INFO) << "FIRST NODE";
  }
  else{
    join_channel.conn_to_peer();
    boost::shared_ptr< Packet > p_ = msger.gen_join_req();
    join_channel.send_to_peer( *p_ );
  }
  //
  wait_for_flag();
  close();
}

DHTNode::~DHTNode()
{
  LOG(INFO) << "destructed.";
}

int DHTNode::get_next_lport()
{
  return ++next_lport;
}

void DHTNode::close()
{
  join_channel.close();
  ptable.close_all_peers();
  LOG(INFO) << "close:: node id=" << id << " closed.";
}

void DHTNode::wait_for_flag()
{
  LOG(INFO) << "wait_for_flag:: waiting...";
  boost::mutex::scoped_lock lock(m);
  cv.wait(lock);
  LOG(INFO) << "wait_for_flag:: done.";
}

void DHTNode::test()
{
  //ping all
  for (std::vector<char>::const_iterator it = ptable.peer_id_vector.begin();
      it != ptable.peer_id_vector.end(); it++)
  {
    ping_peer(*it);
  }
}

void DHTNode::ping_peer(char peer_id)
{
  boost::shared_ptr< Packet > p_ = msger.gen_ping();
  ptable.send_to_peer( peer_id, *p_ );
  
  LOG(INFO) << "ping_peer:: pinged peer_id=" << peer_id;
}

/*****************************  handle_*** ************************************/
void DHTNode::handle_recv(char* type__srlzedmsgmap)
{
  size_t type__srlzedmsgmap_size =  strlen(type__srlzedmsgmap);
  boost::shared_ptr< Packet > p_ ( new Packet(type__srlzedmsgmap_size, type__srlzedmsgmap) );
  
  switch (p_->get_type())
  {
    case JOIN_REQUEST:
      handle_join_request(*p_);
      break;
    case JOIN_REPLY:
      handle_join_reply(*p_);
      break;
    case JOIN_ACK:
      handle_join_ack(*p_);
      break;
    case PING:
      handle_ping(*p_);
      break;
    case PONG:
      handle_pong(*p_);
      break;
  }
  //
  delete type__srlzedmsgmap;
}

void DHTNode::handle_join_request(const Packet& p)
{
  LOG(INFO) << "handle_join_request:: request = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char* join_lip = string_to_char_(msg_map["join_lip"]);
  int join_lport = atoi(msg_map["join_lport"].c_str());
  char id = (msg_map["id"].c_str())[0];
  char* lip = string_to_char_(msg_map["lip"]);
  int lport = atoi(msg_map["lport"].c_str());
  
  join_channel.reinit_client(join_lip, join_lport);
  join_channel.conn_to_peer();
  
  boost::shared_ptr< Packet > p_ = msger.gen_join_reply(1);
  //LOG(INFO) << "handle_join_request:: reply = \n" << p_->to_str();
  join_channel.send_to_peer( *p_ );
  //
  char* peer_name = new char[2];
  memcpy(peer_name, &id, 1);
  peer_name[1] = '\0';
  
  int r = ptable.add_peer(id, peer_name, this->lip, next_lport, lip, lport);
  if (r == 0){
    ptable.add_comm_channel(id);
    function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
    ptable.set_recv_callback(id, fp_handle_recv);
  }
  else{
    LOG(ERROR) << "handle_join_request:: add_peer failed!";
  }
}

void DHTNode::handle_join_reply(const Packet& p)
{
  LOG(INFO) << "handle_join_reply:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  if (msg_map["ack"].compare("ok") == 0){
    char id = (msg_map["id"].c_str())[0];
    char* lip = string_to_char_(msg_map["lip"]);
    int lport = atoi(msg_map["lport"].c_str());
    //
    char* peer_name = new char[2];
    memcpy(peer_name, &id, 1);
    peer_name[1] = '\0';
    
    int r = ptable.add_peer(id, peer_name, this->lip, next_lport, lip, lport);
    if (r == 0){
      ptable.add_comm_channel(id);
      function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
      ptable.set_recv_callback(id, fp_handle_recv);
      
      ptable.conn_to_peer(id);
      
      boost::shared_ptr< Packet > p_ = msger.gen_join_ack();
      join_channel.send_to_peer( *p_ );
      join_channel.close();
      
      LOG(INFO) << "handle_join_reply:: joined :) Thanks to" << " node; id=" << id;
      
      LOG(INFO) << "handle_join_reply:: ptable=\n" << ptable.to_str();
    }
    else{
      LOG(ERROR) << "handle_join_reply:: add_peer failed!";
    }
  }
  else{
    LOG(INFO) << "handle_join_reply:: couldnt join :(";
  }
}

void DHTNode::handle_join_ack(const Packet& p)
{
  LOG(INFO) << "handle_join_ack:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char id = (msg_map["id"].c_str())[0];
  ptable.conn_to_peer(id);
  
  LOG(INFO) << "handle_join_ack:: ptable=\n" << ptable.to_str();
  
  //for incoming join_requests
  join_channel.reinit();
  
  //
  test();
}

void DHTNode::handle_ping(const Packet& p)
{
  LOG(INFO) << "handle_ping:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char id = (msg_map["id"].c_str())[0];
  
  boost::shared_ptr< Packet > p_ = msger.gen_pong();
  ptable.send_to_peer( id, *p_ );
}

void DHTNode::handle_pong(const Packet& p)
{
  LOG(INFO) << "handle_pong:: p = \n" << p.to_str();
}
