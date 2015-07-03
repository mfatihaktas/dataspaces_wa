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

size_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for(int i = 0; i < ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i] ) {
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
  for(int i = 0; i < ndim; i++)
    volume *= (size_t)dim_length[i];
  
  return volume;
}

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
      case 10:
        opt_map["gftp_lport"] = optarg;
        break;
      case 11:
        opt_map["tmpfs_dir"] = optarg;
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
    std::cout << "parse_opts:: Non-option ARGV-elements: \n";
    while (optind < argc)
      std::cout << "\t" << argv[optind++] << "\n";
  }
  // 
  std::cout << "parse_opts:: opt_map= \n" << patch_ds::map_to_str<>(opt_map);
  
  return opt_map;
}

#define TEST_NUMKEYS 1
#define TEST_SIZE 256
#define TEST_NDIM 1
#define TEST_DATASIZE TEST_NUMKEYS*pow(TEST_SIZE, 3)
#define TEST_VER 0
#define TEST_SGDIM TEST_DATASIZE

void get_test(WADspacesDriver& wads_driver)
{
  std::string var_name = "dummy";
  uint64_t* gdim_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++)
    gdim_[i] = TEST_SGDIM;
  //specifics
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_DATASIZE - 1; //TEST_SIZE - 1;
  }
  
  if (wads_driver.get(true, "int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) )
    LOG(ERROR) << "get_test:: wads_driver.get failed!";
  // size_t data_length = patch::get_data_length(TEST_NDIM, gdim, lb, ub);
  // patch::debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, data_length);
  
  patch_ds::free_all<uint64_t>(3, gdim_, lb_, ub_);
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
    ub_[i] = TEST_DATASIZE - 1; //TEST_SIZE - 1;
  }
  
  for (int i = 0; i < TEST_DATASIZE; i++) {
    data_[i] = i + 1;
  }
  
  LOG(INFO) << "put_test:: will put datasize= " << (float)TEST_DATASIZE*sizeof(int)/1024/1024 << " MB.";
  if (wads_driver.put("int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) ) {
    LOG(ERROR) << "put_test:: wads_driver.local_put failed!";
    return;
  }
  // patch::debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, patch::get_data_length(TEST_NDIM, gdim_, lb_, ub_) );
  
  patch_ds::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  std::string wa_ib_lports[] = {"1234","1235","1236","1237"};
  std::list<std::string> wa_ib_lport_list(wa_ib_lports, wa_ib_lports + sizeof(wa_ib_lports) / sizeof(std::string) );
  
  int num_dscnodes = boost::lexical_cast<int>(opt_map["num_dscnodes"] );
  int app_id = boost::lexical_cast<int>(opt_map["app_id"] );
  char dht_laddr[100];
  char wa_laddr[100];
  
  TProfiler<std::string> tprofiler;
  if (opt_map["type"].compare("put") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes - 1);
    
    std::cout << "Enter for put_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("put_test", "put_test");
    put_test("dummy", wads_driver);
    tprofiler.end_event("put_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("get") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes - 1);
    
    std::cout << "Enter for get_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("get_test", "get_test");
    get_test(wads_driver);
    tprofiler.end_event("get_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("ri") == 0) {
    if (opt_map.count("ipeer_dht_laddr") == 0) {
      opt_map["ipeer_dht_laddr"] = "";
      opt_map["ipeer_dht_lport"] = "0";
    }
    
    size_t buffer_size = 2;
    char alphabet_[] = {'a', 'b'};
    size_t alphabet_size = sizeof(alphabet_)/sizeof(*alphabet_);
    size_t context_size = 2;
    
    RIManager ri_manager(app_id, num_dscnodes-1, 
                         opt_map["dht_id"][0], intf_to_ip(opt_map["dht_lintf"] ), boost::lexical_cast<int>(opt_map["dht_lport"] ),
                         opt_map["ipeer_dht_laddr"], boost::lexical_cast<int>(opt_map["ipeer_dht_lport"] ),
                         opt_map["trans_protocol"], intf_to_ip(opt_map["wa_lintf"] ), opt_map["wa_lintf"], opt_map["gftp_lport"],
                         opt_map["tmpfs_dir"], wa_ib_lport_list,
                         true, buffer_size, alphabet_, alphabet_size, context_size);
    
    patch_ds::syncer<char> dummy_syncer;
    dummy_syncer.add_sync_point('d', 1);
    dummy_syncer.wait('d');
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else {
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  }
  
  std::cout << "main:: tprofiler= \n" << tprofiler.to_str();
  // 
  return 0;
}