#ifndef _GFTP_DELIVERY_H_
#define _GFTP_DELIVERY_H_

#include <boost/make_shared.hpp>

#include "gftp_drive.h"
#include "io_drive.h"

class GFTPDDManager
{
  private:
    std::string gftps_data_intf;
    int gftps_lport;
    std::string tmpfs_dir;
    boost::shared_ptr<IODriver> io_driver_;
    boost::shared_ptr<GFTPDriver> gftp_driver_;
  public:
    GFTPDDManager(std::string gftps_data_intf, int gftps_lport, std::string tmpfs_dir);
    ~GFTPDDManager();
    int get_gftps_port();
    
    int init_gftp_server();
    int put_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                      std::string key, unsigned int ver,
                      size_t datasize_inB, void* data_);
    int get_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                      std::string key, unsigned int ver, 
                      size_t &datasize_inB, void* &data_); //returns datasize_inB
    
    int read_del_datafile(std::string key, unsigned int ver, size_t &datasize_inB, void* &data_);
};

#endif //end of _GFTP_DELIVERY_H_