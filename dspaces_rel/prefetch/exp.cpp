#include "prefetch.h"
#include "sim.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <iostream>
#include <functional>
#include <string>

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
  LZAlgo lz_algo();
  PPMAlgo ppm_algo(2);
  // char access_seq_arr[] = {'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b',
  //                         'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b' };
  unsigned int access_seq_arr[] = {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 };
  
  float hit_rate;
  std::vector<unsigned int> access_seq_v(access_seq_arr, access_seq_arr + sizeof(access_seq_arr)/sizeof(*access_seq_arr) );
  std::vector<char> accuracy_seq_v;
  // lz_algo.sim_prefetch_accuracy(hit_rate, 2, access_seq_v);
  // LOG(INFO) << "main:: lz_algo; hit_rate= " << hit_rate;
  ppm_algo.sim_prefetch_accuracy(hit_rate, 1, access_seq_v, accuracy_seq_v);
  std::cout << "ppm_algo; hit_rate= " << hit_rate << "\n";
  std::cout << "access_seq= \n";
  std::cout << ppm_algo.access_seq_to_str();
  std::cout << "accuracy_seq= \n";
  for (int i = 0; i < accuracy_seq_v.size(); i++) {
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

void handle_prefetch(char ds_id, key_ver_pair kv)
{
  LOG(INFO) << "handle_prefetch:: ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
}

void prefetch_test()
{
  size_t buffer_size = 2;
  size_t app_context_size = 2;
  
  PBuffer pbuffer('a', buffer_size, 
                  true, boost::bind(handle_prefetch, _1, _2), 
                  app_context_size);
  
  std::string keys_[] = {"k1", "k2", "k3", "k4", "k5", "k6", "k7", "k8", "k9", "k10"};
  size_t keys_size = 10;
  for (int i = 0; i < keys_size; i++) {
    pbuffer.reg_key_ver(0, std::make_pair(keys_[i], 0) );
  }
  
  int num_access = 7;
  for (int i = 0; i < num_access; i++) {
    pbuffer.add_access(0, std::make_pair(keys_[i], 0) );
  }
  
  std::cout << "pbuffer= \n" << pbuffer.to_str();
  
  // size_t num_keys = 1;
  // std::vector<key_ver_pair> key_ver_vector;
  // pbuffer.get_to_prefetch(num_keys, key_ver_vector);
  // std::cout << "get_to_prefetch returns:\n";
  // for (int i = 0; i < num_keys; i++) {
  //   std::cout << "<key= " << key_ver_vector[i].first << ", ver= " << key_ver_vector[i].second << "> \n";
  // }
}

void sim_test(int num_ds, int max_num_app, int max_num_putget, float putget_rate_pseudo_mean)
{
  srand(time(NULL) );
  
  char ds_id_[num_ds];
  int base_ascii_dec = 97;
  for (int i = 0; i < num_ds; i++) {
    ds_id_[i] = (char) (base_ascii_dec + i);
  }
  
  int num_p = rand() % max_num_app + 1;
  std::vector<char> p_id__ds_id_vec;
  std::vector<int> p_id__num_put_vec;
  std::vector<float> p_id__put_rate_vector;
  for (int i = 0; i < num_p; i++) {
    int num_put = rand() % max_num_putget + 1;
    // TODO: may be randomized
    p_id__ds_id_vec.push_back(ds_id_[0] ); 
    
    p_id__num_put_vec.push_back(num_put);
    p_id__put_rate_vector.push_back((float) (rand() % 10 + 5) * putget_rate_pseudo_mean / 10);
  }
  
  int num_c = num_p; // rand() % max_num_app + 1;
  std::vector<char> c_id__ds_id_vec;
  std::vector<int> c_id__num_get_vec;
  std::vector<float> c_id__get_rate_vector;
  for (int i = 0; i < num_c; i++) {
    // TODO: may be randomized
    c_id__ds_id_vec.push_back(ds_id_[1] );
    
    // int num_get = rand() % max_num_putget + 1;
    // c_id__num_get_vec.push_back(num_get);
    c_id__num_get_vec.push_back(p_id__num_put_vec[i]);
    c_id__get_rate_vector.push_back((float) (rand() % 10 + 5) * putget_rate_pseudo_mean / 10);
  }
  
  PCSim pc_sim(num_ds, ds_id_,
               num_p, p_id__ds_id_vec, p_id__num_put_vec, p_id__put_rate_vector,
               num_c, c_id__ds_id_vec, c_id__num_get_vec, c_id__get_rate_vector );
  
  pc_sim.sim_all();
  // 
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  // palgo_test();
  // prefetch_test();
  sim_test(2, 5, 5, 0.5);
  
  // srand(time(NULL) );
  // float lambda = 0.1;
  // int num_exp = 1000;
  // float sum = 0;
  // for (int i = 0; i < num_exp; i++) {
  //   // std::cout << "main:: random number= " << static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) << "\n";
  //   sum += -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / lambda;
  // }
  // std::cout << "main:: sample mean= " << sum/num_exp << "\n";
  
  // std::string str = "Meet the new boss...";
  // std::string str_ = "Meet the new boss...";
  // std::string str_2 = "Meet the new boss l...";
  
  // unsigned int hash = patch_pre::hash_str(str.c_str() );
  // unsigned int hash_ = patch_pre::hash_str(str_.c_str() );
  // unsigned int hash_2 = patch_pre::hash_str(str_2.c_str() );

  // std::cout << "main:: hash= " << hash
  //           << ", hash_= " << hash_
  //           << ", hash_2= " << hash_2 << '\n';
  
  return 0;
}
