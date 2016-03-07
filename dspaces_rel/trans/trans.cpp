#include "trans.h"

Trans::Trans(std::string trans_protocol,
             std::string ib_lip, std::list<std::string> ib_lport_list,
             std::string tcp_lip, int tcp_lport,
             std::string gftp_lintf, std::string gftp_lip, std::string gftp_lport, std::string tmpfs_dir)
: trans_protocol(trans_protocol)
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    ib_trans_ = boost::make_shared<IBTrans>(ib_lip, ib_lport_list);
  else if (str_str_equals(trans_protocol, TCP) )
    tcp_trans_ = boost::make_shared<TCPTrans>(tcp_lip, tcp_lport);
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    gftp_trans_ = boost::make_shared<GFTPTrans>(gftp_lintf, gftp_lip, boost::lexical_cast<int>(gftp_lport), tmpfs_dir);
    if (gftp_trans_->init_server() ) {
      log_(ERROR, "gftp_trans_->init_server failed!")
      exit(1);
    }
  }
#endif // _GRIDFTP_
  else {
    log_(ERROR, "unknown trans_protocol= " << trans_protocol)
    exit(1);
  }
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

Trans::~Trans() { log_(INFO, "destructed.";) }

std::string Trans::to_str()
{
  std::stringstream ss;
  ss << "trans_protocol= " << trans_protocol << "\n";
  
  if (str_str_equals(trans_protocol, INFINIBAND) )
    ss << "ib_trans= \n" << ib_trans_->to_str() << "\n";
  else if (str_str_equals(trans_protocol, TCP) )
    ss << "tcp_trans= \n" << tcp_trans_->to_str() << "\n";
  #ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) )
    ss << "gftp_trans= \n" << gftp_trans_->to_str() << "\n";
  #endif // _GRIDFTP_
     
  return ss.str();
}

std::string Trans::get_s_lip()
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    return ib_trans_->get_s_lip();
  else if (str_str_equals(trans_protocol, TCP) )
    return tcp_trans_->get_s_lip();
  #ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) )
    return gftp_trans_->get_s_lip();
  #endif // _GRIDFTP_
}

std::string Trans::get_s_lport()
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    return ib_trans_->get_s_lport();
  else if (str_str_equals(trans_protocol, TCP) )
    return boost::lexical_cast<std::string>(tcp_trans_->get_s_lport() );
  #ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) )
    return gftp_trans_->get_s_lport();
  #endif // _GRIDFTP_
}

std::string Trans::get_tmpfs_dir()
{ 
  #ifdef _GRIDFTP_
    return gftp_trans_->get_tmpfs_dir();
  #endif // _GRIDFTP_
  return "";
}

// Note: should block until get is finished
// int Trans::init_get(std::string s_lport, std::string data_id, data_recv_cb_func data_recv_cb)
int Trans::init_get(std::string data_type, std::string s_lport, std::string data_id, data_recv_cb_func data_recv_cb)
{
  if (str_str_equals(trans_protocol, INFINIBAND) ) {
    // ib_trans_->init_server(s_lport.c_str(), data_recv_cb);
    ib_trans_->init_server(data_type, s_lport.c_str(), data_id, data_recv_cb);
    ib_trans_->return_s_lport(s_lport);
  }
  else if (str_str_equals(trans_protocol, TCP) ) {
    tcp_trans_->init_server(data_id, data_recv_cb);
  }
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    int datasize_inB;
    void* data_;
    if (gftp_trans_->read_del_datafile(data_id, datasize_inB, data_) ) {
      log_(ERROR, "gftp_trans_->read_del_datafile failed; data_id= " << data_id)
      return 1;
    }
    data_recv_cb(data_id, datasize_inB, data_);
  }
#endif // _GRIDFTP_
  
  return 0;
}

int Trans::init_put(std::string s_lip, std::string s_lport, std::string tmpfs_dir,
                    std::string data_type, std::string data_id, uint64_t data_length, void* data_)
{
  if (str_str_equals(trans_protocol, INFINIBAND) ) {
    // char s_lip_[32];
    // memcpy(s_lip_, s_lip.c_str(), 32);
    // char s_lport_[16];
    // memcpy(s_lport_, s_lport.c_str(), 16);
    // log_(INFO, "s_lip_= " << s_lip_ << ", s_lport_= " << s_lport_)
    // ib_trans_->init_client(s_lip_, s_lport_, data_id, data_length, data_);
    ib_trans_->init_client(s_lip.c_str(), s_lport.c_str(), data_type, data_length, data_);
  }
  else if (str_str_equals(trans_protocol, TCP) ) {
    // Note: data_length should be data_size for TCP
    int data_size = data_length;
    if (data_type == "char")
      data_size *= sizeof(char);
    else if (data_type == "int")
      data_size *= sizeof(int);
    else if (data_type == "float")
      data_size *= sizeof(float);
    else if (data_type == "double")
      data_size *= sizeof(double);
    else {
      log_(ERROR, "unknown data_type= " << data_type)
      return 1;
    }
    tcp_trans_->send(s_lip, boost::lexical_cast<int>(s_lport), data_id, data_size, data_);
  }
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    if (gftp_trans_->put(s_lip, boost::lexical_cast<int>(s_lport), tmpfs_dir, data_id, data_length, data_) ) {
      log_(ERROR, "gftp_trans_->put failed; data_id= " << data_id)
      return 1;
    }
  }
  free(data_);
#endif // _GRIDFTP_
    
  return 0;
}