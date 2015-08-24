#include "gftp_trans.h"

GFTPTManager::GFTPTManager(std::string s_lintf, std::string s_laddr, int s_lport, std::string tmpfs_dir)
: s_lintf(s_lintf), s_laddr(s_laddr), s_lport(s_lport), tmpfs_dir(tmpfs_dir),
  io_driver_(boost::make_shared<IODriver>(tmpfs_dir) ),
  gftp_driver_(boost::make_shared<GFTPDriver>() )
{
  // 
  LOG(INFO) << "GFTPTManager:: constructed.";
}

GFTPTManager::~GFTPTManager() { LOG(INFO) << "GFTPTManager:: destructed."; }

int GFTPTManager::get_s_lport() { return s_lport; }

int GFTPTManager::init_server()
{
  return gftp_driver_->init_server(s_lintf, s_lport);
}

int GFTPTManager::put(std::string s_laddr, int s_lport, std::string tmpfs_dir,
                      std::string data_id, size_t datasize_inB, void* data_)
{
  std::string fname = "/ds_" + data_id + ".dat";
  if (io_driver_->write_file("", fname, datasize_inB, data_) ) {
    LOG(ERROR) << "put_over_gftp:: io_driver_->write_file failed!";
    return 1;
  }
  std::string src_url = tmpfs_dir + fname;
  std::string dst_url = "ftp://" + s_laddr + ":" + boost::lexical_cast<std::string>(s_lport) + tmpfs_dir + fname;
  
  if (gftp_driver_->put_file(src_url, dst_url) ) {
    LOG(ERROR) << "put_over_gftp:: gftp_driver_->put_file failed!";
    return 1;
  }
  // 
  LOG(INFO) << "put_over_gftp:: done for data_id= " << data_id << " to dst_url= " << dst_url;
  return 0;
}

int GFTPTManager::get(std::string s_laddr, int s_lport, std::string tmpfs_dir,
                      std::string data_id, size_t &datasize_inB, void* &data_)
{
  std::string fname = "/ds_" + data_id + ".dat";
  std::string src_url = "ftp://" + s_laddr + ":" + boost::lexical_cast<std::string>(s_lport) + tmpfs_dir + fname;
  std::string dst_url = tmpfs_dir + fname;
  
  if (gftp_driver_->get_file(src_url, dst_url) ) {
    LOG(ERROR) << "get_over_gftp:: gftp_driver_->get_file failed!";
    return 1;
  }
  
  datasize_inB = io_driver_->read_file("", fname, data_);
  // 
  LOG(INFO) << "get_over_gftp:: done for data_id= " << data_id << " from src_url= " << src_url;
  return 0;
}

int GFTPTManager::read_del_datafile(std::string data_id, size_t &datasize_inB, void* &data_)
{
  std::string fname = "/ds_" + data_id + ".dat";
  
  datasize_inB = io_driver_->read_file("", fname, data_);
  
  // Del fname
  
  // 
  LOG(INFO) << "read_del_datafile:: done for data_id= " << data_id;
  return 0;
}