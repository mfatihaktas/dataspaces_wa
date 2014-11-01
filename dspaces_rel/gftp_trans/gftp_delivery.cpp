#include "gftp_delivery.h"

GFTPDDManager::GFTPDDManager(std::string gftps_data_intf, int gftps_lport, std::string tmpfs_dir)
: gftps_data_intf(gftps_data_intf),
  gftps_lport(gftps_lport),
  tmpfs_dir( tmpfs_dir ),
  io_driver_( boost::make_shared<IODriver>(tmpfs_dir) ),
  gftp_driver_( boost::make_shared<GFTPDriver>() )
{
  //
  LOG(INFO) << "GFTPDDManager:: constructed.";
}

GFTPDDManager::~GFTPDDManager()
{
  //
  LOG(INFO) << "GFTPDDManager:: destructed.";
}

int GFTPDDManager::get_gftps_port()
{
  return gftps_lport;
}

int GFTPDDManager::init_gftp_server()
{
  return gftp_driver_->init_server(gftps_data_intf, gftps_lport);
}

int GFTPDDManager::put_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                                 std::string key, unsigned int ver,
                                 size_t datasize_inB, void* data_)
{
  std::string fname = "/ds_" + key + "_" + boost::lexical_cast<std::string>(ver) + ".dat";
  if (io_driver_->write_file("", fname, datasize_inB, data_) ) {
    LOG(ERROR) << "put_over_gftp:: io_driver_->write_file failed!";
    return 1;
  }
  std::string src_url = tmpfs_dir + fname;
  std::string dst_url = "ftp://" + s_laddr + ":" + s_lport + s_tmpfs_dir + fname;
  
  if (gftp_driver_->put_file(src_url, dst_url) ) {
    LOG(ERROR) << "put_over_gftp:: gftp_driver_->put_file failed!";
    return 1;
  }
  // 
  LOG(INFO) << "put_over_gftp:: done for <key= " << key << ", ver= " << ver << "> to dst_url= " << dst_url;
  return 0;
}

int GFTPDDManager::get_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                                 std::string key, unsigned int ver, 
                                 size_t &datasize_inB, void* &data_) //returns datasize_inB
{
  std::string fname = "/ds_" + key + "_" + boost::lexical_cast<std::string>(ver) + ".dat";
  std::string src_url = "ftp://" + s_laddr + ":" + s_lport + s_tmpfs_dir + fname;
  std::string dst_url = tmpfs_dir + fname;
  
  if (gftp_driver_->get_file(src_url, dst_url) ) {
    LOG(ERROR) << "get_over_gftp:: gftp_driver_->get_file failed!";
    return 1;
  }
  
  datasize_inB = io_driver_->read_file("", fname, data_);
  // 
  LOG(INFO) << "get_over_gftp:: done for <key= " << key << ", ver= " << ver << "> from src_url= " << src_url;
  return 0;
}

int GFTPDDManager::read_del_datafile(std::string key, unsigned int ver, size_t &datasize_inB, void* &data_)
{
  std::string fname = "/ds_" + key + "_" + boost::lexical_cast<std::string>(ver) + ".dat";
  
  datasize_inB = io_driver_->read_file("", fname, data_);
  
  // del fname
  
  // 
  LOG(INFO) << "read_del_datafile:: done for <key= " << key << ", ver= " << ver << ">";
  return 0;
}