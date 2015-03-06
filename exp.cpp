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

void debug_print(std::string key, unsigned int ver, int size, int ndim, 
                uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data, size_t data_length)
{
  std::cout << "debug_print::";
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
  if (data == NULL) {
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
  
  for(int i=0; i<ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i]) {
      LOG(ERROR) << "get_data_length:: lb= " << lb << " is not feasible!";
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb) {
      LOG(ERROR) << "get_data_length:: ub= " << ub << " is not feasible!";
      return 0;
    }
    dim_length[i] = ub - lb;
  }
  
  size_t volume = 1;
  for(int i=0; i<ndim; i++) {
    volume *= (size_t)dim_length[i];
  }
  
  return volume;
}

template <typename T>
void free_all(int num, ...)
{
  va_list arguments;                     // A place to store the list of arguments

  va_start ( arguments, num );           // Initializing arguments to store all values after num
  
  for ( int x = 0; x < num; x++ )        // Loop until all numbers are added
    va_arg ( arguments, T* );
  
  va_end ( arguments );                  // Cleans up the list
}

char* intf_to_ip(const char* intf)
{
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  // Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name , intf , IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  
  return inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
}

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
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
    {"gftp_lport", optional_argument, NULL, 10},
    {"tmpfs_dir", optional_argument, NULL, 11},
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
        opt_map[(char*)"wa_lintf"] = optarg;
        break;
      case 10:
        opt_map[(char*)"gftp_lport"] = optarg;
        break;
      case 11:
        opt_map[(char*)"tmpfs_dir"] = optarg;
        break;
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
    std::cout << "non-option ARGV-elements: \n";
    while (optind < argc)
      std::cout << "\t" << argv[optind++] << "\n";
  }
  // 
  std::cout << "opt_map=\n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it) {
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

#define TEST_SIZE 300
#define TEST_NDIM 3
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 0
#define TEST_SGDIM 1024

void get_test(WADspacesDriver& wads_driver)
{
  std::string var_name = "dummy";
  uint64_t* gdim_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    gdim_[i] = TEST_SGDIM;
  }
  //specifics
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_SIZE - 1;
  }
  
  if (wads_driver.get(true, "int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "get_test:: wads_driver.get failed!";
  }
  // size_t data_length = patch::get_data_length(TEST_NDIM, gdim, lb, ub);
  // patch::debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, data_length);
  
  free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void put_test(std::string var_name, WADspacesDriver& wads_driver)
{
  uint64_t* gdim_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    gdim_[i] = TEST_SGDIM;
  }
  //specifics
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_SIZE - 1;
  }
  
  for (int i = 0; i < TEST_DATASIZE; i++) {
    data_[i] = i + 1;
  }
  
  if (wads_driver.put("int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "put_test:: wads_driver.local_put failed!";
    return;
  }
  // patch::debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, patch::get_data_length(TEST_NDIM, gdim_, lb_, ub_) );
  
  free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
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
  char dht_laddr[100];
  char wa_laddr[100];
  
  if (strcmp(opt_map[(char*)"type"], (char*)"put") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    std::cout << "Enter for put_test...\n";
    getline(std::cin, temp);
    
    put_test("dummy", wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"put_2") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    std::cout << "Enter for put_test\n";
    getline(std::cin, temp);
    
    put_test("dummy_2", wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"get") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    std::cout << "Enter for get_test...\n";
    getline(std::cin, temp);
    
    get_test(wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"ri") == 0) {
    char* dht_laddr_t = intf_to_ip(opt_map[(char*)"dht_lintf"]);
    strcpy(dht_laddr, dht_laddr_t);
    std::cout << "main:: dht_laddr= " << dht_laddr << "\n";
    
    char* wa_laddr_t = intf_to_ip(opt_map[(char*)"wa_lintf"]);
    strcpy(wa_laddr, wa_laddr_t);
    std::cout << "main:: wa_laddr= " << wa_laddr << "\n";
    // 
    if ( (!opt_map.count((char*)"ipeer_dht_laddr") ) || (strcmp(opt_map[(char*)"ipeer_dht_laddr"], (char*)"") == 0) ) {
      opt_map[(char*)"ipeer_dht_laddr"] = NULL;
      opt_map[(char*)"ipeer_dht_lport"] = (char*)"0";
    }
    
    std::string trans_protocol(opt_map[(char*)"trans_protocol"] );
    std::string wa_laddr_str(wa_laddr);
    std::string wa_lintf(opt_map[(char*)"wa_lintf"] );
    std::string wa_gftp_lport(opt_map[(char*)"gftp_lport"] );
    std::string tmpfs_dir(opt_map[(char*)"tmpfs_dir"] );
    
    size_t buffer_size = 2;
    char alphabet_[] = {'a', 'b'};
    size_t alphabet_size = sizeof(alphabet_)/sizeof(*alphabet_);
    size_t context_size = 2;
    
    RIManager ri_manager(app_id, num_dscnodes-1, 
                         opt_map[(char*)"dht_id"][0], dht_laddr, atoi(opt_map[(char*)"dht_lport"]), opt_map[(char*)"ipeer_dht_laddr"], atoi( (opt_map[(char*)"ipeer_dht_lport"]) ),
                         trans_protocol, wa_laddr_str, wa_lintf, wa_gftp_lport, 
                         tmpfs_dir, wa_ib_lport_list,
                         buffer_size, alphabet_, alphabet_size, context_size );
    
    syncer<char> dummy_syncer;
    dummy_syncer.add_sync_point('d', 1);
    dummy_syncer.wait('d');
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else {
    LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  return 0;
}