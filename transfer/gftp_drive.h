#ifndef _GFTP_DRIVE_H_
#define _GFTP_DRIVE_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "boost/thread.hpp"
#include <boost/lexical_cast.hpp>

#include <glog/logging.h>
extern "C" {
  #include "gridftp_api_drive.h"
}

class GFtpDriver
{
  public:
    GFtpDriver();
    ~GFtpDriver();
    
    void close();
    
    int init_server(int port);
    void read_print_stream(std::string name, FILE* fp);
    //int init_file_transfer(std::string src_url, std::string dst_url, int p, int cc);
    //void close();
  private:
    std::vector<boost::shared_ptr<boost::thread> > read_print_stream_thread_v;
    std::map<int, boost::shared_ptr<FILE> > port_serverfp_map;
};


#endif //end of _GFTP_DRIVE_H_