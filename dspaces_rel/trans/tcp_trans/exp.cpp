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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include "tcp_trans.h"

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
  int c;
  
  static struct option long_options[] =
  {
    {"type", required_argument, NULL, 0},
    {"lintf", required_argument, NULL, 1},
    {"lport", required_argument, NULL, 2},
    {"s_lip", required_argument, NULL, 3},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "",
                     long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["lintf"] = optarg;
        break;
      case 2:
        opt_map["lport"] = optarg;
        break;
      case 3:
        opt_map["s_lip"] = optarg;
        break;
      default:
        break;
    }
  }
  if (optind < argc) {
    std::cout << "Non-option ARGV-elements: \n";
    while (optind < argc)
      std::cout << argv[optind++] << ", ";
    std::cout << "\n";
  }
  // 
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  
  return opt_map;

}

typedef char DATA_T;
const int data_length = 1000;

int recved_size = 0;
void handle_recv(std::string data_id, int chunk_size, void* chunk_)
{
  recved_size += chunk_size;
  
  std::cout << "handle_recv:: data_id= " << data_id << ", chunk_size= " << chunk_size << "B, recved_size= " << recved_size << "\n";
            // << ", chunk_= " << patch::arr_to_str<>(chunk_size/sizeof(DATA_T), static_cast<DATA_T*>(chunk_) ) << "\n";
  free(chunk_);
}

int main (int argc, char **argv)
{
  google::InitGoogleLogging("exp");
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  std::string temp;
  // 
  if (str_cstr_equals(opt_map["type"], "server") ) {
    TCPTrans tcp_trans(intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ) );
    tcp_trans.init_server("dummy", boost::bind(&handle_recv, _1, _2, _3) );
    tcp_trans.init_server("dummy_1", boost::bind(&handle_recv, _1, _2, _3) );
    
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "client") ) {
    char alphabet_[] = {'a', 'b', 'c', 'd'};
    int alphabet_size = sizeof(alphabet_) / sizeof(*alphabet_);
    
    DATA_T* data_ = (DATA_T*)malloc(sizeof(DATA_T)*data_length);
    for (int i = 0; i < data_length; i++)
      data_[i] = alphabet_[i % alphabet_size];
      // data_[i] = i;
    
    TCPTrans tcp_trans(intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ) + 1);
    tcp_trans.send(opt_map["s_lip"], boost::lexical_cast<int>(opt_map["lport"] ),
                  "dummy", data_length*sizeof(DATA_T), data_);
    tcp_trans.send(opt_map["s_lip"], boost::lexical_cast<int>(opt_map["lport"] ),
                  "dummy_1", data_length*sizeof(DATA_T), data_);
    
    free(data_);
    
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  
  return 0;
}
