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

#include <boost/math/distributions/normal.hpp>

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
  LZAlgo lz_algo;
  // PPMAlgo ppm_algo(2);
  // char access_seq_arr[] = {'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b',
  //                         'a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b','a','b' };
  // int access_seq_arr[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int access_seq_arr[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int access_seq_arr[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  // int access_seq_arr[] = {0, 1, 1, 0, 1, 0, 1, 0, 0, 0 };
  //aaababbbbbaabccddcbaaaa = 00010111110012233210000
  int access_seq_arr[] = {0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 2, 2, 3, 3, 2, 1, 0, 0, 0, 0 };
  
  float hit_rate;
  std::vector<int> access_seq_v(access_seq_arr, access_seq_arr + sizeof(access_seq_arr)/sizeof(*access_seq_arr) );
  std::vector<char> accuracy_seq_v;
  
  lz_algo.sim_prefetch_accuracy(hit_rate, 2, access_seq_v, accuracy_seq_v);
  std::cout << "LZ_ALGO:\n";
  std::cout << "hit_rate= " << hit_rate << "\n";
  std::cout << "access_seq= \n" << lz_algo.access_seq_to_str() << "\n";
  std::cout << "parse_tree_to_str= \n" << lz_algo.parse_tree_to_str() << "\n";
  std::cout << "parse_tree_to_pstr= \n" << lz_algo.parse_tree_to_pstr() << "\n";
  
  // ppm_algo.sim_prefetch_accuracy(hit_rate, 1, access_seq_v, accuracy_seq_v);
  // std::cout << "PPM_ALGO:\n";
  // std::cout << "hit_rate= " << hit_rate << "\n";
  // std::cout << "access_seq= \n" << ppm_algo.access_seq_to_str() << "\n";
  // std::cout << "parse_tree_to_pstr= \n" << ppm_algo.parse_tree_to_pstr() << "\n";
  
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

void handle_del(key_ver_pair kv)
{
  LOG(INFO) << "handle_del:: <key= " << kv.first << ", ver= " << kv.second << ">.";
}

void add_access(PBuffer* pbuffer, int p_id, int c)
{
  pbuffer->add_access(p_id, std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(c), 0) );
}

void prefetch_test()
{
  size_t buffer_size = 2;
  size_t app_context_size = 2;
  
  PBuffer pbuffer('a', buffer_size, app_context_size,
                  true, boost::bind(handle_prefetch, _1, _2),
                  boost::bind(handle_del, _1) );
  
  int p_id_[] = {0, 1};
  int num_p = sizeof(p_id_)/sizeof(*p_id_);
  int num_put = 20;
  std::map<int, int> p_id__v_map;
  
  for (int i = 0; i < num_p; i++) {
    int p_id = p_id_[i];
    for (int j = 0; j < num_put; j++) {
      pbuffer.reg_key_ver(p_id, std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(j), 0) );
    }
  }
  
  // int acced_p_id_[] = {0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0};
  // int acced_p_id_[] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1};
  // int acced_p_id_[] = {0,0,0,1,0,1,0,1,0,0,1,0,0,0,0};
  // int num_access = sizeof(acced_p_id_)/sizeof(*acced_p_id_);
  srand(time(NULL) );
  int num_access = 30 * (rand() % 10) / 10;
  int acced_p_id_[num_access];
  for (int i = 0; i < num_access; i++) {
    acced_p_id_[i] = rand() % num_p;
  }
  
  std::map<int, int> p_id__last_i_map;
  
  for (int i = 0; i < num_access; i++) {
    int p_id = acced_p_id_[i];
    int c = 0;
    if (p_id__last_i_map.count(p_id) != 0) { // contains
      ++p_id__last_i_map[p_id];
      c = p_id__last_i_map[p_id];
    }
    else {
      p_id__last_i_map[p_id] = c;
    }
    // pbuffer.add_access(p_id, std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(c), 0) );
    boost::thread t(add_access, &pbuffer, p_id, c);
    // add_access(&pbuffer, p_id, c);
  }
  
  // size_t num_keys = 1;
  // std::vector<key_ver_pair> key_ver_vector;
  // pbuffer.get_to_prefetch(num_keys, key_ver_vector);
  // std::cout << "get_to_prefetch returns:\n";
  // for (int i = 0; i < num_keys; i++) {
  //   std::cout << "<key= " << key_ver_vector[i].first << ", ver= " << key_ver_vector[i].second << "> \n";
  // }
  // 
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
  
  std::cout << "pbuffer= \n" << pbuffer.to_str();
}

void gen_scenario(int num_ds, char*& ds_id_,
                  int max_num_p, int max_num_c, int num_putget_mean, float put_rate_pseudo_mean, float get_rate_pseudo_mean,
                  int& num_p, int& num_c,
                  std::vector<char>& p_id__ds_id_vec, std::vector<char>& c_id__ds_id_vec,
                  std::vector<int>& p_id__num_put_vec, std::vector<int>& c_id__num_get_vec,
                  std::vector<float>& p_id__put_rate_vec, std::vector<float>& c_id__get_rate_vec,
                  std::vector<std::vector<float> >& p_id__inter_arr_time_vec_vec, std::vector<std::vector<float> >& c_id__inter_arr_time_vec_vec )
{
  srand(time(NULL) );
  
  int base_ascii_dec = 97;
  for (int i = 0; i < num_ds; i++) {
    ds_id_[i] = (char) (base_ascii_dec + i);
  }
  
  num_p = 2; // rand() % max_num_p + 1;
  for (int i = 0; i < num_p; i++) {
    boost::math::normal_distribution<float> num_putget_dist(num_putget_mean, 2);
    int num_put = quantile(num_putget_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ));
    
    // TODO: may be randomized
    p_id__ds_id_vec.push_back(ds_id_[0] ); 
    
    p_id__num_put_vec.push_back(num_put);
    p_id__put_rate_vec.push_back((float) (rand() % 10 + 5) * put_rate_pseudo_mean / 10);
  }
  
  num_c = num_p; // rand() % max_num_p + 1;
  for (int i = 0; i < num_c; i++) {
    // TODO: may be randomized
    c_id__ds_id_vec.push_back(ds_id_[1] );
    
    // int num_get = rand() % num_putget_mean + 1;
    // c_id__num_get_vec.push_back(num_get);
    c_id__num_get_vec.push_back(p_id__num_put_vec[i]);
    c_id__get_rate_vec.push_back((float) (rand() % 10 + 5) * get_rate_pseudo_mean / 10);
  }
  
  // 
  for (int p_id = 0; p_id < num_p; p_id++) {
    std::vector<float> inter_arr_time_vec;
    for (int i = 0; i < p_id__num_put_vec[p_id]; i++) {
      // boost::math::exponential_distribution exp_dist(p_id__put_rate_vec[p_id] );
      inter_arr_time_vec.push_back(
        -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / p_id__put_rate_vec[p_id] );
    }
    p_id__inter_arr_time_vec_vec.push_back(inter_arr_time_vec);
  }
  
  for (int c_id = 0; c_id < num_c; c_id++) {
    std::vector<float> inter_arr_time_vec;
    for (int i = 0; i < c_id__num_get_vec[c_id]; i++) {
      inter_arr_time_vec.push_back(
        // -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / c_id__get_rate_vec[p_id] )
        2 * (1.0 - (float)(rand() % 10) / 10) );
    }
    c_id__inter_arr_time_vec_vec.push_back(inter_arr_time_vec);
  }
}

void sim_test()
{
  int num_p, num_c;
  std::vector<char> p_id__ds_id_vec, c_id__ds_id_vec;
  std::vector<int> p_id__num_put_vec, c_id__num_get_vec;
  std::vector<float> p_id__put_rate_vec, c_id__get_rate_vec;
  std::vector<std::vector<float> > p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec;
  
  int num_ds = 2;
  char* ds_id_ = (char*) malloc(num_ds*sizeof(char) );
  gen_scenario(num_ds, ds_id_,
               5, 5, 20, 5, 0.2,
               num_p, num_c,
               p_id__ds_id_vec, c_id__ds_id_vec,
               p_id__num_put_vec, c_id__num_get_vec,
               p_id__put_rate_vec, c_id__get_rate_vec,
               p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec);
  
  // PCSim pc_sim(num_ds, ds_id_, 20, 2,
  //             num_p, num_c,
  //             p_id__ds_id_vec, c_id__ds_id_vec,
  //             p_id__num_put_vec, c_id__num_get_vec,
  //             p_id__put_rate_vec, c_id__get_rate_vec,
  //             p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec );
  
  // pc_sim.sim_all();
  
  std::map<int, std::map<int, float> > app_context_size___c_id__get_lperc_map_map;
  int app_context_size = 4;
  for (int i = 1; i <= app_context_size; i++) {
    PCSim pc_sim(num_ds, ds_id_, 20, i,
                num_p, num_c,
                p_id__ds_id_vec, c_id__ds_id_vec,
                p_id__num_put_vec, c_id__num_get_vec,
                p_id__put_rate_vec, c_id__get_rate_vec,
                p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec );
  
    pc_sim.sim_all();
    pc_sim.wait_for_threads();
    app_context_size___c_id__get_lperc_map_map[i] = pc_sim.get_c_id__get_lperc_map();
  }
  
  LOG(INFO) << "sim_test:: app_context_size___c_id__get_lperc_map_map= \n";
  for (int i = 1; i <= app_context_size; i++) {
    std::cout << "\t app_context_size= " << i << ": \n";
    std::map<int, float> c_id__get_lperc_map = app_context_size___c_id__get_lperc_map_map[i];
    for (int c_id = 0; c_id < num_c; c_id++)
      std::cout << "\t\t c_id= " << c_id << ": lperc= " << c_id__get_lperc_map[c_id] << "\n";
  }
  // free(ds_id_);
  
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
  palgo_test();
  // prefetch_test();
  // sim_test();
  
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
  
  // boost::math::normal_distribution<float> normal_dist(0, 1);
  // std::cout << "normal_dist.mean= " << normal_dist.mean() << "\n"
  //           << "normal_dist.standard_deviation= " << normal_dist.standard_deviation() << "\n";
  // std::cout << "normal_dist cdf= \n";
  // for (float x = -4; x < 4 + 0.5; x += 0.5)
  //   std::cout << "x= " << x << ", cdf(x)= " << cdf(normal_dist, x) << "\n";
  
  return 0;
}
