// for intf_to_ip
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <sys/time.h>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "ib_trans.h"

char* intf_to_ip(std::string intf)
{
  int fd;
  struct ifreq ifr;
  // 
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  // Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name, intf.c_str(), IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  // 
  return inet_ntoa( ( (struct sockaddr_in*)& ifr.ifr_addr )->sin_addr);
}

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"s_lport", optional_argument, NULL, 1},
    {"s_lip", optional_argument, NULL, 2},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s",
                     long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["s_lport"] = optarg;
        break;
      case 2:
        opt_map["s_lip"] = optarg;
        break;
      default:
        break;
    }
  }
  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc)
      printf("%s ", argv[optind++] );
    putchar('\n');
  }
  // 
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  
  return opt_map;
}

#define data_type int
const std::string data_type_str = "int";

void msg_recv_handler(uint64_t size, char* msg_)
{
  log_(INFO, "recved; size= " << size << ", msg_= " << msg_)
}

int total_recved_size = 0;
void data_recv_handler(std::string data_id, uint64_t data_size, void* data_)
{
  total_recved_size += data_size;
  log_(INFO, "data_recv_handler:: for data_id= " << data_id
             << ", recved data_size= " << data_size
             << ", total_recved_size= " << (float)total_recved_size/(1024*1024) << "MB \n")
            // << "data_= " << patch::arr_to_str<>(data_size, (data_type*)data_) )
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  struct timeval start_time;
  struct timeval end_time;
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  std::string ib_lports[] = {"1234","1235","1236","1237"};
  std::list<std::string> ib_lport_list(ib_lports, ib_lports + sizeof(ib_lports) / sizeof(std::string) );
  // 
  IBTrans ib_trans(opt_map["s_lip"], ib_lport_list);
  if (str_cstr_equals(opt_map["type"], "server") ) {
    ib_trans.init_server(ib_trans.get_s_lport().c_str(),
                         boost::bind(&data_recv_handler, _1, _2, _3),
                         boost::bind(&msg_recv_handler, _1, _2) );
  }
  else if (str_cstr_equals(opt_map["type"], "client") ) {
    uint64_t data_length = 1024; // 1024*1024*256; // 1024*1024*256;
    void* data_ = (void*)malloc(sizeof(data_type)*data_length);
    
    for (int i = 0; i < data_length; i++)
      static_cast<data_type*>(data_)[i] = (data_type)i*1.2;
    // 
    if (gettimeofday(&start_time, NULL) ) {
      log_(ERROR, "gettimeofday returned non-zero.")
      return 1;
    }
    
    ib_trans.init_client(opt_map["s_lip"].c_str(), opt_map["s_lport"].c_str(),
                         "dummy", data_length*sizeof(data_type), data_);
    
    if (gettimeofday(&end_time, NULL) ) {
      log_(ERROR, "gettimeofday returned non-zero.")
      return 1;
    }
    long exec_time_sec = end_time.tv_sec - start_time.tv_sec;
    long exec_time_usec = end_time.tv_usec - start_time.tv_usec;
    log_(INFO, "exec_time= " << exec_time_sec << "." << exec_time_usec / 1000 << " sec.")
    // 
    free(data_);
    // std::cout << "Enter\n";
    // getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "bqueue") ) {
    // BQueue<int> bq;
    // bq.create_timed_push_thread(12);
    
    // std::string ib_lport = ib_trans.get_next_avail_ib_lport();
    // log_(INFO, "ib_lport= " << ib_lport)
    // ib_lport = ib_trans.get_next_avail_ib_lport();
    // log_(INFO, "ib_lport= " << ib_lport)
  }
  else
     log_(ERROR, "unknown type= " << opt_map["type"] )

  return 0;
}