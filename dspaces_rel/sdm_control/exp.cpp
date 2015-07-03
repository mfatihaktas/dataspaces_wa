#include "sdm_control.h"

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

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

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
  
  static struct option long_options[] = {
    {"type", optional_argument, NULL, 0},
    {"id", optional_argument, NULL, 1},
    {"node_type", optional_argument, NULL, 2},
    {"lintf", optional_argument, NULL, 3},
    {"lport", optional_argument, NULL, 4},
    {"joinhost_lip", optional_argument, NULL, 5},
    {"joinhost_lport", optional_argument, NULL, 6},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s", long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["id"] = optarg;
        break;
      case 2:
        opt_map["node_type"] = optarg;
        break;
      case 3:
        opt_map["lintf"] = optarg;
        break;
      case 4:
        opt_map["lport"] = optarg;
        break;
      case 5:
        opt_map["joinhost_lip"] = optarg;
        break;
      case 6:
        opt_map["joinhost_lport"] = optarg;
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc) {
    std::cout << "parse_opts:: Non-option ARGV-elements= \n";
    while (optind < argc)
      std::cout << argv[optind++] << "\n";
  }
  // 
  std::cout << "parse_opts:: opt_map= \n";
  for (std::map<std::string, std::string>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it)
    std::cout << it->first << " : " << it->second << "\n";
  
  return opt_map;
}

void handle_char_recv(char* msg_)
{
  LOG(INFO) << "handle_char_recv:: msg_= " << msg_;
}

void handle_rimsg_recv(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_rimsg_recv:: msg_map= \n" << patch_sdm::map_to_str<std::string, std::string>(msg_map);
}

void handle_cp_recv(boost::shared_ptr<Packet> p_)
{
  LOG(INFO) << "handle_cp_recv:: p= \n" << p_->to_str();
}

int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  if (opt_map.count("joinhost_lip") == 0) {
    opt_map["joinhost_lip"] = "";
    opt_map["joinhost_lport"] = "0";
  }
  
  std::string s_lip = "192.168.2.151";
  int s_lport = 6633;
  
  if (opt_map["type"].compare("server") == 0) {
    SDMServer sdm_server("sdm_server", s_lip, s_lport, boost::bind(handle_char_recv, _1) );
    // 
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("client") == 0) {
    SDMClient sdm_client("sdm_client", s_lip, s_lport);
    sdm_client.connect();
    
    std::string str = "0010 deneme 1 2 3";
    sdm_client.send(str.size(), &str[0u] );
    // 
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("node") == 0) {
    SDMNode sdm_node(opt_map["id"].c_str()[0], opt_map["node_type"],
                     intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ),
                     opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                     boost::bind(&handle_rimsg_recv, _1), boost::bind(&handle_cp_recv, _1) );
  
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("control") == 0) {
    if (opt_map["node_type"].compare("m") == 0) {
      SimpleMaster master(opt_map["id"].c_str()[0], "m",
                          intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ),
                          opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                          boost::bind(&handle_rimsg_recv, _1) );
      std::cout << "Enter \n";
      getline(std::cin, temp);
    }
    else if (opt_map["node_type"].compare("s") == 0) {
      SimpleSlave slave(opt_map["id"].c_str()[0], "s",
                        intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ),
                        opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                        boost::bind(&handle_rimsg_recv, _1) );
      std::cout << "Enter \n";
      getline(std::cin, temp);
    }
  }
  else {
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  }
  
  return 0;
}