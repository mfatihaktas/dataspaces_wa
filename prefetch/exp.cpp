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
  for (std::map<char*, char*>::iterator it = opt_map.begin(); it != opt_map.end(); ++it) {
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  // std::map<float, char> den_map;
  // den_map[0.5] = 'a';
  // den_map[0.5] = 'b';
  // den_map[0.5] = 'c';
  
  // std::cout << "den_map: \n";
  // for (std::map<float, char>::iterator it = den_map.begin(); it != den_map.end(); ++it) {
  //   std::cout << it->first << " => " << it->second << '\n';
  // }
  
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  // LZAlgo lz_algo(2, NULL, true, 2);
  PPMAlgo ppm_algo(2, NULL, 2);
  // char access_seq_arr[] = {'a',  'a','a',  'a','b',  'a','b','a',  'a','b','b',  'b'};
  // char access_seq_arr[] = {'a','b','a','b','a','b','a','b',  'a','b','a','b','a','b','a','b','a','b','a','b','a'};
  char access_seq_arr[] = {'a','b','b','a','b','a','b','a','b'};
  // char access_seq_arr[] = {'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b',
  //                         'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b'
  // };
  // float hit_rate;
  // std::vector<char> access_seq_v (access_seq_arr, access_seq_arr + sizeof(access_seq_arr)/sizeof(*access_seq_arr) );
  // lz_algo.sim_prefetch_accuracy(hit_rate, 2, access_seq_v);
  // LOG(INFO) << "main:: hit_rate= " << hit_rate;
  
  for (int i = 0; i < sizeof(access_seq_arr)/sizeof(*access_seq_arr); i++) {
  // // for (int i = 0; i < 10; i++) {
    // lz_algo.add_access(access_seq_arr[i]);
    ppm_algo.add_access(access_seq_arr[i]);
    
  //   lz_algo.print_access_seq();
  //   lz_algo.pprint_parse_tree();
    
  //   std::map<char, float> key_prob_map;
  //   lz_algo.get_key_prob_map_for_prefetch(key_prob_map);
  //   std::cout << "key_prob_map= ";
  //   for (std::map<char, float>::iterator it = key_prob_map.begin(); it != key_prob_map.end(); it++) {
  //     std::cout << it->first << ": " << it->second << ", ";
  //   }
  //   std::cout << "\n";
  }
  // lz_algo.pprint_parse_tree();
  ppm_algo.pprint_parse_tree();
  
  return 0;
}
