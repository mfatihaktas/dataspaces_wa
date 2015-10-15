// for intf_to_ip
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
// 
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <sys/time.h>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
// 
#include "ib_trans.h"

char* intf_to_ip(const char* intf)
{
  int fd;
  struct ifreq ifr;
  // 
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  // Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name , intf , IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  // 
  return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
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
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  // 
  std::cout << "opt_map= \n";
  for (std::map<std::string, std::string>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it)
    std::cout << it->first << " : " << it->second << '\n';
  
  return opt_map;
}

#define data_type int
const std::string data_type_str = "int";

size_t total_recved_size = 0;
void data_recv_handler(std::string recv_id, size_t data_size, void* data_)
{
  total_recved_size += data_size;
  LOG(INFO) << "data_recv_handler:: for recv_id= " << recv_id
            << ", recved data_size= " << data_size
            << ", total_recved_size= " << (float)total_recved_size/(1024*1024) << "MB";
  
  // size_t length = data_size/sizeof(data_type);
  // for (int i = 0; i < length; i++){
  //   std::cout << static_cast<data_type*>(data_)[i] << ",";
  // }
  // std::cout << "\n";
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
  if (str_equals(opt_map["type"], "server") ) {
    ib_trans.init_server(data_type_str, ib_trans.get_s_lport().c_str(),
                         "dummy", boost::bind(&data_recv_handler, _1, _2, _3) );
  }
  else if (str_equals(opt_map["type"], "client") ) {
    size_t data_length = 1024*1024*256; //1024*1024*256;
    void* data_ = (void*)malloc(sizeof(data_type)*data_length);
    
    for (int i = 0; i < data_length; i++)
      static_cast<data_type*>(data_)[i] = (data_type)i*1.2;
    // 
    if (gettimeofday(&start_time, NULL) ) {
      LOG(ERROR) << "main:: gettimeofday returned non-zero.";
      return 1;
    }
    
    ib_trans.init_client(opt_map["s_lip"].c_str(), opt_map["s_lport"].c_str(),
                         data_type_str, data_length, data_);
    
    if (gettimeofday(&end_time, NULL) ) {
      LOG(ERROR) << "main:: gettimeofday returned non-zero.";
      return 1;
    }
    long exec_time_sec = end_time.tv_sec - start_time.tv_sec;
    long exec_time_usec = end_time.tv_usec - start_time.tv_usec;
    LOG(INFO) << "main:: exec_time= " << exec_time_sec << "." << exec_time_usec / 1000 << " sec.";
    // 
    free(data_);
    // std::cout << "Enter\n";
    // getline(std::cin, temp);
  }
  else if (str_equals(opt_map["type"], "bqueue") ) {
    // BQueue<int> bq;
    // bq.create_timed_push_thread(12);
    
    // std::string ib_lport = ib_trans.get_next_avail_ib_lport();
    // LOG(INFO) << "main:: ib_lport= " << ib_lport;
    // ib_lport = ib_trans.get_next_avail_ib_lport();
    // LOG(INFO) << "main:: ib_lport= " << ib_lport;
  }
  else
     LOG(ERROR) << "main:: unknown type= " << opt_map["type"];

  return 0;
}