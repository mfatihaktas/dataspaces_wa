#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <fstream>
#include <math.h>

#include <boost/math/distributions/exponential.hpp>
#include <boost/lexical_cast.hpp>

#include "profiler.h"
#include "dataspaces_wa.h"

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
    {"lcontrol_lintf", optional_argument, NULL, 3},
    {"lcontrol_lport", optional_argument, NULL, 4},
    {"join_lcontrol_lip", optional_argument, NULL, 5},
    {"join_lcontrol_lport", optional_argument, NULL, 6},
    {"num_putget", optional_argument, NULL, 7},
    {0, 0, 0, 0}
  };
  
  while (1) {
    int option_index = 0;
    c = getopt_long(argc, argv, "s",
                    long_options, &option_index);

    if (c == -1) // Detect the end of the options.
      break;
    
    switch (c) {
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
        opt_map["lcontrol_lintf"] = optarg;
        break;
      case 4:
        opt_map["lcontrol_lport"] = optarg;
        break;
      case 5:
        opt_map["join_lcontrol_lip"] = optarg;
        break;
      case 6:
        opt_map["join_lcontrol_lport"] = optarg;
        break;
      case 7:
        opt_map["num_putget"] = optarg;
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
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  
  return opt_map;
}
// 
const float INTER_ARR_TIME = 0; // sec
const float GET_ARR_RATE = (float)1/20; // num_arr / sec

const uint64_t TEST_DATA_SIZE = 400*1024*1024; // 1024*1024*1024;
const uint64_t TEST_DATA_LENGTH = TEST_DATA_SIZE/sizeof(int);
const uint64_t TEST_UB_LIMIT = floor(pow(TEST_DATA_LENGTH, (float)1/NDIM) );
const int TEST_VER = 0;

void mget_test(int num_get,
               std::string base_var_name, WADSDriver& wads_driver, TProfiler<int>& get_tprofiler,
               std::ofstream& log_file)
{
  int* data_ = (int*)malloc(TEST_DATA_LENGTH*sizeof(int) );
  uint64_t* gdim_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *lb_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++) {
    gdim_[i] = TEST_UB_LIMIT;
    lb_[i] = 0;
    ub_[i] = TEST_UB_LIMIT - 1;
  }
  
  std::ofstream mget_log_file("mget.log", std::ios::out | std::ios::app);
  if (!mget_log_file.is_open() ) {
    log_(ERROR, "mget_log_file is not open.")
    return;
  }
  // TProfiler<int> get_tprofiler;
  for (int i = 0; i < num_get; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    int counter = 0;
    while (1) {
      get_tprofiler.add_event(i, std::string("get_") + var_name);
      int err = wads_driver.get(true, var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_);
      get_tprofiler.end_event(i);
      if (err) {
        log_(ERROR, "wads_driver.get failed; var_name= " << var_name)
        ++counter;
        if (counter < 3) {
          log_(INFO, "will try wads_driver.get again; var_name= " << var_name)
        }
        else {
          log_(ERROR, "tried wads_driver.get for 3 times already, will return failure; var_name= " << var_name)
          // return;
          break;
        }
      }
      else {
        log_(INFO, "got; " << KV_TO_STR(var_name, TEST_VER) )
        break;
      }
    }
    sleep(INTER_ARR_TIME);
  }
  // log_(INFO, "get_tprofiler= \n" << get_tprofiler.to_str() )
  log_s(mget_log_file, INFO, "base_var_name= " << base_var_name << ", " << get_tprofiler.to_brief_str() )
  mget_log_file.close();
  
  // int data_length = patch::get_data_length(NDIM, gdim, lb, ub);
  // patch_ds::debug_print(var_name, TEST_VER, sizeof(int), NDIM, gdim_, lb_, ub_, data_, data_length);
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void poisson_mget_test(int num_get,
                       std::string base_var_name, WADSDriver& wads_driver, TProfiler<int>& get_tprofiler,
                       std::ofstream& log_file)
{
  int* data_ = (int*)malloc(TEST_DATA_LENGTH*sizeof(int) );
  uint64_t* gdim_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *lb_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++) {
    gdim_[i] = TEST_UB_LIMIT;
    lb_[i] = 0;
    ub_[i] = TEST_UB_LIMIT - 1;
  }
  
  std::ofstream mget_log_file("poisson_mget.log", std::ios::out | std::ios::app);
  if (!mget_log_file.is_open() ) {
    log_(ERROR, "mget_log_file is not open.")
    return;
  }
  // TProfiler<int> get_tprofiler;
  for (int i = 0; i < num_get; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    get_tprofiler.add_event(i, std::string("get_") + var_name);
    if (wads_driver.get(true, var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      log_(ERROR, "wads_driver.get failed for var_name= " << var_name)
      log_s(log_file, ERROR, "wads_driver.get failed for var_name= " << var_name)
      return;
    }
    log_(INFO, "got; " << KV_TO_STR(var_name, TEST_VER) )
    get_tprofiler.end_event(i);
    
    boost::math::exponential_distribution<float> inter_arr_time_dist(GET_ARR_RATE);
    float inter_arr_time = std::abs(quantile(inter_arr_time_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
    log_(INFO, "will sleep for " << inter_arr_time << "sec...")
    sleep(inter_arr_time);
  }
  // log_(INFO, "get_tprofiler= \n" << get_tprofiler.to_str() )
  // log_s(mget_log_file, INFO, "base_var_name= " << base_var_name << ", " << get_tprofiler.to_brief_str() )
  mget_log_file <<  "base_var_name= " << base_var_name << ", " << get_tprofiler.to_brief_str() << "\n";
  mget_log_file.close();
  
  // int data_length = patch::get_data_length(NDIM, gdim, lb, ub);
  // patch_ds::debug_print(var_name, TEST_VER, sizeof(int), NDIM, gdim_, lb_, ub_, data_, data_length);
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void mput_test(int num_put,
               std::string base_var_name, WADSDriver& wads_driver, TProfiler<int>& put_tprofiler,
               std::ofstream& log_file)
{
  int* data_ = (int*)malloc(TEST_DATA_LENGTH*sizeof(int) );
  uint64_t* gdim_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *lb_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  uint64_t *ub_ = (uint64_t*)malloc(NDIM*sizeof(uint64_t) );
  for (int i = 0; i < NDIM; i++) {
    gdim_[i] = TEST_UB_LIMIT;
    lb_[i] = 0;
    ub_[i] = TEST_UB_LIMIT - 1;
  }
  
  for (int i = 0; i < TEST_DATA_LENGTH; i++)
    data_[i] = i + 1;
  
  std::ofstream mput_log_file("mput.log", std::ios::out | std::ios::app);
  if (!mput_log_file.is_open() ) {
    log_(ERROR, "mput_log_file is not open.")
    return;
  }
  for (int i = 0; i < num_put; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    put_tprofiler.add_event(i, std::string("put_") + var_name);
    if (wads_driver.put(var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      log_(ERROR, "wads_driver.put failed for var_name= " << var_name)
      log_s(log_file, ERROR, "wads_driver.put failed for var_name= " << var_name)
      return;
    }
    log_(INFO, "put; " << KV_TO_STR(var_name, TEST_VER) )
    put_tprofiler.end_event(i);
    
    sleep(INTER_ARR_TIME);
  }
  // log_s(mput_log_file, INFO, "base_var_name= " << base_var_name << ", " << put_tprofiler.to_brief_str() )
  mput_log_file << "base_var_name= " << base_var_name << ", " << put_tprofiler.to_brief_str() << "\n";
  mput_log_file.close();
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

int main(int argc , char **argv)
{
  std::string temp;
  // FLAGS_logtostderr = true;
  // FLAGS_minloglevel = google::INFO;
  // FLAGS_log_dir = "/cac/u01/mfa51/Desktop/dataspaces_wa";
  // google::SetLogDestination(google::GLOG_INFO, "/cac/u01/mfa51/Desktop/dataspaces_wa");
  google::InitGoogleLogging(argv[0] );
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  std::ofstream log_file("main.log", std::ios::out | std::ios::app);
  if (!log_file.is_open() ) {
    log_(ERROR, "log_file is not open.")
    return 1;
  }
  
  // int rank, size;
  // MPI_Comm comm = MPI_COMM_WORLD;
  // MPI_Init(&argc, &argv);
  // MPI_Comm_rank(comm, &rank);
  // rank += 1; // Note: mpi starts ranking from 0
  // MPI_Comm_size(comm, &size);
  // std::cout << "main:: size= " << size << ", rank= " << rank << "\n";
  
  int cl_id = boost::lexical_cast<int>(opt_map["cl_id"] );
  // if (cl_id == 0) cl_id = rank;
  // MWADSDriver wads_driver(
  //   cl_id, boost::lexical_cast<int>(opt_map["base_client_id"] ), size, comm,
  //   intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ) + cl_id,
  //   opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
  MWADSDriver wads_driver(
    cl_id, boost::lexical_cast<int>(opt_map["base_client_id"] ), 1,
    intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ) + cl_id,
    opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
  TProfiler<int> putget_tprofiler;
  
  std::cout << "Enter for " << opt_map["type"] << "_test... \n";
  getline(std::cin, temp);
  if (str_cstr_equals(opt_map["type"], "mput") ) {
    mput_test(boost::lexical_cast<int>(opt_map["num_putget"] ),
              std::string("dummy_") + boost::lexical_cast<std::string>(cl_id), wads_driver, putget_tprofiler, log_file);
  }
  else if (str_cstr_equals(opt_map["type"], "mget") ) {
    mget_test(boost::lexical_cast<int>(opt_map["num_putget"] ),
              std::string("dummy_") + boost::lexical_cast<std::string>(cl_id), wads_driver, putget_tprofiler, log_file);
  }
  else {
    log_(ERROR, "unknown type= " << opt_map["type"] )
  }
  std::cout << "Enter. \n";
  getline(std::cin, temp);
  // 
  log_(INFO, "putget_tprofiler= \n" << putget_tprofiler.to_str() )
  log_s(log_file, INFO, putget_tprofiler.to_str() )
  log_file.close();
  // 
  return 0;
}
