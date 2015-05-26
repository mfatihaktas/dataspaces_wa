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

#include "exp_patch.h"

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
  if (optind < argc) {
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
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<LZAlgo>();
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<ALZAlgo>();
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<PPMAlgo>(2);
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<POAlgo>();
  
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  //                   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  // int acc_[] = {0, 1, 1, 0, 1, 0, 1, 0, 0, 0 };
  //aaababbbbbaabccddcbaaaa = 00010111110012233210000
  // int acc_[] = {0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 2, 2, 3, 3, 2, 1, 0, 0, 0, 0 };
  // int acc_[] = {0, 0, 0, 1, 0, 1, 1 };
  // /* 
  size_t alphabet_size = 10;
  size_t acc_size = 1000; //sizeof(acc_)/sizeof(*acc_);
  int* acc_ = NULL;
  std::map<ACC_T, float> acc__arr_rate_map;
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  
  // gen_random_acc_seq(alphabet_size, acc_size, acc_);
  // gen_semirandom_acc_seq(alphabet_size, 1, acc_size, acc_);
  gen_poisson_acc_seq(alphabet_size, acc_size, acc__arr_rate_map, acc_);
  // gen_intermittent_poisson_acc_seq(alphabet_size, acc_size, acc__arr_rate_map, acc_);
  
  std::map<ACC_T, float> acc__emp_prob_map;
  get_emprical_dist(alphabet_size, acc_size, acc_, acc__emp_prob_map);
  std::cout << "palgo_test:: acc__emp_prob_map= \n" << patch_pre::map_to_str<ACC_T, float>(acc__emp_prob_map);
  std::vector<int> acc_v(acc_, acc_ + acc_size);
  std::cout << "palgo_test:: acc_= \n" << patch_pre::arr_to_str(acc_size, acc_) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // /* 
  // float hit_rate;
  // palgo_->sim_prefetch_accuracy(hit_rate, 1, acc_step_v, accuracy_v);
  // std::cout << "hit_rate= " << hit_rate << "\n";
  // std::cout << "access_seq= \n" << palgo_->access_seq_to_str() << "\n";
  // std::cout << "parse_tree_to_pstr= \n" << palgo_->parse_tree_to_pstr() << "\n";
  size_t cache_size = 9;
  
  boost::shared_ptr<PrefetchAlgo> lz_algo_ = boost::make_shared<LZAlgo>();
  float lz_hit_rate;
  std::vector<char> accuracy_v;
  lz_algo_->sim_prefetch_accuracy(lz_hit_rate, cache_size, acc_step_v, accuracy_v);
  std::cout << "LZ_ALGO:\n";
  std::cout << "hit_rate= " << lz_hit_rate << "\n";
  // std::cout << "access_seq= \n" << lz_algo_->access_seq_to_str() << "\n";
  // std::cout << "parse_tree_to_pstr= \n" << lz_algo_->parse_tree_to_pstr() << "\n";
  std::cout << "accuracy_v= \n" << patch_pre::vector_to_str<char>(accuracy_v) << "\n";
  
  // boost::shared_ptr<PrefetchAlgo> alz_algo_ = boost::make_shared<ALZAlgo>();
  // float alz_hit_rate;
  // accuracy_v.clear();
  // alz_algo_->sim_prefetch_accuracy(alz_hit_rate, cache_size, acc_step_v, accuracy_v);
  // std::cout << "ALZ_ALGO:\n";
  // std::cout << "hit_rate= " << alz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << alz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vector_to_str<char>(accuracy_v) << "\n";
  
  // boost::shared_ptr<PrefetchAlgo> ppm_algo_ = boost::make_shared<PPMAlgo>(2);
  // float ppm_hit_rate;
  // accuracy_v.clear();
  // ppm_algo_->sim_prefetch_accuracy(ppm_hit_rate, cache_size, acc_step_v, accuracy_v);
  // std::cout << "PPM_ALGO:\n";
  // std::cout << "hit_rate= " << ppm_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vector_to_str<char>(accuracy_v) << "\n";
  
  // boost::shared_ptr<PrefetchAlgo> po_algo_ = boost::make_shared<POAlgo>();
  // float po_hit_rate;
  // accuracy_v.clear();
  // po_algo_->sim_prefetch_accuracy(po_hit_rate, cache_size, acc_step_v, accuracy_v);
  // std::cout << "PO_ALGO:\n";
  // std::cout << "hit_rate= " << po_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vector_to_str<char>(accuracy_v) << "\n";
  
  // std::cout << "lz_hit_rate= " << lz_hit_rate << "\n"
  //           << "alz_hit_rate= " << alz_hit_rate << "\n"
  //           << "ppm_hit_rate= " << ppm_hit_rate << "\n"
  //           << "po_hit_rate= " << po_hit_rate << "\n";
  
  free(acc_);
  // */
  
  /* 
  for (int i = 0; i < sizeof(acc_)/sizeof(*acc_); i++) {
    palgo_->add_access(acc_[i] );
    
    std::cout << "access_seq= " << palgo_->access_seq_to_str();
    std::cout << "parse_tree_to_pstr= \n" << palgo_->parse_tree_to_pstr();
    
    size_t num_acc = 1;
    ACC_T* keys_;
    palgo_->get_to_prefetch(num_acc, keys_, std::vector<ACC_T>(), std::vector<ACC_T>() );
    std::cout << "keys_= " << patch_pre::arr_to_str<ACC_T>(num_acc, keys_) << "\n";
    
    // std::map<int, float> acc__emp_prob_map;
    // palgo_->get_acc__emp_prob_map_for_prefetch(acc__emp_prob_map);
    // std::cout << "acc__emp_prob_map= \n" << patch_pre::map_to_str<int, float>(acc__emp_prob_map) << "\n";
  }
  */
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
  
  for (int i = 0; i < num_p; i++) {
    int p_id = p_id_[i];
    for (int j = 0; j < num_put; j++)
      pbuffer.reg_key_ver(p_id, std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(j), 0) );
  }
  
  // int acced_p_id_[] = {0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0};
  // int acced_p_id_[] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1};
  // int acced_p_id_[] = {0,0,0,1,0,1,0,1,0,0,1,0,0,0,0};
  // int num_acc = sizeof(acced_p_id_)/sizeof(*acced_p_id_);
  srand(time(NULL) );
  int num_acc = 30 * (rand() % 10) / 10;
  int acced_p_id_[num_acc];
  for (int i = 0; i < num_acc; i++)
    acced_p_id_[i] = rand() % num_p;
  
  std::map<int, int> p_id__last_i_map;
  
  for (int i = 0; i < num_acc; i++) {
    int p_id = acced_p_id_[i];
    if (p_id__last_i_map.count(p_id) == 0)
      p_id__last_i_map[p_id] = 0;
    
    // pbuffer.add_access(p_id, std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(p_id__last_i_map[p_id] ), 0) );
    boost::thread t(add_access, &pbuffer, p_id, p_id__last_i_map[p_id] );
    // add_access(&pbuffer, p_id, p_id__last_i_map[p_id]);
    
    p_id__last_i_map[p_id]++;
  }
  
  // size_t num_acc = 1;
  // std::vector<key_ver_pair> key_ver_vector;
  // pbuffer.get_to_prefetch(num_acc, key_ver_vector, std::vector<ACC_T>(), std::vector<ACC_T>() );
  // std::cout << "get_to_prefetch returns= \n";
  // for (int i = 0; i < num_acc; i++)
  //   std::cout << "<key= " << key_ver_vector[i].first << ", ver= " << key_ver_vector[i].second << "> \n";
  
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
  
  std::cout << "pbuffer= \n" << pbuffer.to_str();
}

void sim_test()
{
  int num_p, num_c;
  std::vector<char> p_id__ds_id_vec, c_id__ds_id_vec;
  std::vector<int> p_id__num_put_vec, c_id__num_get_vec;
  std::vector<float> p_id__put_rate_vec, c_id__get_rate_vec;
  std::vector<std::vector<float> > p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec;
  
  int num_ds = 2;
  int max_num_p = 5;
  int max_num_c = 5;
  int num_putget_mean = 20;
  float put_rate_mean = 5;
  float get_rate_mean = 0.2;
  
  char* ds_id_ = (char*) malloc(num_ds*sizeof(char) );
  gen_scenario(num_ds, ds_id_,
              max_num_p, max_num_c, num_putget_mean, put_rate_mean, get_rate_mean,
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
  int app_context_size = 1;
  for (int i = 0; i <= app_context_size; i++) {
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
  for (int i = 0; i <= app_context_size; i++) {
    std::cout << "\t app_context_size= " << i << ": \n";
    std::map<int, float> c_id__get_lperc_map = app_context_size___c_id__get_lperc_map_map[i];
    for (int c_id = 0; c_id < num_c; c_id++)
      std::cout << "\t\t c_id= " << c_id << ": lperc= " << c_id__get_lperc_map[c_id] << "\n";
  }
  free(ds_id_);
  
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
