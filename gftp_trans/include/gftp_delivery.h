#ifndef _GFTP_DELIVERY_H_
#define _GFTP_DELIVERY_H_

#include <boost/make_shared.hpp>

#include "gftp_drive.h"
#include "io_drive.h"

class GFTPDDManager
{
  private:
    std::string tmpfs_dir;
    boost::shared_ptr<IODriver> io_driver_;
    boost::shared_ptr<GFTPDriver> gftp_driver_;
  public:
    GFTPDDManager(std::string tmpfs_dir);
    ~GFTPDDManager();
    
    int init_gftp_server(int port);
    int put_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                      std::string key, unsigned int ver,
                      size_t datasize_inB, void* data_);
    int get_over_gftp(std::string s_laddr, std::string s_lport, std::string s_tmpfs_dir,
                      std::string key, unsigned int ver, 
                      size_t &datasize_inB, void* &data_); //returns datasize_inB
};

#endif //end of _GFTP_DELIVERY_H_