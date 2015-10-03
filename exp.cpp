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

#include "profiler.h"
#include "dataspaces_wa.h"

int get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
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
  
  int volume = 1;
  for(int i = 0; i < ndim; i++)
    volume *= (int)dim_length[i];
  
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
  // 
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"cl_id", optional_argument, NULL, 1},
    {"base_client_id", optional_argument, NULL, 2},
    {"num_client", optional_argument, NULL, 3},
    {"ds_id", optional_argument, NULL, 4},
    {"control_lintf", optional_argument, NULL, 5},
    {"control_lport", optional_argument, NULL, 6},
    {"join_control_lip", optional_argument, NULL, 7},
    {"join_control_lport", optional_argument, NULL, 8},
    {"trans_protocol", optional_argument, NULL, 9},
    {"ib_lintf", optional_argument, NULL, 10},
    {"gftp_lintf", optional_argument, NULL, 11},
    {"gftp_lport", optional_argument, NULL, 12},
    {"tmpfs_dir", optional_argument, NULL, 13},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "", long_options, &option_index);

    if (c == -1) // Detect the end of the options.
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
        opt_map["control_lintf"] = optarg;
        break;
      case 6:
        opt_map["control_lport"] = optarg;
        break;
      case 7:
        opt_map["join_control_lip"] = optarg;
        break;
      case 8:
        opt_map["join_control_lport"] = optarg;
        break;
      case 9:
        opt_map["trans_protocol"] = optarg;
        break;
      case 10:
        opt_map["ib_lintf"] = optarg;
        break;
      case 11:
        opt_map["gftp_lintf"] = optarg;
        break;
      case 12:
        opt_map["gftp_lport"] = optarg;
        break;
      case 13:
        opt_map["tmpfs_dir"] = optarg;
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
  std::cout << "parse_opts:: opt_map= \n" << patch_all::map_to_str<>(opt_map);
  
  return opt_map;
}

#define TEST_SIZE 256
#define TEST_NDIM 1
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 0
#define TEST_SGDIM TEST_DATASIZE

void get_test(std::string var_name, WADSDriver& wads_driver)
{
  uint64_t* gdim_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++)
    gdim_[i] = TEST_SGDIM;
  
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t) );
  for (int i = 0; i < TEST_NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_SIZE - 1;
  }
  
  if (wads_driver.get(true, var_name, TEST_VER, "int", sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) )
    LOG(ERROR) << "get_test:: wads_driver.get failed!";
  
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void put_test(std::string var_name, WADSDriver& wads_driver)
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
  
  for (int i = 0; i < TEST_DATASIZE; i++)
    data_[i] = i + 1;
  
  LOG(INFO) << "put_test:: will put datasize= " << (float)TEST_DATASIZE*sizeof(int)/1024/1024 << " MB.";
  if (wads_driver.put(var_name, TEST_VER, "int", sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) )
    LOG(ERROR) << "put_test:: wads_driver.local_put failed!";
  
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  TProfiler<std::string> tprofiler;
  if (str_cstr_equals(opt_map["type"], "put") ) {
    MWADSDriver wads_driver(boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ) );
    
    std::cout << "Enter for put_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("put_test", "put_test");
    put_test("dummy", wads_driver);
    tprofiler.end_event("put_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "get") ) {
    MWADSDriver wads_driver(boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ) );
    
    std::cout << "Enter for get_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("get_test", "get_test");
    get_test("dummy", wads_driver);
    tprofiler.end_event("get_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
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
       MMRIManager ri_manager(
        boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
        boost::lexical_cast<char>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
        malgo_t, max_num_key_ver_in_mpbuffer, w_prefetch,
        opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
        opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] );
      
      // SMRIManager ri_manager(
      //   boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
      //   boost::lexical_cast<char>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
      //   salgo_t, lcoor_, ucoor_, sexpand_length, w_prefetch,
      //   opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
      //   opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] );
    
      std::cout << "Enter\n";
      getline(std::cin, temp);
    }
    else {
       MSRIManager ri_manager(
        boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
        boost::lexical_cast<char>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
        opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
        opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] );
      
      // SSRIManager ri_manager(
      //     boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
      //     boost::lexical_cast<char>(opt_map["ds_id"] ), intf_to_ip(opt_map["control_lintf"] ), boost::lexical_cast<int>(opt_map["control_lport"] ), opt_map["join_control_lip"], boost::lexical_cast<int>(opt_map["join_control_lport"] ),
      //     opt_map["trans_protocol"], intf_to_ip(opt_map["ib_lintf"] ), ib_lport_list,
      //     opt_map["gftp_lintf"], intf_to_ip(opt_map["gftp_lintf"] ), opt_map["gftp_lport"], opt_map["tmpfs_dir"] );
      
      std::cout << "Enter\n";
      getline(std::cin, temp);
    }
    // patch_all::syncer<char> dummy_syncer;
    // dummy_syncer.add_sync_point('d', 1);
    // dummy_syncer.wait('d');
    
  }
  else
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  
  std::cout << "main:: tprofiler= \n" << tprofiler.to_str();
  // 
  return 0;
}