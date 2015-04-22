// for intf_to_ip
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
// for boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <glog/logging.h>

#include "tcp_client.h"
#include "tcp_server.h"

char* intf_to_ip(char* intf)
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
  return inet_ntoa( ( (struct sockaddr_in*)& ifr.ifr_addr )->sin_addr);
}

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
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
    c = getopt_long (argc, argv, "s",
                     long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map[(char*)"type"] = optarg;
        break;
      case 1:
        opt_map[(char*)"lintf"] = optarg;
        break;
      case 2:
        opt_map[(char*)"lport"] = optarg;
        break;
      case 3:
        opt_map[(char*)"s_lip"] = optarg;
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
  std::cout << "opt_map= \n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it) {
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;

}
void handle_recv(std::string key, unsigned int ver, size_t chunk_length, void* chunk_)
{
  std::cout << "handle_recv:: <key= " << key << ", ver= " << ver << ">, chunk_length= " << chunk_length 
            << ", chunk_= " << boost::lexical_cast<std::string>(static_cast<char*>(chunk_) ) << "\n";
  free(chunk_);
}

int main (int argc, char **argv)
{
  google::InitGoogleLogging("exp");
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  std::string temp;
  // 
  
  if (strcmp(opt_map[(char*)"type"], (char*)"server") == 0) {
    TCPServer<char> tcp_server("dummy", 0, intf_to_ip(opt_map[(char*)"lintf"]), atoi(opt_map[(char*)"lport"]),
                               8, boost::bind(&handle_recv, _1, _2, _3, _4) );
  
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"client") == 0) {
    char alphabet_[] = {'a', 'b', 'c', 'd'};
    size_t alphabet_size = sizeof(alphabet_) / sizeof(*alphabet_);
    
    size_t data_length = 100; //100*1024*1024;
    char* data_ = (char*)malloc(sizeof(char)*data_length);
    for (int i = 0; i < data_length; i++) {
      data_[i] = alphabet_[i % alphabet_size];
    }
    
    TCPClient<char> tcp_client((char*)"client", opt_map[(char*)"s_lip"], atoi(opt_map[(char*)"lport"]),
                               data_length, data_);
    tcp_client.init();
    free(data_);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  
  //std::cout << "opt_map[(char*)\"intf\"]=" << opt_map[(char*)"intf"] << std::endl;
  
  //char* ip = intf_to_ip((char*)"lo");
  //std::cout << "ip=" << ip << std::endl;
  
  return 0;
}
