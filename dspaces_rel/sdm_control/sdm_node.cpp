#include "sdm_node.h"

/******************************************  peer_info  *******************************************/
peer_info::peer_info(std::string name, std::string lip, int lport)
: name(name), lip(lip), lport(lport)
{}
peer_info::~peer_info() {}

std::string peer_info::to_str()
{
  std::stringstream ss;
  ss << "name= " << name << ", lip= " << lip << ", lport= " << lport << "\n";
  
  return ss.str();
}

/********************************************  Commer  ********************************************/
Commer::Commer(char id, std::string lip, int lport,
               func_recv_cb _recv_cb)
: id(id), lip(lip), lport(lport),
  _recv_cb(_recv_cb),
  server("server_" + boost::lexical_cast<std::string>(id), lip, lport, _recv_cb)
{
  // 
  LOG(INFO) << "Commer:: constructed.";
}

Commer::~Commer() { LOG(INFO) << "Commer:: destructed."; }

int Commer::close()
{
  server.close();
  
  bool err_flag = false;
  for (std::map<char, boost::shared_ptr<SDMClient> >::iterator it = peer_id__client_map.begin(); it != peer_id__client_map.end(); it++) {
    if ((it->second)->close() )
      err_flag = true;
  }
  
  if (err_flag)
    return 1;
  return 0;
}

std::string Commer::to_str()
{
  std::stringstream ss;
  ss << "id= " << id << ", lip= " << lip << ", lport= " << lport << "\n";
  for (std::map<char, boost::shared_ptr<peer_info> >::iterator it = peer_id__peer_info_map.begin(); it != peer_id__peer_info_map.end(); it++)
    ss << "\t peer_id= " << it->first << "= \n" << (it->second)->to_str();

  return ss.str();
}

patch_sdm::thread_safe_map<char, boost::shared_ptr<peer_info> >& Commer::get_peer_id__peer_info_map() { return peer_id__peer_info_map; }
int Commer::get_num_peers() { return peer_id__peer_info_map.size(); }
bool Commer::is_peer(char peer_id) { return peer_id__client_map.contains(peer_id); }

int Commer::add_peer(char peer_id, std::string peer_name, std::string peer_lip, int peer_lport)
{
  if (peer_id__peer_info_map.contains(peer_id) ) {
    LOG(ERROR) << "add_peer:: already contained peer_id= " << peer_id;
    return 1;
  }
  peer_id__peer_info_map[peer_id] = boost::make_shared<peer_info>(peer_name, peer_lip, peer_lport);

  boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>(peer_name, peer_lip, peer_lport);
  client_->connect();
  peer_id__client_map[peer_id] = client_;
  
  return 0;
}

int Commer::rm_peer(char peer_id)
{
  if (!peer_id__peer_info_map.contains(peer_id) ) {
    LOG(ERROR) << "rm_peer:: does not contain peer_id= " << peer_id;
    return 1;
  }
  peer_id__client_map.del(peer_id);
  peer_id__peer_info_map.del(peer_id);
  
  return 0;
}

int Commer::send_to_all_peers(const Packet& p)
{
  bool err_flag = false;
  for (std::map<char, boost::shared_ptr<SDMClient> >::iterator it = peer_id__client_map.begin(); it != peer_id__client_map.end(); it++) {
    if (send_to_peer(it->first, p) ) {
      LOG(INFO) << "send_to_all_peers:: send_to_peer failed for peer_id= " << it->first;
      err_flag = true;
    }
  }
  
  if (err_flag)
    return 1;
  return 0;
}

int Commer::send_to_peer(char peer_id, const Packet& p)
{
  return peer_id__client_map[peer_id]->send(p.size(), p.get_data_() );
}

int Commer::connect_send_to(std::string to_lip, int to_lport, const Packet& p)
{
  // boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>(NULL, to_lip, to_lport);
  boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>("c", to_lip, to_lport);
  client_->connect();
  if (client_->send(p.size(), p.get_data_() ) ) {
    LOG(ERROR) << "connect_send_to:: client_->send failed!";
    client_.reset();
    return 1;
  }
  
  return 0;
}

/*******************************************  SDMNode  ********************************************/
SDMNode::SDMNode(char id, std::string lip, int lport,
                 std::string joinhost_lip, int joinhost_lport,
                 func_rimsg_recv_cb _rimsg_recv_cb)
: id(id), lip(lip), lport(lport),
  joinhost_lip(joinhost_lip), joinhost_lport(joinhost_lport),
  _rimsg_recv_cb(_rimsg_recv_cb),
  commer(id, lip, lport, boost::bind(&SDMNode::handle_recv, this, _1) )
{
  // 
  LOG(INFO) << "SDMNode:: constructed.";
  // 
  if (joinhost_lip.compare("") == 0) //first node
    LOG(INFO) << "SDMNode:: FIRST NODE.";
  else {
    // Just to get check joinhost status and get the join_reply
    if (commer.connect_send_to(joinhost_lip, joinhost_lport, *gen_join_req() ) ) {
      LOG(ERROR) << "SDMNode:: commer.connect_send_to failed for initial join; joinhost_lip= " << joinhost_lip << ", joinhost_lport= " << joinhost_lport;
      close();
      exit(1);
    }
  }
}

SDMNode::~SDMNode() { LOG(INFO) << "destructed."; }

void SDMNode::close()
{
  commer.close();
  LOG(INFO) << "close:: id= " << id << " closed.";
}

std::string SDMNode::to_str()
{
  std::stringstream ss;
  ss << "\t id= " << id << ", lip= " << lip << ", lport= " << lport << "\n"
     << "\t joinhost_lip= " << joinhost_lip << ", joinhost_lport= " << joinhost_lport << "\n";
  
  return ss.str();
}

int SDMNode::get_num_peers() { return commer.get_num_peers(); }

//--------------------------------------------  gen_packet  --------------------------------------//
boost::shared_ptr<Packet> SDMNode::gen_join_req()
{
  std::map<std::string, std::string> msg_map;
  msg_map["lip"] = lip;
  msg_map["lport"] = boost::lexical_cast<std::string>(lport);
  
  return gen_packet(SDM_JOIN_REQUEST, msg_map);
}

boost::shared_ptr<Packet> SDMNode::gen_join_reply(char peer_id, bool pos)
{
  std::map<std::string, std::string> msg_map;
  msg_map["lip"] = lip;
  msg_map["lport"] = boost::lexical_cast<std::string>(lport);
  
  if (pos)
    msg_map["ack"] = "a";
  else
    msg_map["ack"] = "n";
  
  if (pos) {
    // Add other peers new peer should connect to
    int count = 0;
    patch_sdm::thread_safe_map<char, boost::shared_ptr<peer_info> >& peer_id__peer_info_map = commer.get_peer_id__peer_info_map();
    for (std::map<char, boost::shared_ptr<peer_info> >::iterator it = peer_id__peer_info_map.begin(); it != peer_id__peer_info_map.end(); it++) {
      if (it->first != peer_id) {
        peer_info& pinfo = *(it->second);
        
        std::string key_tail_str = boost::lexical_cast<std::string>(count);
        msg_map["id_" + key_tail_str] = boost::lexical_cast<std::string>(it->first);
        msg_map["lip_" + key_tail_str] = pinfo.lip;
        msg_map["lport_" + key_tail_str] = boost::lexical_cast<std::string>(pinfo.lport);
        count++;
      }
    }
  }
  // 
  return gen_packet(SDM_JOIN_REPLY, msg_map);
}

boost::shared_ptr<Packet> SDMNode::gen_packet(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{
  msg_map["id"] = id;
  
  return boost::make_shared<Packet>(packet_t, msg_map);
}

//----------------------------------------  messaging  -------------------------------------------//
int SDMNode::send_msg(char to_id, PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{
  return commer.send_to_peer(to_id, *gen_packet(packet_t, msg_map) );
}

int SDMNode::broadcast_msg(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{
  return commer.send_to_all_peers(*gen_packet(packet_t, msg_map) );
}

//-----------------------------------------  handle_  --------------------------------------------//
void SDMNode::handle_recv(char* type__srlzed_msg_map)
{
  boost::shared_ptr<Packet> p_ = boost::make_shared<Packet>((int)strlen(type__srlzed_msg_map), type__srlzed_msg_map);
  
  switch (p_->get_type() ) {
    case SDM_JOIN_REQUEST:
      handle_join_req(*p_);
      break;
    case SDM_JOIN_REPLY:
      handle_join_reply(*p_);
      break;
    case SDM_JOIN_NACK:
      handle_join_nack(*p_);
      break;
    case SDM_PING:
      handle_ping(*p_);
      break;
    case SDM_PONG:
      handle_pong(*p_);
      break;
    case SDM_RIMSG:
      _rimsg_recv_cb(p_->get_msg_map() );
      break;
  }
  // 
  free(type__srlzed_msg_map);
}

void SDMNode::handle_join_req(const Packet& p)
{
  LOG(INFO) << "handle_join_req:: request = \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char from_id = (msg_map["id"].c_str() )[0];
  std::string from_lip = msg_map["lip"];
  int from_lport = atoi(msg_map["lport"].c_str() );
  
  if (commer.add_peer(from_id, "peer_" + boost::lexical_cast<std::string>(from_id), from_lip, from_lport) ) {
    LOG(ERROR) << "handle_join_req:: commer.add_peer failed; peer_id= " << from_id;
    commer.connect_send_to(from_lip, from_lport, *gen_join_reply(from_id, false) );
  }
  else
    commer.send_to_peer(from_id, *gen_join_reply(from_id, true) );
}

void SDMNode::handle_join_reply(const Packet& p)
{
  LOG(INFO) << "handle_join_reply:: reply= \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  if (msg_map["ack"].compare("a") == 0) {
    char from_id = (msg_map["id"].c_str() )[0];
    std::string from_lip = msg_map["lip"];
    int from_lport = atoi(msg_map["lport"].c_str() );
    // 
    if (commer.add_peer(from_id, "peer_" + boost::lexical_cast<std::string>(from_id), from_lip, from_lport) ) {
      commer.send_to_peer(from_id, *gen_packet(SDM_JOIN_NACK) );
      LOG(ERROR) << "handle_join_reply:: add_peer failed; peer_id= " << from_id;
    }
    else {
      LOG(INFO) << "handle_join_reply:: joined :) to peer_id= " << from_id;
      LOG(INFO) << "handle_join_reply:: commer= \n" << commer.to_str();
      commer.send_to_peer(from_id, *gen_packet(SDM_PING) );
      // Join others
      int count = 0;
      while (1) {
        std::string key_tail_str = boost::lexical_cast<std::string>(count);
        if (msg_map.count("id_" + key_tail_str) == 0)
          break;
        
        char ppeer_id = boost::lexical_cast<char>(msg_map["id_" + key_tail_str] );
        if (commer.is_peer(ppeer_id) )
          continue;
        
        std::string ppeer_lip = msg_map["lip_" + key_tail_str];
        int ppeer_lport = boost::lexical_cast<int>(msg_map["lport_" + key_tail_str] );
        
        if (commer.connect_send_to(ppeer_lip, ppeer_lport, *gen_join_req() ) )
          LOG(ERROR) << "SDMNode:: commer.connect_send_to failed for join; ppeer_lip= " << ppeer_lip << ", ppeer_lport= " << ppeer_lport;
        
        count++;
      }
    }
  }
  else
    LOG(INFO) << "handle_join_reply:: could not join :(";
}

void SDMNode::handle_join_nack(const Packet& p)
{
  LOG(INFO) << "handle_join_nack:: p= \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char from_id = (msg_map["id"].c_str() )[0];
  // With join_req recved, we connected to it, now we learned things did not work out on the other side
  if (commer.rm_peer(from_id) )
    LOG(ERROR) << "handle_join_nack:: commer.rm_peer failed; peer_id= " << from_id;
  
  LOG(INFO) << "handle_join_nack:: commer= \n" << commer.to_str();
}

void SDMNode::handle_ping(const Packet& p)
{
  LOG(INFO) << "handle_ping:: p= \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
  
  char from_id = (msg_map["id"].c_str() )[0];
  commer.send_to_peer(from_id, *gen_packet(SDM_PONG) );
}

void SDMNode::handle_pong(const Packet& p)
{
  LOG(INFO) << "handle_pong:: p= \n" << p.to_str();
  std::map<std::string, std::string> msg_map = p.get_msg_map();
}
