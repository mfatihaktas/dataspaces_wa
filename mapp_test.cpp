#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

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
    {"num_peer", optional_argument, NULL, 3},
    {"lcontrol_lintf", optional_argument, NULL, 4},
    {"lcontrol_lport", optional_argument, NULL, 5},
    {"join_lcontrol_lip", optional_argument, NULL, 6},
    {"join_lcontrol_lport", optional_argument, NULL, 7},
    {"num_app", optional_argument, NULL, 8},
    {"num_putget", optional_argument, NULL, 9},
    {0, 0, 0, 0}
  };
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv, "s",
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
        opt_map["num_peer"] = optarg;
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
        opt_map["num_app"] = optarg;
        break;
      case 9:
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
  std::cout << "parse_opts:: opt_map= \n" << patch::map_to_str<>(opt_map);
  
  return opt_map;
}

// 10*1024*
#define TEST_SIZE 1024
#define TEST_DATASIZE pow(TEST_SIZE, NDIM)
#define TEST_VER 0
#define TEST_SGDIM TEST_SIZE

/*******************************************  get  ************************************************/
void mget_test(MPI_Comm& mpi_comm, int cl_id, int num_get, std::string base_var_name, std::map<std::string, std::string> opt_map)
{
  log_(INFO, "started; cl_id= " << cl_id)
  
  boost::shared_ptr<MWADSDriver> wads_driver_ = boost::make_shared<MWADSDriver>(
    cl_id, boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_peer"] ), mpi_comm,
    intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ) + cl_id,
    opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
  
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
    get_time_profiler.add_event(i, var_name);
    if (wads_driver_->get(true, var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      log_(ERROR, "wads_driver_->get failed; var_name= " << var_name)
      return;
    }
    get_time_profiler.end_event(i);
    
    // sleep(inter_get_time_sec);
  }
  log_(INFO, "get_time_profiler= \n" << get_time_profiler.to_str() )
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
  
  log_(INFO, "Enter to end; cl_id= " << cl_id)
  std::string temp;
  getline(std::cin, temp);
}

void mapp_mget_test(MPI_Comm& mpi_comm, int num_app, int num_get, std::string base_var_name, std::map<std::string, std::string> opt_map)
{
  log_(INFO, "started; num_app= " << num_app << ", num_get= " << num_get << ", base_var_name= " << base_var_name)

  for (int i = 1; i <= num_app; i++) {
    std::string app_base_var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    boost::thread t(mget_test, mpi_comm, i, num_get, app_base_var_name, opt_map);
  }
  
  log_(INFO, "Enter to end...")
  std::string temp;
  getline(std::cin, temp);
}

/*******************************************  put  ************************************************/
void mput_test(MPI_Comm& mpi_comm, int cl_id, int num_put, std::string base_var_name, std::map<std::string, std::string> opt_map)
{
  log_(INFO, "started; cl_id= " << cl_id)
  std::cout << "sleeping for " << cl_id << "sec \n";
  sleep(cl_id);
  
  boost::shared_ptr<MWADSDriver> wads_driver_ = boost::make_shared<MWADSDriver>(
    cl_id, boost::lexical_cast<int>(opt_map["base_client_id"] ), boost::lexical_cast<int>(opt_map["num_peer"] ), mpi_comm,
    intf_to_ip(opt_map["lcontrol_lintf"] ), boost::lexical_cast<int>(opt_map["lcontrol_lport"] ) + cl_id,
    opt_map["join_lcontrol_lip"], boost::lexical_cast<int>(opt_map["join_lcontrol_lport"] ) );
  
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
    if (wads_driver_->put(var_name, TEST_VER, "int", sizeof(int), NDIM, gdim_, lb_, ub_, data_) ) {
      log_(ERROR, "wads_driver_->put failed; var_name= " << var_name)
      return;
    }
    // float inter_put_time = -1 * log(1.0 - std::abs(static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) - 0.001) ) / put_rate
    // sleep(inter_put_time_sec);
  }
  
  patch::free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
  
  log_(INFO, "Enter to end...; cl_id= " << cl_id)
  std::string temp;
  getline(std::cin, temp);
}

void mapp_mput_test(MPI_Comm& mpi_comm, int num_app, int num_put, std::string base_var_name, std::map<std::string, std::string> opt_map)
{
  log_(INFO, "started; num_app= " << num_app << ", num_put= " << num_put << ", base_var_name= " << base_var_name)
  
  for (int i = 1; i <= num_app; i++) {
    std::string app_base_var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    boost::thread t(mput_test, mpi_comm, i, num_put, app_base_var_name, opt_map);
  }
  
  log_(INFO, "Enter to end...")
  std::string temp;
  getline(std::cin, temp);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  int nprocs, rank;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Comm mpi_comm = MPI_COMM_WORLD;
  
  TProfiler<std::string> tprofiler;
  if (str_cstr_equals(opt_map["type"], "mput") ) {
    std::cout << "Enter for mapp_mput_test... \n";
    getline(std::cin, temp);
    
    // tprofiler.add_event("mapp_mput_test", "mapp_mput_test");
    mapp_mput_test(mpi_comm, boost::lexical_cast<int>(opt_map["num_app"] ), boost::lexical_cast<int>(opt_map["num_putget"] ), "dummy", opt_map);
    // tprofiler.end_event("mapp_mput_test");
  }
  else if (str_cstr_equals(opt_map["type"], "mget") ) {
    std::cout << "Enter for mapp_mget_test... \n";
    getline(std::cin, temp);
    
    // tprofiler.add_event("mapp_mget_test", "mapp_mget_test");
    mapp_mget_test(mpi_comm, boost::lexical_cast<int>(opt_map["num_app"] ), boost::lexical_cast<int>(opt_map["num_putget"] ), "dummy", opt_map);
    // tprofiler.end_event("mapp_mget_test");
  }
  else
    log_(ERROR, "unknown type= " << opt_map["type"] )
  
  std::cout << "tprofiler= \n" << tprofiler.to_str();
  // 
  return 0;
}
