#include <iostream>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <map>
#include <vector>
#include <signal.h>

#include "boost/thread.hpp"
#include <boost/lexical_cast.hpp>

#include <glog/logging.h>

typedef std::map<int, FILE*> port_sstream_map;
typedef std::map<std::string, FILE*> port_cstream_map;

class GridFTP
{
  public:
    static boost::condition_variable cv;
    
    //
    GridFTP();
    ~GridFTP();
    int init_server(int port);
    int init_file_transfer(std::string src_url, std::string dst_url, int p, int cc);
    void read_print_stream(std::string name, FILE* fp);
  private:
    port_sstream_map p_ss_map;
    port_cstream_map p_cs_map;
    std::vector<boost::shared_ptr<boost::thread> > read_print_stream_thread_v;
    boost::shared_ptr<boost::thread> hold_t_;
    
    boost::mutex m;
    //
    void hold();
    void wait_for_flag();
    void close();
};