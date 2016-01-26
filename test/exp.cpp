#include "ds_test.h"

#include <getopt.h>
#include <limits.h>

std::map<std::string, std::string> parse_opts(int argc, char** argv__)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"app_id", optional_argument, NULL, 1},
    {"num_dscnodes", optional_argument, NULL, 2},
    {"num_putget_threads", optional_argument, NULL, 3},
    {"data_size", optional_argument, NULL, 4},
    {0, 0, 0, 0}
  };
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv__, "s", long_options, &option_index);

    if (c == -1) // Detect the end of the options.
      break;
    
    switch (c) {
      case 0:
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["app_id"] = optarg;
        break;
      case 2:
        opt_map["num_dscnodes"] = optarg;
        break;
      case 3:
        opt_map["num_putget_threads"] = optarg;
        break;
      case 4:
        opt_map["data_size"] = optarg;
        break;
      default:
        break;
    }
  }
  if (optind < argc) {
    printf ("non-option ARGV__-elements: ");
    while (optind < argc)
      printf ("%s ", argv__[optind++] );
    putchar ('\n');
  }
  // 
  log(INFO, "opt_map= \n" << patch_test::map_to_str<>(opt_map) )
  return opt_map;
}

int main(int argc , char** argv__)
{
  std::string temp;
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv__);
  int num_dscnodes = atoi(opt_map["num_dscnodes"].c_str() );
  int app_id = atoi(opt_map["app_id"].c_str() );
  int num_putget_threads = atoi(opt_map["num_putget_threads"].c_str() );
  uint64_t data_size = atof(opt_map["data_size"].c_str() );
  log(INFO, "data_size= " << data_size)
  // 
  DSTest ds_test(num_dscnodes, app_id, num_putget_threads);
  // DSDriver ds_driver(num_dscnodes, app_id);
  
  if (str_cstr_equals(opt_map["type"], "put_test") ) {
    ds_test.run_multithreaded_put_test("thread", data_size);
    // ds_test.exp_put(data_size);
    
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else if (str_cstr_equals(opt_map["type"], "get_test") ) {
    // sleep(3); // wait for getter to lock on things first
    std::cout << "Enter for run_multithreaded_get_test \n";
    getline(std::cin, temp);
    ds_test.run_multithreaded_get_test("thread", data_size);
    
    // std::cout << "Enter for exp_get... \n";
    // getline(std::cin, temp);
    // ds_test.exp_get(data_size);
    
    std::cout << "Enter \n";
    getline(std::cin, temp);
  }
  else {
    log(ERROR, "unknown type= " << opt_map["type"] )
  }
  
  return 0;
}
