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

#include "patch_exp.h"

// #include "boost/tuple/tuple.hpp"
// #include "boost/tuple/tuple_comparison.hpp"
// #include "boost/tuple/tuple_io.hpp"

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
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv, "s", long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c) {
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
    log_(INFO, "Non-option ARGV-elements=")
    while (optind < argc)
      std::cout << argv[optind++] << "\n";
  }
  // 
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  
  return opt_map;
}

int main(int argc, char **argv)
{
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  // Packet p(PACKET_RIMSG, opt_map);
  // log_(INFO, "p.int_to_char_(8, 12)= " << patch::arr_to_str<>(8, p.int_to_char_(8, 12) ) )
  
  if (opt_map.count("joinhost_lip") == 0) {
    opt_map["joinhost_lip"] = "";
    opt_map["joinhost_lport"] = "0";
  }
  
  std::string s_lip = "192.168.2.151";
  int s_lport = 6633;
  // /*
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
    SDMNode sdm_node(opt_map["node_type"], true,
                     boost::lexical_cast<int>(opt_map["id"] ), intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ), opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                     boost::bind(&handle_rimsg_recv, _1), boost::bind(&handle_cp_recv, _1) );
    sleep(1);
    if (opt_map["node_type"].compare("s") == 0) {
      std::map<std::string, std::string> msg_map;
      msg_map["type"] = "HI";
      sdm_node.send_msg_to_master(PACKET_RIMSG, msg_map);
    }
      
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("control") == 0) {
    if (opt_map["node_type"].compare("m") == 0)
      master_test(opt_map);
    else if (opt_map["node_type"].compare("s") == 0)
      slave_test(opt_map);
  }
  else
    log_(ERROR, "unknown type= " << opt_map["type"] )
  
  return 0;
}
