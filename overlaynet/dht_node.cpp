#include "dht_node.h"
#include <glog/logging.h>

DHTNode::DHTNode(char* lip, int lport)
:  server_( new DHTServer(lip, lport, NULL) )
//: fp_handle_recv( boost::bind(&DHTNode::handle_recv, this, _1) ),
//  server_( new DHTServer(lip, lport, fp_handle_recv) )
{
  this->fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
  server_->set_recv_callback(fp_handle_recv);
  //
  
  //
  LOG(INFO) << "constructed.";
}

DHTNode::~DHTNode()
{
  server_ -> close();
  //
  LOG(INFO) << "destructed.";
}

void DHTNode::handle_recv(char* type__srlzedmsgmap)
{
  LOG(INFO) << "type__srlzedmsgmap = " << type__srlzedmsgmap;
}
