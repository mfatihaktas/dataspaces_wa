#include "dht_node.h"
#include <glog/logging.h>

boost::condition_variable DHTNode::cv;

void handle_signal(int signum)
{
  LOG(INFO) << "handle_signal:: recved signum=" << signum;
  DHTNode::cv.notify_one();
}

DHTNode::DHTNode(char* lip, int lport)
: server_( new DHTServer(lip, lport, NULL) )
{
  this->fp_handle_recv = boost::bind(&DHTNode::handle_recv, this, _1);
  server_->set_recv_callback(fp_handle_recv);
  signal(SIGINT, handle_signal);
  //
  LOG(INFO) << "constructed.";
  wait_for_flag();
}

DHTNode::~DHTNode()
{
  server_ -> close();
  LOG(INFO) << "destructed.";
}

void DHTNode::handle_recv(char* type__srlzedmsgmap)
{
  LOG(INFO) << "type__srlzedmsgmap = " << type__srlzedmsgmap;
}

void DHTNode::wait_for_flag()
{
  LOG(INFO) << "wait_for_flag:: waiting...";
  boost::mutex::scoped_lock lock(m);
  cv.wait(lock);
  LOG(INFO) << "wait_for_flag:: done.";
}
