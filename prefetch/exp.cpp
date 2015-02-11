#include "lzprefetch.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
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
        opt_map[(char*)"type"] = optarg;
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
  std::cout << "opt_map=\n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  // std::map<char*, char*> opt_map = parse_opts(argc, argv);
  LZAlgo lz_algo(2, NULL);
  char access_seq_arr[] = {'a',  'a','a',  'a','b',  'a','b','a',  'a','b','b',  'b'};
  // for (int i = 0; i < (sizeof(access_seq_arr)/sizeof(*access_seq_arr)); i++) {
  //   lz_algo.add_access(access_seq_arr[i]);
  // }
  lz_algo.add_access('a');
  
  lz_algo.add_access('a');
  lz_algo.add_access('a');
  
  lz_algo.add_access('a');
  lz_algo.add_access('b');
  
  lz_algo.add_access('a');
  lz_algo.add_access('b');
  lz_algo.add_access('a');
  
  lz_algo.add_access('a');
  lz_algo.add_access('b');
  lz_algo.add_access('b');
  
  lz_algo.add_access('b');
  
  lz_algo.add_access('a');
  lz_algo.add_access('b');
  lz_algo.add_access('a');
  lz_algo.add_access('a');
  
  // lz_algo.print_parse_tree();
  lz_algo.pprint_parse_tree();
  
  return 0;
}