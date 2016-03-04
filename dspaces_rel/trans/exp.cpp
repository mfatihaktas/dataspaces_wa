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
// 
#include "trans.h"

std::string intf_to_ip(std::string intf)
{
  int fd;
  struct ifreq ifr;
  // 
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  // Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name, intf.c_str(), IFNAMSIZ - 1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  // 
  return boost::lexical_cast<std::string>(inet_ntoa( ( (struct sockaddr_in*)&ifr.ifr_addr)->sin_addr) );
}

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"trans_protocol", optional_argument, NULL, 1},
    {"ib_lintf", optional_argument, NULL, 2},
    {"tcp_lintf", optional_argument, NULL, 3},
    {"gftp_lintf", optional_argument, NULL, 4},
    {"tmpfs_dir", optional_argument, NULL, 5},
    {"s_lip", optional_argument, NULL, 6},
    {"s_lport", optional_argument, NULL, 7},
    {0, 0, 0, 0}
  };
  
  while (1) {
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
        opt_map["trans_protocol"] = optarg;
        break;
      case 2:
        opt_map["ib_lintf"] = optarg;
        break;
      case 3:
        opt_map["tcp_lintf"] = optarg;
        break;
      case 4:
        opt_map["gftp_lintf"] = optarg;
        break;
      case 5:
        opt_map["tmpfs_dir"] = optarg;
        break;
      case 6:
        opt_map["s_lip"] = optarg;
        break;
      case 7:
        opt_map["s_lport"] = optarg;
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
      printf ("%s ", argv[optind++] );
    putchar ('\n');
  }
  // 
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  
  return opt_map;
}

int total_recved_size = 0;
void data_recv_handler(std::string data_id, uint64_t data_size, void* data_)
{
  total_recved_size += data_size;
  log_(INFO, "for data_id= " << data_id
             << ", recved data_size= " << data_size
             << ", total_recved_size= " << (float)total_recved_size/(1024*1024) << "MB")
}

#define DATA_T int
const std::string DATA_T_STR = "int";

int main(int argc , char **argv)
{
  int err;
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  struct timeval start_time;
  struct timeval end_time;
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  std::string ib_lports[] = {"1234", "1235", "1236", "1237"};
  std::list<std::string> ib_lport_list(ib_lports, ib_lports + sizeof(ib_lports)/sizeof(*ib_lports) );
  std::string gftp_lport = "1234";
  // 
  Trans trans(opt_map["trans_protocol"],
              intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
              intf_to_ip(opt_map["tcp_lintf"] ), boost::lexical_cast<int>(opt_map["s_lport"] ),
              opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), gftp_lport, opt_map["tmpfs_dir"] );
  if (str_cstr_equals(opt_map["type"], "get") ) {
    trans.init_get(trans.get_s_lport(), "dummy", boost::bind(&data_recv_handler, _1, _2, _3) );
    
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "put") ) {
    // int data_length = 4* 1024*1024*256;
    int data_length = 1024; //1024*1024*256;
    void* data_ = (void*)malloc(sizeof(DATA_T)*data_length);
    
    for (int i = 0; i < data_length; i++)
      static_cast<DATA_T*>(data_)[i] = (DATA_T)i*1.2;
    // 
    return_if_err(gettimeofday(&start_time, NULL), err)
    trans.init_put(opt_map["s_lip"], opt_map["s_lport"], opt_map["tmpfs_dir"],
                   DATA_T_STR, "dummy", data_length, data_);
    return_if_err(gettimeofday(&end_time, NULL), err)
    
    long exec_time_sec = end_time.tv_sec - start_time.tv_sec;
    long exec_time_usec = end_time.tv_usec - start_time.tv_usec;
    log_(INFO, "exec_time= " << exec_time_sec << "." << exec_time_usec / 1000 << " sec.")
    // 
    free(data_);
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else {
     log_(ERROR, "unknown type= " << opt_map["type"] )
  }
  
  return 0;
}