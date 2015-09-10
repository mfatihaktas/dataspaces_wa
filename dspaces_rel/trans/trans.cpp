#include "trans.h"

TManager::TManager(std::string trans_protocol,
                   std::string ib_laddr, std::list<std::string> ib_lport_list, 
                   std::string gftp_lintf, std::string gftp_laddr, std::string gftp_lport, std::string tmpfs_dir)
: trans_protocol(trans_protocol),
  ib_laddr(ib_laddr), gftp_laddr(gftp_laddr), gftp_lport(gftp_lport)
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    ibt_manager_ = boost::make_shared<IBTManager>(ib_lport_list);
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    gftpt_manager_ = boost::make_shared<GFTPTManager>(gftp_lintf, gftp_laddr, boost::lexical_cast<int>(gftp_lport), tmpfs_dir);
    if (gftpt_manager_->init_server() ) {
      LOG(ERROR) << "TManager:: gftpt_manager_->init_server failed!";
      exit(1);
    }
  }
#endif // _GRIDFTP_
  else {
    LOG(ERROR) << "TManager:: unknown trans_protocol= " << trans_protocol;
    exit(1);
  }
  // 
  LOG(INFO) << "TManager:: constructed.";
}

TManager::~TManager() { LOG(INFO) << "TManager:: destructed."; }

std::string TManager::to_str()
{
  std::stringstream ss;
  ss << "\t trans_protocol= " << trans_protocol << "\n"
     << "\t ibt_manager= \n" << ibt_manager_->to_str() << "\n"
  #ifdef _GRIDFTP_
     << "\t gftpt_manager= \n" << gftpt_manager_->to_str() << "\n"
  #endif // _GRIDFTP_
     << "";
     
  return ss.str();
}

std::string TManager::get_s_laddr()
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    return ib_laddr;
  else if (str_str_equals(trans_protocol, GRIDFTP) )
    return gftp_laddr;
}

std::string TManager::get_s_lport()
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    return ibt_manager_->get_next_avail_ib_lport();
  else if (str_str_equals(trans_protocol, GRIDFTP) )
    return gftp_lport;
}

std::string TManager::get_tmpfs_dir()
{ 
  #ifdef _GRIDFTP_
    return gftpt_manager_->get_tmpfs_dir();
  #endif // _GRIDFTP_
  return "";
}

int TManager::init_get(std::string s_lport, std::string data_id, std::string data_type, data_recv_cb_func recv_cb)
{
  if (str_str_equals(trans_protocol, INFINIBAND) ) {
    ibt_manager_->init_ib_server(s_lport.c_str(), recv_cb, data_type, data_id);
    ibt_manager_->give_ib_lport_back(s_lport);
  }
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    int datasize_inB;
    void* data_;
    if (gftpt_manager_->read_del_datafile(data_id, datasize_inB, data_) ) {
      LOG(ERROR) << "init_get:: gftpt_manager_->read_del_datafile failed!";
      return 1;
    }
    recv_cb(data_id, datasize_inB, data_);
  }
#endif // _GRIDFTP_
  
  return 0;
}

int TManager::init_put(std::string s_laddr, std::string s_lport, std::string tmpfs_dir,
                       std::string data_id, std::string data_type, int data_length, void* data_)
{
  if (str_str_equals(trans_protocol, INFINIBAND) )
    ibt_manager_->init_ib_client(s_laddr.c_str(), s_lport.c_str(), data_type, data_length, data_);
#ifdef _GRIDFTP_
  else if (str_str_equals(trans_protocol, GRIDFTP) ) {
    if (gftpt_manager_->put(s_laddr, boost::lexical_cast<int>(s_lport), tmpfs_dir, data_id, data_length, data_) ) {
      LOG(ERROR) << "init_put:: gftpt_manager_->put for data_id= " << data_id <<" failed!";
      return 1;
    }
  }
  free(data_);
#endif // _GRIDFTP_
    
  return 0;
}