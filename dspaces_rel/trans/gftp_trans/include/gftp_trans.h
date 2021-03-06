#ifndef _GFTP_DELIVERY_H_
#define _GFTP_DELIVERY_H_

#include <boost/make_shared.hpp>

#include "gftp_drive.h"
#include "io_drive.h"

class GFTPTrans
{
  private:
    std::string s_lintf, s_lip;
    int s_lport;
    std::string tmpfs_dir;
    boost::shared_ptr<IODriver> io_driver_;
    boost::shared_ptr<GFTPDriver> gftp_driver_;
  public:
    GFTPTrans(std::string s_lintf, std::string s_lip, int s_lport, std::string tmpfs_dir);
    ~GFTPTrans();
    std::string to_str();
    
    std::string get_s_lip();
    int get_s_lport();
    std::string get_tmpfs_dir();
    
    int init_server();
    int put(std::string s_lip, int s_lport, std::string tmpfs_dir,
            std::string data_id, int datasize_inB, void* data_);
    int get(std::string s_lip, int s_lport, std::string tmpfs_dir,
            std::string data_id, int &datasize_inB, void* &data_);
    
    int read_del_datafile(std::string data_id, int &datasize_inB, void* &data_);
};

#endif //end of _GFTP_DELIVERY_H_