#include "ds_client.h"

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
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"dht_id", optional_argument, NULL, 1},
    {"num_dscnodes", optional_argument, NULL, 2},
    {"app_id", optional_argument, NULL, 3},
    {"dht_lintf", optional_argument, NULL, 4},
    {"dht_lport", optional_argument, NULL, 5},
    {"ipeer_dht_laddr", optional_argument, NULL, 6},
    {"ipeer_dht_lport", optional_argument, NULL, 7},
    {"trans_protocol", optional_argument, NULL, 8},
    {"wa_lintf", optional_argument, NULL, 9},
    {"wa_lport", optional_argument, NULL, 10},
    {"tmpfs_dir", optional_argument, NULL, 11},
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
        opt_map["dht_id"] = optarg;
        break;
      case 2:
        opt_map["num_dscnodes"] = optarg;
        break;
      case 3:
        opt_map["app_id"] = optarg;
        break;
      case 4:
        opt_map["dht_lintf"] = optarg;
        break;
      case 5:
        opt_map["dht_lport"] = optarg;
        break;
      case 6:
        opt_map["ipeer_dht_laddr"] = optarg;
        break;
      case 7:
        opt_map["ipeer_dht_lport"] = optarg;
        break;
      case 8:
        opt_map["trans_protocol"] = optarg;
        break;
      case 9:
        opt_map["wa_lintf"] = optarg;
        break;
      case 11:
        opt_map["wa_lport"] = optarg;
        break;
      case 12:
        opt_map["tmpfs_dir"] = optarg;
        break;
      case 's':
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
  std::cout << "parse_opts:: opt_map= \n" << patch_ds::map_to_str<>(opt_map);
  
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  if (opt_map["type"].compare("put") == 0) {
    // 
  }
  else if (opt_map["type"].compare("get") == 0) {
    // 
  }
  else if (opt_map["type"].compare("ri") == 0) {
    if (opt_map.count("ipeer_dht_laddr") == 0) {
      opt_map["ipeer_dht_laddr"] = "";
      opt_map["ipeer_dht_lport"] = "0";
    }
    
    std::string wa_ib_dht_lports[] = {"1234","1235","1236","1237"};
    std::list<std::string> wa_ib_lport_list(wa_ib_dht_lports, wa_ib_dht_lports + sizeof(wa_ib_dht_lports) / sizeof(std::string) );
    
    
    size_t buffer_size = 2;
    char alphabet_[] = {'a', 'b'};
    size_t alphabet_size = sizeof(alphabet_)/sizeof(*alphabet_);
    size_t context_size = 2;
    
    RIManager ri_manager(boost::lexical_cast<int>(opt_map["app_id"] ), boost::lexical_cast<int>(opt_map["num_dscnodes"] ) - 1,
                         opt_map["dht_id"].c_str()[0], intf_to_ip(opt_map["dht_lintf"] ), boost::lexical_cast<int>(opt_map["dht_lport"] ),
                         opt_map["ipeer_dht_laddr"], boost::lexical_cast<int>(opt_map["ipeer_dht_lport"] ),
                         opt_map["trans_protocol"], intf_to_ip(opt_map["wa_lintf"] ), opt_map["wa_lintf"], opt_map["wa_lport"],
                         opt_map["tmpfs_dir"], wa_ib_lport_list,
                         true, buffer_size, alphabet_, alphabet_size, context_size);
    // usleep(1*1000*1000);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else {
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  }
  
  return 0;
}