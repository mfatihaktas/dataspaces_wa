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
  return inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
}

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
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
    {"ib_lintf", optional_argument, NULL, 9},
    {"gftp_lintf", optional_argument, NULL, 10},
    {"gftp_lport", optional_argument, NULL, 11},
    {"tmpfs_dir", optional_argument, NULL, 12},
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
        opt_map[(char*)"type"] = optarg;
        break;
      case 1:
        opt_map[(char*)"dht_id"] = optarg;
        break;
      case 2:
        opt_map[(char*)"num_dscnodes"] = optarg;
        break;
      case 3:
        opt_map[(char*)"app_id"] = optarg;
        break;
      case 4:
        opt_map[(char*)"dht_lintf"] = optarg;
        break;
      case 5:
        opt_map[(char*)"dht_lport"] = optarg;
        break;
      case 6:
        opt_map[(char*)"ipeer_dht_laddr"] = optarg;
        break;
      case 7:
        opt_map[(char*)"ipeer_dht_lport"] = optarg;
        break;
      case 8:
        opt_map[(char*)"trans_protocol"] = optarg;
        break;
      case 9:
        opt_map[(char*)"ib_lintf"] = optarg;
        break;
      case 10:
        opt_map[(char*)"gftp_lintf"] = optarg;
        break;
      case 11:
        opt_map[(char*)"gftp_lport"] = optarg;
        break;
      case 12:
        opt_map[(char*)"tmpfs_dir"] = optarg;
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

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  
  int num_dscnodes = boost::lexical_cast<int>(opt_map[(char*)"num_dscnodes"]);
  int app_id = boost::lexical_cast<int>(opt_map[(char*)"app_id"]);
  
  if (strcmp(opt_map[(char*)"type"], (char*)"put") == 0) {
    //
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"get") == 0){
    //
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri") == 0) {
    if ( (!opt_map.count((char*)"ipeer_dht_laddr") ) || (strcmp(opt_map[(char*)"ipeer_dht_laddr"], (char*)"") == 0) ) {
      opt_map[(char*)"ipeer_dht_laddr"] = NULL;
      opt_map[(char*)"ipeer_dht_lport"] = (char*)"0";
    }
    
    std::string wa_ib_dht_lports[] = {"1234","1235","1236","1237"};
    std::list<std::string> wa_ib_lport_list(wa_ib_dht_lports, wa_ib_dht_lports + sizeof(wa_ib_dht_lports) / sizeof(std::string) );
    
    std::string trans_protocol(opt_map[(char*)"trans_protocol"] );
    std::string wa_laddr(intf_to_ip(opt_map[(char*)"ib_lintf"] ) );
    std::string wa_gftp_lintf(opt_map[(char*)"gftp_lintf"] );
    std::string wa_gftp_lport(opt_map[(char*)"gftp_lport"] );
    std::string tmpfs_dir(opt_map[(char*)"tmpfs_dir"] );
    
    size_t buffer_size = 2;
    char alphabet_[] = {'a', 'b'};
    size_t alphabet_size = sizeof(alphabet_)/sizeof(*alphabet_);
    size_t context_size = 2;
    
    RIManager ri_manager(app_id, num_dscnodes-1,
                         opt_map[(char*)"dht_id"][0], intf_to_ip(opt_map[(char*)"dht_lintf"]), atoi(opt_map[(char*)"dht_lport"]), opt_map[(char*)"ipeer_dht_laddr"], atoi(opt_map[(char*)"ipeer_dht_lport"]),
                         trans_protocol, wa_laddr, wa_gftp_lintf, wa_gftp_lport, 
                         tmpfs_dir, wa_ib_lport_list,
                         buffer_size, alphabet_, alphabet_size, context_size );
    //usleep(1*1000*1000);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else{
    LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  return 0;
}