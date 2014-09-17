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
//************************************  DHTNode  **********************************//
DHTNode::DHTNode(char id, func_rimsg_recv_cb _rimsg_recv_cb,
                 char* lip, int lport,
                 char* joinhost_lip, int joinhost_lport )
: join_channel((char*)"join", lip, lport, joinhost_lip, joinhost_lport),
  msger( this ),
  next_lport(lport)
{
  this->id = id;
  this->_rimsg_recv_cb = _rimsg_recv_cb;
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
    boost::shared_ptr<Packet> p_ = msger.gen_join_req();
    join_channel.send_to_peer( *p_ );
  }
  //
  // wait_for_flag();
  // close();
}

DHTNode::~DHTNode()
{
  LOG(INFO) << "destructed.";
}

int DHTNode::get_num_peers()
{
  return ptable.peer_id_vector.size();
}

int DHTNode::get_next_lport()
{
  boost::lock_guard<boost::mutex> guard(this->mutex);
  
  return ++next_lport;
}

void DHTNode::close()
{
  join_channel.close();
  ptable.close_all_peers();
  LOG(INFO) << "close:: node id=" << id << " closed.";
}

std::string DHTNode::to_str()
{
  std::stringstream ss;
  ss << "\t id=" << id << "\n";
  ss << "\t lip=" << lip << "\n";
  ss << "\t lport=" << lport << "\n";
  ss << "\t joinhost_lip=" << joinhost_lip << "\n";
  ss << "\t joinhost_lport=" << joinhost_lport << "\n";
  //
  return ss.str();
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
  boost::shared_ptr<Packet> p_ = msger.gen_ping();
  send_to_node(peer_id, *p_);
  
  LOG(INFO) << "ping_peer:: pinged peer_id=" << peer_id;
}

void DHTNode::fill_pptable(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "fill_pptable:: started.";
  int count = 0;
  while(1)
  {
    ++count;
    std::string key_tail_str = boost::lexical_cast<std::string>(count);
    std::string join_id_key_str = "join_id_" + key_tail_str;
    if (msg_map.count(join_id_key_str) == 0){
      //LOG(INFO) << "fill_pptable:: " << join_lip_key_str << " not in join_reply.";
      break;
    }
    char join_id = boost::lexical_cast<char>(msg_map[join_id_key_str] );
    if (ptable.is_peer(join_id) ){
      continue;
    }
    std::string join_lip_key_str = "join_lip_" + key_tail_str;
    char* join_lip = string_to_char_(msg_map[join_lip_key_str] );
    std::string join_lport_key_str = "join_lport_" + key_tail_str;
    int join_lport = boost::lexical_cast<int>(msg_map[join_lport_key_str] );
    
    pptable.push(join_lip, join_lport);
  }
  LOG(INFO) << "fill_pptable:: done.";
}

void DHTNode::handle_next_ppeer()
{
  boost::shared_ptr<prospective_peer_info> ppinfo_ = pptable.pop();
  if (!ppinfo_){
    LOG(INFO) << "handle_next_ppeer:: no pper to handle.";
    return;
  }
  
  join_channel.reinit_client(ppinfo_->join_lip, ppinfo_->join_lport);
  join_channel.conn_to_peer();
  
  boost::shared_ptr<Packet> p_ = msger.gen_join_req();
  join_channel.send_to_peer( *p_ );
}
/*****************************  messaging *************************************/
int DHTNode::send_msg(char to_id, char msg_type, std::map<std::string, std::string> msg_map)
{
  msg_map["id"] = this->id;
  // boost::shared_ptr<Packet> temp_p_( new Packet(msg_type, msg_map) );
  boost::shared_ptr<Packet> temp_p_ = boost::make_shared<Packet>(msg_type, msg_map);
  
  return send_to_node(to_id, *temp_p_);
}

int DHTNode::broadcast_msg(char msg_type, std::map<std::string, std::string> msg_map)
{
  if (ptable.peer_id_vector.empty() ){
    LOG(INFO) << "broadcast_msg:: no peer to broadcast.";
    return 0;
  }
  msg_map["id"] = this->id;
  
  // boost::shared_ptr< Packet > temp_p_( new Packet(msg_type, msg_map) );
  boost::shared_ptr<Packet> temp_p_ = boost::make_shared<Packet>(msg_type, msg_map);
  
  //LOG(INFO) << "broadcast_msg:: broadcasting...";
  return send_to_allpeernodes(*temp_p_);
}

int DHTNode::send_to_allpeernodes(const Packet& p)
{
  for (std::vector<char>::const_iterator it = ptable.peer_id_vector.begin();
      it != ptable.peer_id_vector.end(); it++)
  {
    if (send_to_node(*it, p)){
      LOG(ERROR) << "send_to_allpeernodes:: send_to_node failed for peer_id=" << *it;
      return 1;
    }
  }
  
  return 0;
}

int DHTNode::send_to_node(char id, const Packet& p)
{
  return ptable.send_to_peer(id, p);
}

/*****************************  handle_*** ************************************/
void DHTNode::handle_recv(char* type__srlzedmsgmap)
{
  size_t type__srlzedmsgmap_size =  strlen(type__srlzedmsgmap);
  // boost::shared_ptr<Packet> p_ ( new Packet(type__srlzedmsgmap_size, type__srlzedmsgmap) );
  boost::shared_ptr<Packet> p_ = boost::make_shared<Packet>(type__srlzedmsgmap_size, type__srlzedmsgmap);
  
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
    case RIMSG:
      handle_rimsg(*p_);
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
  //
  char* peer_name = new char[2];
  memcpy(peer_name, &id, 1);
  peer_name[1] = '\0';
  
  int r = ptable.add_peer(id, peer_name, this->lip, get_next_lport(), lip, lport, join_lip, join_lport);
  if (r == 0){
    ptable.add_comm_channel(id);
    function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
    ptable.set_recv_callback(id, fp_handle_recv);
  }
  else{
    LOG(ERROR) << "handle_join_request:: add_peer failed!";
  }
  boost::shared_ptr<Packet> p_ = msger.gen_join_reply(id, (r == 0) );
  //LOG(INFO) << "handle_join_request:: reply = \n" << p_->to_str();
  join_channel.send_to_peer( *p_ );
}

void DHTNode::handle_join_reply(const Packet& p)
{
  LOG(INFO) << "handle_join_reply:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  if (msg_map["ack"].compare("ok") == 0){
    char* join_lip = string_to_char_(msg_map["join_lip"]);
    int join_lport = atoi(msg_map["join_lport"].c_str());
    char id = (msg_map["id"].c_str())[0];
    char* lip = string_to_char_(msg_map["lip"]);
    int lport = atoi(msg_map["lport"].c_str());
    //
    char* peer_name = new char[2];
    memcpy(peer_name, &id, 1);
    peer_name[1] = '\0';
    
    int r = ptable.add_peer(id, peer_name, this->lip, next_lport, lip, lport, join_lip, join_lport);
    if (r == 0){
      ptable.add_comm_channel(id);
      function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
      ptable.set_recv_callback(id, fp_handle_recv);
      
      ptable.conn_to_peer(id);
      
      boost::shared_ptr< Packet > p_ = msger.gen_join_ack();
      join_channel.send_to_peer( *p_ );
      
      LOG(INFO) << "handle_join_reply:: joined :) to" << " node; id=" << id;
      
      LOG(INFO) << "handle_join_reply:: ptable=\n" << ptable.to_str();
      //get ready for incoming join_requests
      join_channel.reinit();
      join_channel.set_recv_callback(fp_handle_recv);
      //
      fill_pptable(msg_map);
      handle_next_ppeer();
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
  function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
  join_channel.set_recv_callback(fp_handle_recv);
  //
  test();
}

void DHTNode::handle_ping(const Packet& p)
{
  LOG(INFO) << "handle_ping:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char id = (msg_map["id"].c_str())[0];
  
  boost::shared_ptr< Packet > p_ = msger.gen_pong();
  send_to_node(id, *p_);
}

void DHTNode::handle_pong(const Packet& p)
{
  LOG(INFO) << "handle_pong:: p = \n" << p.to_str();
}

void DHTNode::handle_rimsg(const Packet& p)
{
  // LOG(INFO) << "handle_rimsg:: p = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  _rimsg_recv_cb(msg_map);
}
