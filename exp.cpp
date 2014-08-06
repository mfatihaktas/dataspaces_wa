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
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "ds_client.h"
#include "dataspaces_wa.h"

char* intf_to_ip(const char* intf)
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
    {"type", optional_argument, NULL, 0},
    {"dht_id", optional_argument, NULL, 1},
    {"num_dscnodes", optional_argument, NULL, 2},
    {"app_id", optional_argument, NULL, 3},
    {"lintf", optional_argument, NULL, 4},
    {"lport", optional_argument, NULL, 5},
    {"ipeer_lip", optional_argument, NULL, 6},
    {"ipeer_lport", optional_argument, NULL, 7},
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
        opt_map[(char*)"dht_id"] = optarg;
        break;
      case 2:
        opt_map[(char*)"num_dscnodes"] = optarg;
        break;
      case 3:
        opt_map[(char*)"app_id"] = optarg;
        break;
      case 4:
        opt_map[(char*)"lintf"] = optarg;
        break;
      case 5:
        opt_map[(char*)"lport"] = optarg;
        break;
      case 6:
        opt_map[(char*)"ipeer_lip"] = optarg;
        break;
      case 7:
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

#define TEST_SIZE 5
#define TEST_NDIM 1
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 1
#define TEST_SGDIM 10

void l_put_test(WADspacesDriver& wads_driver)
{
  std::string var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  for (int i=0; i<TEST_DATASIZE; i++){
    data[i] = (i+1);
  }
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  int result = wads_driver.local_put(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data);
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
}

void r_get_test(WADspacesDriver& wads_driver)
{
  std::string var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  int result = wads_driver.remote_get(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data);
  
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  
  int num_dscnodes = boost::lexical_cast<int>(opt_map[(char*)"num_dscnodes"]);
  int app_id = boost::lexical_cast<int>(opt_map[(char*)"app_id"]);
  
  if (strcmp(opt_map[(char*)"type"], (char*)"l_put") == 0){
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    //l_put_test(wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"l_get") == 0){
    //
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"r_put") == 0){
    //
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"r_get") == 0){
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    //r_get_test(wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri_t") == 0){
    if (!opt_map.count((char*)"ipeer_lip")){
      opt_map[(char*)"ipeer_lip"] = NULL;
      opt_map[(char*)"ipeer_lport"] = (char*)"0";
    }
    
    RIManager ri_manager(opt_map[(char*)"dht_id"][0], num_dscnodes-1, app_id, 
                         intf_to_ip(opt_map[(char*)"lintf"]), atoi(opt_map[(char*)"lport"]),
                         opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]) );

    usleep(5*1000*1000);
    ri_manager.remote_query("dummy");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri") == 0){
    if (!opt_map.count((char*)"ipeer_lip")){
      opt_map[(char*)"ipeer_lip"] = NULL;
      opt_map[(char*)"ipeer_lport"] = (char*)"0";
    }
    RIManager ri_manager(opt_map[(char*)"dht_id"][0], num_dscnodes-1, app_id, 
                         intf_to_ip(opt_map[(char*)"lintf"]), atoi(opt_map[(char*)"lport"]),
                         opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]) );

    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else{
    LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  return 0;
}