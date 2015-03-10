#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "dataspaces_wa.h"

template <typename T>
void free_all(int num, ...)
{
  va_list arguments;                     // A place to store the list of arguments

  va_start ( arguments, num );           // Initializing arguments to store all values after num
  
  for ( int x = 0; x < num; x++ )        // Loop until all numbers are added
    va_arg ( arguments, T* );
  
  va_end ( arguments );                  // Cleans up the list
}

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

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"num_dscnodes", optional_argument, NULL, 1},
    {"app_id", optional_argument, NULL, 2},
    {"num_putget", optional_argument, NULL, 3},
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
        opt_map[(char*)"num_dscnodes"] = optarg;
        break;
      case 2:
        opt_map[(char*)"app_id"] = optarg;
        break;
      case 3:
        opt_map[(char*)"num_putget"] = optarg;
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
  std::cout << "opt_map=\n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it) {
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

#define TEST_SIZE 512
#define TEST_NDIM 3
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 0
#define TEST_SGDIM 1024

void multi_get_test(int num_gets, std::string base_var_name, WADspacesDriver& wads_driver)
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
  
  for (int i = 0; i < num_gets; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    std::cout << "\n"; // to see better on terminal
    if (wads_driver.get(true, "int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) ) {
      LOG(ERROR) << "multi_get_test:: wads_driver.get failed for var_name= " << var_name;
      return;
    }
  }
  // size_t data_length = patch::get_data_length(TEST_NDIM, gdim, lb, ub);
  // debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, data_length);
  
  free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

void multi_put_test(int num_puts, std::string base_var_name, WADspacesDriver& wads_driver)
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
  
  for (int i = 0; i < num_puts; i++) {
    std::string var_name = base_var_name + "_" + boost::lexical_cast<std::string>(i);
    std::cout << "\n"; // to see better on terminal
    if (wads_driver.put("int", var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_) ) {
      LOG(ERROR) << "multi_put_test:: wads_driver.put failed for var_name= " << var_name;
      return;
    }
  }
  // debug_print(var_name, TEST_VER, sizeof(int), TEST_NDIM, gdim_, lb_, ub_, data_, patch::get_data_length(TEST_NDIM, gdim_, lb_, ub_) );
  
  free_all<uint64_t>(3, gdim_, lb_, ub_);
  free(data_);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  
  int num_dscnodes = boost::lexical_cast<int>(opt_map[(char*)"num_dscnodes"]);
  int app_id = boost::lexical_cast<int>(opt_map[(char*)"app_id"]);
  int num_putget = boost::lexical_cast<int>(opt_map[(char*)"num_putget"]);
  
  TProfiler<std::string> tprofiler;
  if (strcmp(opt_map[(char*)"type"], (char*)"mput") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    std::cout << "Enter for multi_put_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("multi_put_test", "multi_put_test");
    multi_put_test(num_putget, "dummy", wads_driver);
    tprofiler.end_event("multi_put_test");
    // multi_put_test(num_putget, "dummy2", wads_driver);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"mget") == 0) {
    WADspacesDriver wads_driver(app_id, num_dscnodes-1);
    
    std::cout << "Enter for multi_get_test...\n";
    getline(std::cin, temp);
    
    tprofiler.add_event("multi_get_test", "multi_get_test");
    multi_get_test(num_putget, "dummy", wads_driver);
    tprofiler.end_event("multi_get_test");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else {
    LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  std::cout << "main:: tprofiler= \n" << tprofiler.to_str();
  // 
  return 0;
}