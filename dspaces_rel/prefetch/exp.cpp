#include "prefetch.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

namespace patch
{
  std::string str_str_map_to_str(std::map<std::string, std::string> str_map)
  {
    std::stringstream ss;
    for (std::map<std::string, std::string>::const_iterator it = str_map.begin(); 
         it != str_map.end(); it++){
      ss << "\t" << it->first << ": " << it->second << "\n";
    }
    
    return ss.str();
  }
}

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

    if (c == -1) // Detect the end of the options.
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

void palgo_test()
{
  LZAlgo lz_algo(2, NULL);
  PPMAlgo ppm_algo(2, NULL, 2);
  // char access_seq_arr[] = {'a',  'a','a',  'a','b',  'a','b','a',  'a','b','b',  'b'};
  // char access_seq_arr[] = {'a','b','a','b','a','b','a','b',  'a','b','a','b','a','b','a','b','a','b','a','b','a'};
  // char access_seq_arr[] = {'a','b','b','a','b','a','b','a','b'};
  // char access_seq_arr[] = {'a','b','b','a','b','a','b','a','b'};
  char access_seq_arr[] = {'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b',
                           'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b' };
  // };
  float hit_rate;
  std::vector<char> access_seq_v(access_seq_arr, access_seq_arr + sizeof(access_seq_arr)/sizeof(*access_seq_arr) );
  std::vector<char> accuracy_seq_v;
  // lz_algo.sim_prefetch_accuracy(hit_rate, 2, access_seq_v);
  // LOG(INFO) << "main:: lz_algo; hit_rate= " << hit_rate;
  ppm_algo.sim_prefetch_accuracy(hit_rate, 1, access_seq_v, accuracy_seq_v);
  std::cout << "ppm_algo; hit_rate= " << hit_rate;
  std::cout << "access_seq= \n";
  std::cout << ppm_algo.access_seq_to_str();
  std::cout << "accuracy_seq= \n";
  for (int i =0; i < accuracy_seq_v.size(); i++) {
    std::cout << accuracy_seq_v[i] << ",";
  }
  std::cout << "\n";
  
  // for (int i = 0; i < sizeof(access_seq_arr)/sizeof(*access_seq_arr); i++) {
  //   // lz_algo.add_access(access_seq_arr[i]);
  //   ppm_algo.add_access(access_seq_arr[i]);
    
  //   // std::cout << "parse_tree= \n" << lz_algo.parse_tree_to_pstr();
  //   // std::cout << "parse_tree= \n" << ppm_algo.parse_tree_to_pstr();
  //   std::cout << "access_seq= " << ppm_algo.access_seq_to_str();
    
  //   size_t num_keys = 1;
  //   char* keys_;
  //   ppm_algo.get_to_prefetch(num_keys, keys_);
  //   std::cout << "keys_= ";
  //   for (int i = 0; i < num_keys; i++) {
  //     std::cout << keys_[i] << ",";
  //   }
  //   std::cout << "\n";
    
  //   // std::map<char, float> key_prob_map;
  //   // // lz_algo.get_key_prob_map_for_prefetch(key_prob_map);
  //   // ppm_algo.get_key_prob_map_for_prefetch(key_prob_map);
  //   // std::cout << "key_prob_map= ";
  //   // for (std::map<char, float>::iterator it = key_prob_map.begin(); it != key_prob_map.end(); it++) {
  //   //   std::cout << it->first << ": " << it->second << ", ";
  //   // }
  //   // std::cout << "\n";
  // }
  // std::cout << "parse_tree= \n" << lz_algo.parse_tree_to_pstr();
  // std::cout << "parse_tree= \n" << ppm_algo.parse_tree_to_pstr();
}

void handle_prefetch(std::map<std::string, std::string> pmap)
{
  LOG(INFO) << "handle_prefetch:: pmap= \n" << patch::str_str_map_to_str(pmap);
}

void prefetch_test()
{
  size_t buffer_size = 2;
  char alphabet_[] = {'a', 'b'};
  int alphabet_size = sizeof(alphabet_)/sizeof(*alphabet_);
  size_t context_size = 2;
  
  PBuffer pbuffer(true, buffer_size, boost::bind(handle_prefetch, _1), 
                  alphabet_, alphabet_size, context_size);
  
  std::string keys_[] = {"k1", "k2", "k3", "k4", "k5", "k6", "k7", "k8", "k9", "k10"};
  size_t keys_size = 10;
  for (int i = 0; i < keys_size; i++) {
    pbuffer.reg_key_ver__pkey_pver_pair(keys_[i], 0);
  }
  
  int num_access = 7;
  for (int i = 0; i < num_access; i++) {
    pbuffer.add_access(keys_[i], 0);
  }
  
  std::cout << "pbuffer= " << pbuffer.to_str();
  
  // size_t num_keys = 1;
  // std::vector<key_ver_pair> key_ver_vector;
  // pbuffer.get_to_prefetch(num_keys, key_ver_vector);
  // std::cout << "get_to_prefetch returns:\n";
  // for (int i = 0; i < num_keys; i++) {
  //   std::cout << "<key= " << key_ver_vector[i].first << ", ver= " << key_ver_vector[i].second << "> \n";
  // }
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  // palgo_test();
  prefetch_test();
  
  return 0;
}
