//for intf_to_ip
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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <glog/logging.h>

#include "dht_node.h"
#include "dht_client.h"
#include "packet.h"

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
    {"intf", required_argument, NULL, 0},
    {"lport", required_argument, NULL, 1},
    {"ipeer_lip", required_argument, NULL, 2},
    {"ipeer_lport", required_argument, NULL, 3},
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
        opt_map[(char*)"intf"] = optarg;
      case 1:
        opt_map[(char*)"lport"] = optarg;
      case 2:
        opt_map[(char*)"ipeer_lip"] = optarg;
      case 3:
        opt_map[(char*)"ipeer_lport"] = optarg;  
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


int main(int argc, char **argv){
  google::InitGoogleLogging("exp");
  //
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  /*
  DHTClient dhtc((char*)"localhost", 6000);
  dhtc.connect();
  
  Packet packet('J', (char*)"SOS");
  char* data = packet.get_data();
  dhtc.send(packet.get_packet_size(), packet.get_data());
  */
  DHTNode dhtn('2',
               intf_to_ip(opt_map[(char*)"intf"]), atoi(opt_map[(char*)"lport"]),
               opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]) );
  //
  /*
  std::cout << "Enter\n";
  std::cin.ignore();
  */
	return 0;
}
