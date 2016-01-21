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

#include "remote_interact.h"

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
    {"cl_id", optional_argument, NULL, 1},
    {"base_client_id", optional_argument, NULL, 2},
    {"num_client", optional_argument, NULL, 3},
    {"ds_id", optional_argument, NULL, 4},
    {"lcontrol_lintf", optional_argument, NULL, 5},
    {"lcontrol_lport", optional_argument, NULL, 6},
    {"join_lcontrol_lip", optional_argument, NULL, 7},
    {"join_lcontrol_lport", optional_argument, NULL, 8},
    {"control_lintf", optional_argument, NULL, 9},
    {"control_lport", optional_argument, NULL, 10},
    {"join_control_lip", optional_argument, NULL, 11},
    {"join_control_lport", optional_argument, NULL, 12},
    {"trans_protocol", optional_argument, NULL, 13},
    {"ib_lintf", optional_argument, NULL, 14},
    {"tcp_lintf", optional_argument, NULL, 15},
    {"tcp_lport", optional_argument, NULL, 16},
    {"gftp_lintf", optional_argument, NULL, 17},
    {"gftp_lport", optional_argument, NULL, 18},
    {"tmpfs_dir", optional_argument, NULL, 19},
    {0, 0, 0, 0}
  };
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv, "", long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["cl_id"] = optarg;
        break;
      case 2:
        opt_map["base_client_id"] = optarg;
        break;  
      case 3:
        opt_map["num_client"] = optarg;
        break;
      case 4:
        opt_map["ds_id"] = optarg;
        break;
      case 5:
        opt_map["lcontrol_lintf"] = optarg;
        break;
      case 6:
        opt_map["lcontrol_lport"] = optarg;
        break;
      case 7:
        opt_map["join_lcontrol_lip"] = optarg;
        break;
      case 8:
        opt_map["join_lcontrol_lport"] = optarg;
        break;
      case 9:
        opt_map["control_lintf"] = optarg;
        break;
      case 10:
        opt_map["control_lport"] = optarg;
        break;
      case 11:
        opt_map["join_control_lip"] = optarg;
        break;
      case 12:
        opt_map["join_control_lport"] = optarg;
        break;
      case 13:
        opt_map["trans_protocol"] = optarg;
        break;
      case 14:
        opt_map["ib_lintf"] = optarg;
        break;
      case 15:
        opt_map["tcp_lintf"] = optarg;
        break;
      case 16:
        opt_map["tcp_lport"] = optarg;
        break;
      case 17:
        opt_map["gftp_lintf"] = optarg;
        break;
      case 18:
        opt_map["gftp_lport"] = optarg;
        break;
      case 19:
        opt_map["tmpfs_dir"] = optarg;
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
  std::cout << "parse_opts:: opt_map= \n" << patch::map_to_str<>(opt_map);
  
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  if (str_cstr_equals(opt_map["type"], "put") ) {
    // 
  }
  else if (str_cstr_equals(opt_map["type"], "get") ) {
    // 
  }
  else if (str_cstr_equals(opt_map["type"], "ri") ) {
    std::string ib_lports[] = {"1234","1235","1236","1237"};
    std::list<std::string> ib_lport_list(ib_lports, ib_lports + sizeof(ib_lports)/sizeof(*ib_lports) );
    
    MALGO_T malgo_t = MALGO_W_PPM;
    int max_num_key_ver_in_mpbuffer = 10;
    
    SALGO_T salgo_t = SALGO_H;
    COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
    COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 16) };
    int sexpand_length = 1;
    
    bool w_prefetch = true;
    
    if (str_cstr_equals(opt_map["join_control_lip"], "") ) {
      boost::shared_ptr<RIManager> ri_manager_(
        new MMRIManager(
          boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
          intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ),
          boost::lexical_cast<int>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
          malgo_t, max_num_key_ver_in_mpbuffer, w_prefetch,
          opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
          intf_to_ip(opt_map["tcp_lintf"] ), boost::lexical_cast<int>(opt_map["tcp_lport"] ),
          opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] ) );
      
      // boost::shared_ptr<RIManager> ri_manager_(
      //   new SMRIManager(
      //     boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
      //     intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ),
      //     boost::lexical_cast<int>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
      //     salgo_t, lcoor_, ucoor_, sexpand_length, w_prefetch,
      //     opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
      //     intf_to_ip(opt_map["tcp_lintf"] ), boost::lexical_cast<int>(opt_map["tcp_lport"] ),
      //     opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] ) );
    }
    else {
      boost::shared_ptr<RIManager> ri_manager_(
        new MSRIManager(
          boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
          intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ),
          boost::lexical_cast<int>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
          opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
          intf_to_ip(opt_map["tcp_lintf"] ), boost::lexical_cast<int>(opt_map["tcp_lport"] ),
          opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] ) );
      
      // boost::shared_ptr<RIManager> ri_manager_(
      //   new SSRIManager(
      //     boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
      //     intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ),
      //     boost::lexical_cast<int>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
      //     opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
      //     intf_to_ip(opt_map["tcp_lintf"] ), boost::lexical_cast<int>(opt_map["tcp_lport"] ),
      //     opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] ) );
    }
    // usleep(1*1000*1000);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else
    log_(ERROR, "unknown type= " << opt_map["type"] )
  
  return 0;
}