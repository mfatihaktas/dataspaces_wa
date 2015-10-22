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
  
  int volume = 1;
  for(int i=0; i<ndim; i++) {
    volume *= (int)dim_length[i];
  }
  
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
    {"cl_id", optional_argument, NULL, 1},
    {"base_client_id", optional_argument, NULL, 2},
    {"num_client", optional_argument, NULL, 3},
    {"lcontrol_lintf", optional_argument, NULL, 4},
    {"lcontrol_lport", optional_argument, NULL, 5},
    {"join_lcontrol_lip", optional_argument, NULL, 6},
    {"join_lcontrol_lport", optional_argument, NULL, 7},
    {"num_putget", optional_argument, NULL, 8},
    {"inter_time_sec", optional_argument, NULL, 9},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s",
                     long_options, &option_index);

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
        opt_map["lcontrol_lintf"] = optarg;
        break;
      case 5:
        opt_map["lcontrol_lport"] = optarg;
        break;
      case 6:
        opt_map["join_lcontrol_lip"] = optarg;
        break;
      case 7:
        opt_map["join_lcontrol_lport"] = optarg;
        break;
      case 8:
        opt_map["num_putget"] = optarg;
        break;
      case 9:
        opt_map["inter_time_sec"] = optarg;
        break;
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
// 10*1024*
#define TEST_SIZE 200*1024*1024
#define TEST_DATASIZE pow(TEST_SIZE, NDIM)
#define TEST_VER 0
#define TEST_SGDIM TEST_SIZE

void mget_test(int num_get, float inter_get_time_sec, std::string base_var_name, WADSDriver& wads_driver)
{
  uint64_t* gdim_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++)
    gdim_[i] = TEST_SGDIM;
  //specifics
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_SIZE - 1;
  }
  
  TProfiler<int> get_time_profiler;
  for (int i = 0; i < num_get; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    std::cout << "\n"; // to see better on terminal
    get_time_profiler.add_event(i, var_name);
    if (wads_driver.get(true, var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      LOG(ERROR) << "mget_test:: wads_driver.get failed for var_name= " << var_name;
      return;
    }
    get_time_profiler.end_event(i);
    
    sleep(inter_get_time_sec);
  }
  LOG(INFO) << "mget_test:: get_time_profiler= \n" << get_time_profiler.to_str();
  // int data_length = patch::get_data_length(NDIM, gdim, lb, ub);
  // patch_ds::debug_print(var_name, TEST_VER, sizeof(int), NDIM, gdim_, lb_, ub_, data_, data_length);
  
  patch_all::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void mput_test(int num_put, float inter_put_time_sec, std::string base_var_name, WADSDriver& wads_driver)
{
  uint64_t* gdim_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++)
    gdim_[i] = TEST_SGDIM;
  //specifics
  int *data_ = (int*)malloc(TEST_DATASIZE*sizeof(int) );
  uint64_t *lb_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++) {
    lb_[i] = 0;
    ub_[i] = TEST_SIZE - 1;
  }
  
  for (int i = 0; i < TEST_DATASIZE; i++)
    data_[i] = i + 1;
  
  for (int i = 0; i < num_put; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    std::cout << "\n"; // to see better on terminal
    if (wads_driver.put(var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      LOG(ERROR) << "mput_test:: wads_driver.put failed for var_name= " << var_name;
      return;
    }
    sleep(inter_put_time_sec);
  }
  // patch_ds::debug_print(var_name, TEST_VER, sizeof(int), NDIM, gdim_, lb_, ub_, data_, patch::get_data_length(NDIM, gdim_, lb_, ub_) );
  
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
  if (str_cstr_equals(opt_map["type"], "mput") ) {
    MWADSDriver wads_driver(boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
                            intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
    
    std::cout << "Enter for mput_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("mput_test", "mput_test");
    mput_test(boost::lexical_cast<float>(opt_map["num_putget"] ), boost::lexical_cast<float>(opt_map["inter_time_sec"] ),
                   "dummy", wads_driver);
    tprofiler.end_event("mput_test");
    // mput_test(num_putget, "dummy2", wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "mget") ) {
    MWADSDriver wads_driver(boost::lexical_cast<int>(opt_map["cl_id"] ), boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_client"] ),
                            intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ), opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
    
    std::cout << "Enter for mget_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("mget_test", "mget_test");
    mget_test(boost::lexical_cast<float>(opt_map["num_putget"] ), boost::lexical_cast<float>(opt_map["inter_time_sec"] ),
                   "dummy", wads_driver);
    tprofiler.end_event("mget_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  
  std::cout << "main:: tprofiler= \n" << tprofiler.to_str();
  // 
  return 0;
}
