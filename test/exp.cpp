#include "ds_test.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

std::map<std::string, std::string> parse_opts(int argc, char** argv)
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
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s", long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
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
  std::cout << "opt_map= \n";
  for (std::map<std::string, std::string>::iterator it = opt_map.begin(); it != opt_map.end(); ++it) {
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  // 
  DSTest ds_test(atoi(opt_map["num_dscnodes"].c_str()), atoi(opt_map["app_id"].c_str()),
                 atoi(opt_map["num_putget_threads"].c_str()) );
  
  if (opt_map["type"].compare("put_test") == 0) {
    ds_test.run_multithreaded_put_test("put_thread");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if (opt_map["type"].compare("get_test") == 0) {
    ds_test.run_multithreaded_get_test("get_thread");
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else {
    std::cerr << "main:: unknown type= " << opt_map["type"] << "\n";
  }
  
  return 0;
}