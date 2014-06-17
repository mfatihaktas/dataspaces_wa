#include "dht_node.h"
#include <glog/logging.h>

boost::condition_variable DHTNode::cv;

void handle_signal(int signum)
{
  LOG(INFO) << "handle_signal:: recved signum=" << signum;
  DHTNode::cv.notify_one();
}

DHTNode::DHTNode(char id, char* lip, int lport,
                 char* ipeer_lip, int ipeer_lport )
: ninfo(id, lip, lport),
  join_channel(lip, lport, ipeer_lip, ipeer_lport),
  msger( this )
{
  function_recv_callback fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
  join_channel.set_recv_callback(fp_handle_recv);
  
  signal(SIGINT, handle_signal);
  //
  LOG(INFO) << "constructed.";
  //
  if ( ipeer_lip == NULL ){ //first node
    LOG(INFO) << "FIRST NODE";
  }
  else {
    join_channel.conn_to_neigh();
    boost::shared_ptr< Packet > p_ = msger.gen_join_req();
    //LOG(INFO) << "DHTNode:: p = \n" << p_->to_str();
    join_channel.send_to_neigh( *p_ );
  }
  //
  wait_for_flag();
}

DHTNode::~DHTNode()
{
  LOG(INFO) << "destructed.";
}

void DHTNode::handle_recv(char* type__srlzedmsgmap)
{
  size_t type__srlzedmsgmap_size =  strlen(type__srlzedmsgmap);
  LOG(INFO) << "type__srlzedmsgmap_size = " << type__srlzedmsgmap_size;
  LOG(INFO) << "type__srlzedmsgmap = " << type__srlzedmsgmap;
  
  
  boost::shared_ptr< Packet > p_ (
    new Packet(type__srlzedmsgmap_size, type__srlzedmsgmap)
  );
  
  switch (p_->get_type())
  {
    case JOIN_REQUEST:
      handle_join_request(*p_);
  }
  
  //
  delete type__srlzedmsgmap;
}

void DHTNode::handle_join_request(const Packet& p)
{
  //std::map<std::string, std::string> msg_map
  LOG(INFO) << "handle_join_request:: msg_map = \n" << p.to_str();
  
  return;
}

void DHTNode::wait_for_flag()
{
  LOG(INFO) << "wait_for_flag:: waiting...";
  boost::mutex::scoped_lock lock(m);
  cv.wait(lock);
  LOG(INFO) << "wait_for_flag:: done.";
}
