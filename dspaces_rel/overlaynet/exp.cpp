//for intf_to_ip
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
//for boost serialization
#include <fstream>
#include <sstream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
//
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
//for exp
#include <glog/logging.h>
#include "dht_server.h"
#include "packet.h"
#include "dht_node.h"

void handle_read(char* data)
{
  std::cout << "handle_read; data=\n" << data << std::endl;
  delete data;
}

void handle_msg(std::map<std::string, std::string> wamsg_map)
{
  std::cout << "handle_msg; msg_map=..." << std::endl;
}

char* intf_to_ip(char* intf)
{
  int fd;
  struct ifreq ifr;
  //
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  //Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  //Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name , intf , IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  //
  return inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
}

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"id", required_argument, NULL, 0},
    {"intf", required_argument, NULL, 1},
    {"lport", required_argument, NULL, 2},
    {"ipeer_lip", required_argument, NULL, 3},
    {"ipeer_lport", required_argument, NULL, 4},
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
        opt_map[(char*)"id"] = optarg;
        break;
      case 1:
        opt_map[(char*)"intf"] = optarg;
        break;
      case 2:
        opt_map[(char*)"lport"] = optarg;
        break;
      case 3:
        opt_map[(char*)"ipeer_lip"] = optarg;
        break;
      case 4:
        opt_map[(char*)"ipeer_lport"] = optarg;
        break;
      case 's':
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc){
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  //
  std::cout << "opt_map=\n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int main (int argc, char **argv)
{
  google::InitGoogleLogging("exp");
  //
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  
  //std::cout << "opt_map[(char*)\"intf\"]=" << opt_map[(char*)"intf"] << std::endl;
  
  //char* ip = intf_to_ip((char*)"lo");
  //std::cout << "ip=" << ip << std::endl;
  if (!opt_map.count((char*)"ipeer_lip")){
    opt_map[(char*)"ipeer_lip"] = NULL;
  }
  if (!opt_map.count((char*)"ipeer_lport")){
    opt_map[(char*)"ipeer_lport"] = (char*)"0";
  }
  
  DHTNode dhtn( *(opt_map[(char*)"id"]), boost::bind(&handle_msg, _1),
                intf_to_ip(opt_map[(char*)"intf"]), atoi(opt_map[(char*)"lport"]),
                opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]) );
  //boost::function<void(char*)> fp = boost::bind(handle_read, _1);
  //DHTServer dhts( (char*)"localhost", 6000, fp );
  
  return 0;
}
