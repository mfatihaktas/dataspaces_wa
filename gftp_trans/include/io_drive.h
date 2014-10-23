#ifndef _IO_DRIVE_H_
#define _IO_DRIVE_H_

#include <iostream>
#include <fstream>
#include <string>

// #include "boost/thread.hpp"
// #include <boost/lexical_cast.hpp>

#include <glog/logging.h>

class IODrive
{
  public:
    IODrive(std::string working_dir);
    ~IODrive();
    std::string to_str();
    
    int write_file(std::string another_working_dir, std::string file_name, int datasize_inB, void* data_);
    int read_file(std::string another_working_dir, std::string file_name, void* &data_); //returns size_inB of data read from file
  private:
    std::string working_dir;
    // std::vector<boost::shared_ptr<boost::thread> > read_print_stream_thread_v;
    // std::map<int, boost::shared_ptr<FILE> > port_serverfp_map;
};


#endif //end of _IO_DRIVE_H_