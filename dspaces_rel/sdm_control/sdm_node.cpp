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
Commer::Commer(int id, std::string lip, int lport,
               func_recv_cb _recv_cb)
: id(id), lip(lip), lport(lport),
  _recv_cb(_recv_cb),
  server("server_" + boost::lexical_cast<std::string>(id), lip, lport, _recv_cb)
{
  // 
  log_(INFO, "constructed.")
}

Commer::~Commer() { log_(INFO, "destructed.") }

int Commer::close()
{
  server.close();
  
  bool err_flag = false;
  for (std::map<int, boost::shared_ptr<SDMClient> >::iterator it = peer_id__client_map.begin(); it != peer_id__client_map.end(); it++) {
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
  for (std::map<int, boost::shared_ptr<peer_info> >::iterator it = peer_id__peer_info_map.begin(); it != peer_id__peer_info_map.end(); it++)
    ss << "\t peer_id= " << it->first << "= \n" << (it->second)->to_str();

  return ss.str();
}

patch::thread_safe_map<int, boost::shared_ptr<peer_info> >& Commer::get_peer_id__peer_info_map() { return peer_id__peer_info_map; }
int Commer::get_num_peers() { return peer_id__peer_info_map.size(); }
bool Commer::is_peer(int peer_id) { return peer_id__client_map.contains(peer_id); }

int Commer::add_peer(int peer_id, std::string peer_name, std::string peer_lip, int peer_lport)
{
  if (peer_id__peer_info_map.contains(peer_id) ) {
    log_(ERROR, "already contained peer_id= " << peer_id)
    return 1;
  }
  peer_id__peer_info_map[peer_id] = boost::make_shared<peer_info>(peer_name, peer_lip, peer_lport);

  boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>(peer_name, peer_lip, peer_lport);
  client_->connect();
  peer_id__client_map[peer_id] = client_;
  
  return 0;
}

int Commer::rm_peer(int peer_id)
{
  if (!peer_id__peer_info_map.contains(peer_id) ) {
    log_(ERROR, "does not contain peer_id= " << peer_id)
    return 1;
  }
  peer_id__client_map.del(peer_id);
  peer_id__peer_info_map.del(peer_id);
  
  return 0;
}

int Commer::send_to_all_peers(int except_id, const Packet& p)
{
  bool err_flag = false;
  for (std::map<int, boost::shared_ptr<SDMClient> >::iterator it = peer_id__client_map.begin(); it != peer_id__client_map.end(); it++) {
    if (it->first != except_id) {
      if (send_to_peer(it->first, p) ) {
        log_(INFO, "send_to_peer failed for peer_id= " << it->first)
        err_flag = true;
      }
    }
  }
  
  if (err_flag)
    return 1;
  return 0;
}

int Commer::send_to_peer(int peer_id, const Packet& p)
{
  return peer_id__client_map[peer_id]->send(p.size(), p.get_data_() );
}

int Commer::connect_send_to(std::string to_lip, int to_lport, const Packet& p)
{
  // boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>(NULL, to_lip, to_lport);
  boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>("c", to_lip, to_lport);
  client_->connect();
  if (client_->send(p.size(), p.get_data_() ) ) {
    log_(ERROR, "client_->send failed!")
    client_.reset();
    return 1;
  }
  
  return 0;
}

int Commer::connect(std::string to_lip, int to_lport)
{
  boost::shared_ptr<SDMClient> client_ = boost::make_shared<SDMClient>("c", to_lip, to_lport);
  client_->connect();
}

/*******************************************  SDMNode  ********************************************/
SDMNode::SDMNode(std::string type, bool master_slave,
                 int id, std::string lip, int lport, std::string joinhost_lip, int joinhost_lport,
                 func_rimsg_recv_cb rimsg_recv_cb, func_cmsg_recv_cb cmsg_recv_cb)
: type(type), master_slave(master_slave),
  id(id), lip(lip), lport(lport), joinhost_lip(joinhost_lip), joinhost_lport(joinhost_lport),
  rimsg_recv_cb(rimsg_recv_cb), cmsg_recv_cb(cmsg_recv_cb),
  commer(id, lip, lport, boost::bind(&SDMNode::handle_recv, this, _1) ),
  sdm_master_id(-1)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
  
  if (join() )
    log_(ERROR, "join failed; \n" << to_str() )
}

SDMNode::~SDMNode() { log_(INFO, "destructed.") }

int SDMNode::close()
{
  int ret = commer.close();
  if (ret == 0)
    log_(INFO, "id= " << id << " closed.")
  
  return ret;
}

std::string SDMNode::to_str()
{
  std::stringstream ss;
  ss << "\t id= " << id << ", type= " << type << "\n"
     << "\t lip= " << lip << ", lport= " << lport << "\n"
     << "\t joinhost_lip= " << joinhost_lip << ", joinhost_lport= " << joinhost_lport << "\n";
  
  return ss.str();
}

int SDMNode::get_id() { return id; }
int SDMNode::get_num_peers() { return commer.get_num_peers(); }

int SDMNode::join()
{
  if (joinhost_lip.compare("") == 0) { // First node
    log_(INFO, "FIRST NODE.")
    return 0;
  }
  else {
    // To get check joinhost status and get the join_reply
    while (1) {
      if (commer.connect_send_to(joinhost_lip, joinhost_lport, *gen_join_req() ) ) {
        log_(ERROR, "commer.connect_send_to failed for initial join; joinhost_lip= " << joinhost_lip << ", joinhost_lport= " << joinhost_lport << "\n"
                   << "trying again in 1 sec...")
        usleep(1000000);
      }
      else
       break;
    }
    log_(INFO, "waiting for join...; id= " << id)
    syncer.add_sync_point(0, 1);
    syncer.wait(0);
    log_(INFO, "done waiting for join; id= " << id)
    syncer.del_sync_point(0);
    
    return 0;
  }
}

//--------------------------------------------  gen_packet  --------------------------------------//
boost::shared_ptr<Packet> SDMNode::gen_join_req()
{
  std::map<std::string, std::string> msg_map;
  msg_map["lip"] = lip;
  msg_map["lport"] = boost::lexical_cast<std::string>(lport);
  
  return gen_packet(PACKET_JOIN_REQUEST, msg_map);
}

boost::shared_ptr<Packet> SDMNode::gen_join_reply(int peer_id, bool pos)
{
  std::map<std::string, std::string> msg_map;
  msg_map["lip"] = lip;
  msg_map["lport"] = boost::lexical_cast<std::string>(lport);
  
  if (pos)
    msg_map["ack"] = "a";
  else
    msg_map["ack"] = "n";
  
  if (pos && !master_slave) {
    // Add other peers new peer should connect to
    int count = 0;
    patch::thread_safe_map<int, boost::shared_ptr<peer_info> >& peer_id__peer_info_map = commer.get_peer_id__peer_info_map();
    for (std::map<int, boost::shared_ptr<peer_info> >::iterator it = peer_id__peer_info_map.begin(); it != peer_id__peer_info_map.end(); it++) {
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
  return gen_packet(PACKET_JOIN_REPLY, msg_map);
}

boost::shared_ptr<Packet> SDMNode::gen_packet(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{
  msg_map["id"] = boost::lexical_cast<std::string>(id);
  msg_map["node_type"] = type;
  
  return boost::make_shared<Packet>(packet_t, msg_map);
}

//----------------------------------------  messaging  -------------------------------------------//
int SDMNode::send_msg_to_master(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{
  if (sdm_master_id != -1)
    return send_msg(sdm_master_id, packet_t, msg_map);
  else {
    log_(ERROR, "master does not exist!")
    return 1;
  }
}

int SDMNode::broadcast_msg(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{ return commer.send_to_all_peers(' ', *gen_packet(packet_t, msg_map) ); }

int SDMNode::broadcast_msg_to_slaves(PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{ return commer.send_to_all_peers(sdm_master_id, *gen_packet(packet_t, msg_map) ); }

int SDMNode::send_msg(int to_id, PACKET_T packet_t, std::map<std::string, std::string> msg_map)
{ return commer.send_to_peer(to_id, *gen_packet(packet_t, msg_map) ); }

//-----------------------------------------  handle_  --------------------------------------------//
void SDMNode::handle_recv(char* type__srlzed_msg_map)
{
  boost::shared_ptr<Packet> p_ = boost::make_shared<Packet>((int)strlen(type__srlzed_msg_map), type__srlzed_msg_map);
  // log_(INFO, "p= \n" << p_->to_str() )
  
  switch (p_->get_type() ) {
    case PACKET_JOIN_REQUEST:
      handle_join_req(p_->get_msg_map() );
      break;
    case PACKET_JOIN_REPLY:
      handle_join_reply(p_->get_msg_map() );
      break;
    case PACKET_JOIN_ACK: // Note: Used to provide conn_up semantics for sdm_control
      if (cmsg_recv_cb != NULL)
        cmsg_recv_cb(p_);
      break;
    case PACKET_JOIN_NACK:
      handle_join_nack(p_->get_msg_map() );
      break;
    case PACKET_PING:
      handle_ping(p_->get_msg_map() );
      break;
    case PACKET_PONG:
      break;
    case PACKET_RIMSG:
      rimsg_recv_cb(p_->get_msg_map() );
      break;
    case PACKET_CMSG:
      cmsg_recv_cb(p_);
      break;
  }
  // 
  free(type__srlzed_msg_map);
}

void SDMNode::handle_join_req(std::map<std::string, std::string> msg_map)
{
  int from_id = boost::lexical_cast<int>(msg_map["id"] );
  std::string from_lip = msg_map["lip"];
  int from_lport = boost::lexical_cast<int>(msg_map["lport"] );
  
  if (commer.add_peer(from_id, "peer_" + boost::lexical_cast<std::string>(from_id), from_lip, from_lport) ) {
    log_(ERROR, "commer.add_peer failed; peer_id= " << from_id)
    commer.connect_send_to(from_lip, from_lport, *gen_join_reply(from_id, false) );
  }
  else {
    log_(INFO, "new peer added; peer_id= " << from_id << ", lip= " << from_lip << ", lport= " << from_lport << "\n"
               << "commer= \n" << commer.to_str() )
    commer.send_to_peer(from_id, *gen_join_reply(from_id, true) );
  }
}

void SDMNode::handle_join_reply(std::map<std::string, std::string> msg_map)
{
  if (msg_map["ack"].compare("a") == 0) {
    int from_id = boost::lexical_cast<int>(msg_map["id"] );
    
    if (msg_map["node_type"].compare("m") == 0) {
      if (sdm_master_id != -1)
        log_(WARNING, "from_id= " << from_id << " overwriting sdm_master_id= " << sdm_master_id)
      sdm_master_id = from_id;
    }
    std::string from_lip = msg_map["lip"];
    int from_lport = boost::lexical_cast<int>(msg_map["lport"] );
    // 
    if (commer.add_peer(from_id, "peer_" + boost::lexical_cast<std::string>(from_id), from_lip, from_lport) ) {
      commer.send_to_peer(from_id, *gen_packet(PACKET_JOIN_NACK) );
      log_(ERROR, "add_peer failed; peer_id= " << from_id)
    }
    else {
      commer.send_to_peer(from_id, *gen_packet(PACKET_JOIN_ACK) );
      log_(INFO, "joined :) to peer_id= " << from_id)
      syncer.notify(0);
      
      log_(INFO, "commer= \n" << commer.to_str() )
      commer.send_to_peer(from_id, *gen_packet(PACKET_PING) );
      // Join others
      int count = 0;
      while (1) {
        std::string key_tail_str = boost::lexical_cast<std::string>(count);
        if (msg_map.count("id_" + key_tail_str) == 0)
          break;
        
        int ppeer_id = boost::lexical_cast<int>(msg_map["id_" + key_tail_str] );
        if (commer.is_peer(ppeer_id) )
          continue;
        
        std::string ppeer_lip = msg_map["lip_" + key_tail_str];
        int ppeer_lport = boost::lexical_cast<int>(msg_map["lport_" + key_tail_str] );
        
        if (commer.connect_send_to(ppeer_lip, ppeer_lport, *gen_join_req() ) )
          log_(ERROR, "commer.connect_send_to failed for join; ppeer_lip= " << ppeer_lip << ", ppeer_lport= " << ppeer_lport)
        
        count++;
      }
    }
  }
  else
    log_(INFO, "could not join :(")
}

void SDMNode::handle_join_nack(std::map<std::string, std::string> msg_map)
{
  int from_id = boost::lexical_cast<int>(msg_map["id"] );
  // With join_req recved, we connected to it, now we learned things did not work out on the other side
  if (commer.rm_peer(from_id) )
    log_(ERROR, "commer.rm_peer failed; peer_id= " << from_id)
  
  log_(INFO, "commer= \n" << commer.to_str() )
}

void SDMNode::handle_ping(std::map<std::string, std::string> msg_map)
{
  int from_id = boost::lexical_cast<int>(msg_map["id"] );
  commer.send_to_peer(from_id, *gen_packet(PACKET_PONG) );
}
