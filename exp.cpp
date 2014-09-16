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

#include "dataspaces_wa.h"

#ifndef _PRINT_FUNCS_
#define _PRINT_FUNCS_
void exp_debug_print(std::string key, unsigned int ver, int size, int ndim, 
                     uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data, size_t data_length)
{
  LOG(INFO) << "exp_debug_print::";
  std::cout << "key= " << key << "\n"
            << "ver= " << ver << "\n"
            << "size= " << size << "\n"
            << "ndim= " << ndim << "\n";
  std::cout << "gdim=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << gdim[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "lb=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << lb[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "ub=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << ub[i] << ", ";
  }
  std::cout << "\n";
  
  //
  if (data == NULL){
    return;
  }
  std::cout << "data_length= " << data_length << "\n";
  std::cout << "data=";
  for (int i=0; i<data_length; i++){
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}

size_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for(int i=0; i<ndim; i++){
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i]){
      LOG(ERROR) << "get_data_length:: lb= " << lb << " is not feasible!";
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb){
      LOG(ERROR) << "get_data_length:: ub= " << ub << " is not feasible!";
      return 0;
    }
    dim_length[i] = ub - lb;
  }
  
  size_t volume = 1;
  for(int i=0; i<ndim; i++){
    volume *= (size_t)dim_length[i];
  }
  
  return volume;
}
#endif

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
    {"ib_lintf", optional_argument, NULL, 8},
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
      case 8:
        opt_map[(char*)"ib_lintf"] = optarg;
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

#define TEST_SIZE 1024*1024
#define TEST_NDIM 1
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 1
#define TEST_SGDIM 1024*1024*257

void l_put_test(std::string var_name, WADspacesDriver& wads_driver)
{
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
  
  size_t data_length = get_data_length(TEST_NDIM, gdim, lb, ub);
  
  if (wads_driver.local_put("int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data) ){
    LOG(ERROR) << "l_put_test:: wads_driver.local_put failed!";
    return;
  }
  //exp_debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data, data_length);
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
}

void l_get_test(std::string var_name, WADspacesDriver& wads_driver)
{
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  //int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  void* data = malloc(TEST_DATASIZE*sizeof(int));
  
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  // size_t data_length = get_data_length(TEST_NDIM, gdim, lb, ub);
  // LOG(INFO) << "l_get_test:: data_length= " << data_length;
  
  //exp_debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  int result = wads_driver.local_get(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "l_get_test:: after local_get;";
  // exp_debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, static_cast<int*>(data), TEST_DATASIZE);
  
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
  
  // exp_debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  if (wads_driver.remote_get("int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data) ){
    LOG(ERROR) << "r_get_test:: wads_driver.remote_get failed!";
  }
  // size_t data_length = get_data_length(TEST_NDIM, gdim, lb, ub);
  // exp_debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim, lb, ub, data, data_length);
  //
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
  
  std::string wa_ib_lports[] = {"1234","1235","1236","1237"};
  std::list<std::string> wa_ib_lport_list(wa_ib_lports, wa_ib_lports + sizeof(wa_ib_lports) / sizeof(std::string) );
  
  int num_dscnodes = boost::lexical_cast<int>(opt_map[(char*)"num_dscnodes"]);
  int app_id = boost::lexical_cast<int>(opt_map[(char*)"app_id"]);
  char lip[100];
  char ib_lip[100];
  
  if (strcmp(opt_map[(char*)"type"], (char*)"l_put") == 0){
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    l_put_test("dummy", wads_driver);
    //l_put_test("dummy2", wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"l_get") == 0){
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    // usleep(5*1000*1000);
    l_get_test("dummy", wads_driver);
    //
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"r_put") == 0){
    //
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"r_get") == 0){
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    //usleep(2*1000*1000);
    r_get_test(wads_driver);
    //std::cout << "main:: calling r_get_test again!\n";
    //r_get_test(wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri_t") == 0){
    char* lip_t = intf_to_ip(opt_map[(char*)"lintf"]);
    strcpy(lip, lip_t);
    std::cout << "main:: lip= " << lip << "\n";
    
    char* ib_lip_t = intf_to_ip(opt_map[(char*)"ib_lintf"]);
    strcpy(ib_lip, ib_lip_t);
    std::cout << "main:: ib_lip= " << ib_lip << "\n";
    //
    if (!opt_map.count((char*)"ipeer_lip")){
      opt_map[(char*)"ipeer_lip"] = NULL;
      opt_map[(char*)"ipeer_lport"] = (char*)"0";
    }
    
    RIManager ri_manager(opt_map[(char*)"dht_id"][0], num_dscnodes-1, app_id, 
                         lip, atoi(opt_map[(char*)"lport"]),
                         opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]),
                         ib_lip, wa_ib_lport_list);

    //usleep(5*1000*1000);
    //ri_manager.remote_query("dummy");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri") == 0){
    char* lip_t = intf_to_ip(opt_map[(char*)"lintf"]);
    strcpy(lip, lip_t);
    std::cout << "main:: lip= " << lip << "\n";
    
    char* ib_lip_t = intf_to_ip(opt_map[(char*)"ib_lintf"]);
    strcpy(ib_lip, ib_lip_t);
    std::cout << "main:: ib_lip= " << ib_lip << "\n";
    //
    if (!opt_map.count((char*)"ipeer_lip")){
      opt_map[(char*)"ipeer_lip"] = NULL;
      opt_map[(char*)"ipeer_lport"] = (char*)"0";
    }
    RIManager ri_manager(opt_map[(char*)"dht_id"][0], num_dscnodes-1, app_id, 
                         lip, atoi(opt_map[(char*)"lport"]),
                         opt_map[(char*)"ipeer_lip"], atoi(opt_map[(char*)"ipeer_lport"]),
                         opt_map[(char*)"ib_lintf"], wa_ib_lport_list);
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else{
    LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  return 0;
}