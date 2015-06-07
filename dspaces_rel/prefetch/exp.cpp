#include "prefetch.h"
#include "sim.h"

#include <getopt.h>

#include "patch_exp.h"

#include <algorithm>

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

void test_rand_shuffle_train()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<PrefetchAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  // title_v.push_back("ppm_1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm_2");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  // title_v.push_back("ppm_3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm_4");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(5) );
  title_v.push_back("ppm_5");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(6) );
  title_v.push_back("ppm_6");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(8) );
  title_v.push_back("ppm_8");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(12) );
  // title_v.push_back("ppm_12");
  // palgo_v.push_back(boost::make_shared<POAlgo>() );
  // title_v.push_back("po");
  
  std::vector<boost::shared_ptr<MPrefetchAlgo> > mpalgo_v;
  std::map<PREFETCH_T, float> prefetch_t__weight_map;
  prefetch_t__weight_map[W_LZ] = 0.33;
  prefetch_t__weight_map[W_PPM] = 0.33;
  prefetch_t__weight_map[W_PO] = 0.33;
  // mpalgo_v.push_back(boost::make_shared<MPrefetchAlgo>(W_MP_WEIGHT, prefetch_t__weight_map) );
  // title_v.push_back("mp_w_weight");
  // mpalgo_v.push_back(boost::make_shared<MPrefetchAlgo>(W_MP_MAX, prefetch_t__weight_map) );
  // title_v.push_back("mp_w_max");
  
  int num_algo = palgo_v.size() + mpalgo_v.size();
  
  int alphabet_size = 10;
  int num_acc = 100;
  std::vector<int> acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  // for (int i = 0; i < num_acc; i++)
  //   acc_v.push_back(i % alphabet_size);
  std::cout << "acc_v= " << patch_pre::vec_to_str<>(acc_v) << "\n";
  // acc_v_to_acc_step_v(acc_v, acc_step_v);
  
  int shuffle_width = 2;
  float shuffle_prob = 0.5;
  std::vector<int> shuffle_indices;
  // for (int i = shuffle_width; i < num_acc; i += (2*shuffle_width + 1) )
  for (int i = 0; i < num_acc/10; i++)
    shuffle_indices.push_back(rand() % num_acc);
    // shuffle_indices.push_back(i);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_run = 50;
  for (int i = 1; i <= num_run; i++) {
    for (int j = 0; j < num_run; j++) {
      random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
      // std::cout << "for training; acc_v= " << patch_pre::vec_to_str<>(acc_v) << "\n";
      
      for (std::vector<boost::shared_ptr<PrefetchAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++)
        (*it)->train(acc_v);
      for (std::vector<boost::shared_ptr<MPrefetchAlgo> >::iterator it = mpalgo_v.begin(); it != mpalgo_v.end(); it++)
        (*it)->train(acc_v);
    }
    // std::random_shuffle(acc_v_.begin(), acc_v_.end() );
    // for (std::vector<boost::shared_ptr<PrefetchAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++)
    //   (*it)->train(acc_v_);
    // for (std::vector<boost::shared_ptr<MPrefetchAlgo> >::iterator it = mpalgo_v.begin(); it != mpalgo_v.end(); it++)
    //   (*it)->train(acc_v_);
    
    // std::random_shuffle(acc_v.begin(), acc_v.end() );
    random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
    acc_step_v.clear();
    acc_v_to_acc_step_v(acc_v, acc_step_v);
    std::cout << "for prediction; acc_v= " << patch_pre::vec_to_str<>(acc_v) << "\n";
    
    int algo_id = 0;
    for (std::vector<boost::shared_ptr<PrefetchAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
      accuracy_v.clear();
      sim_prefetch_accuracy<PrefetchAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(i);
      // (*it)->reset();
    }
  
    for (std::vector<boost::shared_ptr<MPrefetchAlgo> >::iterator it = mpalgo_v.begin(); it != mpalgo_v.end(); it++, algo_id++) {
      accuracy_v.clear();
      sim_prefetch_accuracy<MPrefetchAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(i);
      // (*it)->reset();
    }
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Hit rate after rand_shuffle training for various palgo "
                << "alph_size= " << boost::lexical_cast<std::string>(alphabet_size)
                << ", num_acc= " << boost::lexical_cast<std::string>(num_acc);
  
  std::string out_url = ""; // "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/img/fig_rand_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Run index", "Hit rate",
                   plot_title_ss.str(), out_url);
}

void plot_palgo_comparison()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<PrefetchAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm_1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm_2");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  title_v.push_back("ppm_3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm_4");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  // title_v.push_back("ppm_4");
  // palgo_v.push_back(boost::make_shared<POAlgo>() );
  // title_v.push_back("po");
  
  std::vector<boost::shared_ptr<MPrefetchAlgo> > mpalgo_v;
  std::map<PREFETCH_T, float> prefetch_t__weight_map;
  prefetch_t__weight_map[W_LZ] = 0.33;
  prefetch_t__weight_map[W_PPM] = 0.33;
  prefetch_t__weight_map[W_PO] = 0.33;
  // mpalgo_v.push_back(boost::make_shared<MPrefetchAlgo>(W_MP_WEIGHT, prefetch_t__weight_map) );
  // title_v.push_back("mp_w_weight");
  mpalgo_v.push_back(boost::make_shared<MPrefetchAlgo>(W_MP_MAX, prefetch_t__weight_map) );
  title_v.push_back("mp_w_max");
  
  int num_algo = palgo_v.size() + mpalgo_v.size();
  
  int alphabet_size = 4;
  int num_acc = 1000;
  std::vector<int> acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_run = 150;
  for (int i = 1; i <= num_run; i++) {
    // acc_v.clear();
    // acc_step_v.clear();
    
    // acc__arr_rate_map.clear();
    // for (ACC_T a = 0; a < alphabet_size; a++)
    //   acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
    
    // gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
    // gen_intermittent_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
    std::map<ACC_T, float> acc__emp_prob_map;
    get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
    std::cout << "run_i= " << i << ", acc__emp_prob_map= \n" << patch_pre::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
    
    // std::cout << "acc_v= \n" << patch_pre::vec_to_str(acc_v) << "\n";
    // acc_v_to_acc_step_v(acc_v, acc_step_v);
    
    int algo_id = 0;
    for (std::vector<boost::shared_ptr<PrefetchAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
      // (*it)->reset();
      accuracy_v.clear();
      sim_prefetch_accuracy<PrefetchAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      // std::cout << "title= " << title_v[algo_id] << ", hit_rate= " << hit_rate << "\n";
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(i);
    }

    for (std::vector<boost::shared_ptr<MPrefetchAlgo> >::iterator it = mpalgo_v.begin(); it != mpalgo_v.end(); it++, algo_id++) {
      // (*it)->reset();
      accuracy_v.clear();
      sim_prefetch_accuracy<MPrefetchAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      // std::cout << "title= " << title_v[algo_id] << ", hit_rate= " << hit_rate << "\n";
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(i);
    }
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Hit rate for various palgo "
                << "alph_size= " << boost::lexical_cast<std::string>(alphabet_size)
                << ", num_acc= " << boost::lexical_cast<std::string>(num_acc);
  
  std::string out_url = ""; //"/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/img/fig_hit_rate_sim.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Run index", "Hit rate",
                   plot_title_ss.str(), out_url);
}

void palgo_test()
{
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<LZAlgo>();
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<ALZAlgo>();
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<PPMAlgo>(4);
  // boost::shared_ptr<PrefetchAlgo> palgo_ = boost::make_shared<POAlgo>();
  
  // std::map<PREFETCH_T, float> prefetch_t__weight_map;
  // prefetch_t__weight_map[W_LZ] = 0.5;
  // prefetch_t__weight_map[W_PPM] = 0.5;
  // boost::shared_ptr<MPrefetchAlgo> palgo_ = boost::make_shared<MPrefetchAlgo>(W_MP_WEIGHT, prefetch_t__weight_map);
  // boost::shared_ptr<MPrefetchAlgo> palgo_ = boost::make_shared<MPrefetchAlgo>(W_MP_MAX, prefetch_t__weight_map);
  
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  //                   1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
  // int acc_[] = {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  // int acc_[] = {0, 1, 1, 0, 1, 0, 1, 0, 0, 0 };
  //aaababbbbbaabccddcbaaaa = 00010111110012233210000
  // int acc_[] = {0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 2, 2, 3, 3, 2, 1, 0, 0, 0, 0 };
  // int acc_[] = {0, 0, 0, 1, 0, 1, 1 };
  // /* 
  int alphabet_size = 4;
  int num_acc = 1000; //sizeof(acc_)/sizeof(*acc_);
  std::map<ACC_T, float> acc__arr_rate_map;
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  
  std::vector<int> acc_v;
  // gen_random_acc_seq(alphabet_size, num_acc, acc_v);
  // gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  gen_intermittent_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  
  std::map<ACC_T, float> acc__emp_prob_map;
  get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
  std::cout << "acc__emp_prob_map= \n" << patch_pre::map_to_str<ACC_T, float>(acc__emp_prob_map);
  std::cout << "acc_v= \n" << patch_pre::vec_to_str(acc_v) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  int cache_size = 1;
  std::vector<char> accuracy_v;
  
  boost::shared_ptr<PrefetchAlgo> lz_algo_ = boost::make_shared<LZAlgo>();
  float lz_hit_rate;
  sim_prefetch_accuracy<PrefetchAlgo>(*lz_algo_, cache_size, acc_step_v, lz_hit_rate, accuracy_v);
  std::cout << "LZ_ALGO: \n";
  std::cout << "hit_rate= " << lz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << lz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_v= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PrefetchAlgo> alz_algo_ = boost::make_shared<ALZAlgo>();
  float alz_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PrefetchAlgo>(*alz_algo_, cache_size, acc_step_v, alz_hit_rate, accuracy_v);
  std::cout << "ALZ_ALGO: \n";
  std::cout << "hit_rate= " << alz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << alz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PrefetchAlgo> ppm_1_algo_ = boost::make_shared<PPMAlgo>(1);
  float ppm_1_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PrefetchAlgo>(*ppm_1_algo_, cache_size, acc_step_v, ppm_1_hit_rate, accuracy_v);
  std::cout << "PPM_1_ALGO: \n";
  std::cout << "hit_rate= " << ppm_1_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_1_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PrefetchAlgo> ppm_2_algo_ = boost::make_shared<PPMAlgo>(2);
  float ppm_2_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PrefetchAlgo>(*ppm_2_algo_, cache_size, acc_step_v, ppm_2_hit_rate, accuracy_v);
  std::cout << "PPM_2_ALGO: \n";
  std::cout << "hit_rate= " << ppm_2_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_2_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PrefetchAlgo> ppm_3_algo_ = boost::make_shared<PPMAlgo>(3);
  float ppm_3_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PrefetchAlgo>(*ppm_3_algo_, cache_size, acc_step_v, ppm_3_hit_rate, accuracy_v);
  std::cout << "PPM_3_ALGO: \n";
  std::cout << "hit_rate= " << ppm_3_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_3_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PrefetchAlgo> po_algo_ = boost::make_shared<POAlgo>();
  float po_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PrefetchAlgo>(*po_algo_, cache_size, acc_step_v, po_hit_rate, accuracy_v);
  std::cout << "PO_ALGO: \n";
  std::cout << "hit_rate= " << po_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  std::map<PREFETCH_T, float> prefetch_t__weight_map;
  prefetch_t__weight_map[W_LZ] = 0.33;
  prefetch_t__weight_map[W_PPM] = 0.33;
  prefetch_t__weight_map[W_PO] = 0.33;
  boost::shared_ptr<MPrefetchAlgo> mp_w_weight_algo_ = boost::make_shared<MPrefetchAlgo>(W_MP_WEIGHT, prefetch_t__weight_map);
  float mp_w_weight_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPrefetchAlgo>(*mp_w_weight_algo_, cache_size, acc_step_v, mp_w_weight_hit_rate, accuracy_v);
  std::cout << "W_MP_WEIGHT_ALGO: \n";
  std::cout << "hit_rate= " << mp_w_weight_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MPrefetchAlgo> mp_w_max_algo_ = boost::make_shared<MPrefetchAlgo>(W_MP_MAX, prefetch_t__weight_map);
  float mp_w_max_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPrefetchAlgo>(*mp_w_max_algo_, cache_size, acc_step_v, mp_w_max_hit_rate, accuracy_v);
  std::cout << "W_MP_MAX_ALGO: \n";
  std::cout << "hit_rate= " << mp_w_max_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  
  std::cout << "lz_hit_rate= " << lz_hit_rate << "\n"
            << "alz_hit_rate= " << alz_hit_rate << "\n"
            << "ppm_1_hit_rate= " << ppm_1_hit_rate << "\n"
            << "ppm_2_hit_rate= " << ppm_2_hit_rate << "\n"
            << "ppm_3_hit_rate= " << ppm_3_hit_rate << "\n"
            << "po_hit_rate= " << po_hit_rate << "\n"
            << "mp_w_weight_hit_rate= " << mp_w_weight_hit_rate << "\n"
            << "mp_w_max_hit_rate= " << mp_w_max_hit_rate << "\n";
  // */
  
  /*
  for (int i = 0; i < sizeof(acc_)/sizeof(*acc_); i++) {
    palgo_->add_access(acc_[i] );
    
    std::cout << "access_seq= " << patch_pre::vec_to_str<ACC_T>(palgo_->get_acc_v() ) << "\n";
    
    int num_acc = 1;
    std::vector<ACC_T> acc_v, eacc_v;
    palgo_->get_to_prefetch(num_acc, acc_v, std::vector<ACC_T>(), eacc_v);
    std::cout << "acc_v= " << patch_pre::vec_to_str<ACC_T>(acc_v) << "\n";
    
    // std::cout << "parse_tree_to_pstr= \n" << palgo_->parse_tree_to_pstr() << "\n";
    // std::map<ACC_T, float> acc_prob_map;
    // palgo_->get_acc_prob_map_for_prefetch(acc_prob_map);
    // std::cout << "acc_prob_map= \n" << patch_pre::map_to_str<ACC_T, float>(acc_prob_map) << "\n";
    
    // std::map<int, float> acc__emp_prob_map;
    // palgo_->get_acc__emp_prob_map_for_prefetch(acc__emp_prob_map);
    // std::cout << "acc__emp_prob_map= \n" << patch_pre::map_to_str<int, float>(acc__emp_prob_map) << "\n";
    
    std::cout << "\n";
  }
  */
}

void handle_prefetch(char ds_id, key_ver_pair kv)
{
  // LOG(INFO) << "handle_prefetch:: ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
}

void handle_del(key_ver_pair kv)
{
  // LOG(INFO) << "handle_del:: <key= " << kv.first << ", ver= " << kv.second << ">.";
}

void add_access(PBuffer* pbuffer, int p_id, key_ver_pair kv)
{
  pbuffer->add_access(p_id, kv);
}

void prefetch_test()
{
  int buffer_size = 10;
  PBuffer pbuffer('a', buffer_size, W_PPM, //W_MP_MAX, // W_LZ
                  true, boost::bind(handle_prefetch, _1, _2),
                  boost::bind(handle_del, _1) );
  
  int p_id_[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int num_p = sizeof(p_id_)/sizeof(*p_id_);
  // int num_put = 20;
  
  int num_acc = 100; // 30 * (rand() % 10) / 10;
  std::vector<int> p_id_v;
  std::vector<key_ver_pair> key_ver_v;
  std::map<int, int> p_id__last_step_map;
  for (int i = 0; i < num_acc; i++) {
    int p_id = rand() % num_p;
    p_id_v.push_back(p_id);
    
    if (p_id__last_step_map.count(p_id) == 0)
      p_id__last_step_map[p_id] = -1;
    
    key_ver_v.push_back( std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(p_id__last_step_map[p_id] + 1), 0) );
    p_id__last_step_map[p_id] += 1;
  }
  
  // std::vector<int>::iterator pid_it;
  // std::vector<key_ver_pair>::iterator kv_it;
  // for (pid_it = p_id_v.begin(), kv_it = key_ver_v.begin(); pid_it != p_id_v.end(), kv_it != key_ver_v.end(); pid_it++, kv_it++)
  //   pbuffer.reg_key_ver(*pid_it, *kv_it);
  // for (pid_it = p_id_v.begin(), kv_it = key_ver_v.begin(); pid_it != p_id_v.end(), kv_it != key_ver_v.end(); pid_it++, kv_it++) {
  //   // boost::thread t(add_access, &pbuffer, p_id, p_id__last_step_map[p_id] );
  //   add_access(&pbuffer, *pid_it, *kv_it);
  // }
  // std::cout << "prefetch_test:: p_id_v= " << patch_pre::vec_to_str<int>(p_id_v) << "\n";
  // std::cout << "prefetch_test:: key_ver_v= \n" << patch_pre::pvec_to_str<key_ver_pair>(key_ver_v) << "\n";
  
  float hit_rate;
  std::vector<char> accuracy_v;
  pbuffer.sim_prefetch_accuracy(p_id_v, key_ver_v, hit_rate, accuracy_v);
  std::cout << "accuracy_v= " << patch_pre::vec_to_str<char>(accuracy_v) << "\n";
  std::cout << "hit_rate= " << hit_rate << "\n";
  
  // 
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
  
  std::cout << "pbuffer= \n" << pbuffer.to_str();
}

void sim_test()
{
  int num_p, num_c;
  std::vector<char> p_id__ds_id_v, c_id__ds_id_v;
  std::vector<int> p_id__num_put_v, c_id__num_get_v;
  std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
  std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;
  
  int num_ds = 2;
  int max_num_p = 5;
  int max_num_c = 5;
  int num_putget_mean = 20;
  float put_rate_mean = 5;
  float get_rate_mean = 0.2;
  
  std::vector<char> ds_id_v;
  gen_scenario(num_ds, ds_id_v,
              max_num_p, max_num_c, num_putget_mean, put_rate_mean, get_rate_mean,
              num_p, num_c,
              p_id__ds_id_v, c_id__ds_id_v,
              p_id__num_put_v, c_id__num_get_v,
              p_id__put_rate_v, c_id__get_rate_v,
              p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v);
  
  PCSim pc_sim(ds_id_v, max_num_p + 1, W_PPM,
              num_p, num_c,
              p_id__ds_id_v, c_id__ds_id_v,
              p_id__num_put_v, c_id__num_get_v,
              p_id__put_rate_v, c_id__get_rate_v,
              p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v );
  
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
  srand(time(NULL) );
  
  // test_rand_shuffle_train();
  // plot_palgo_comparison();
  // palgo_test();
  // prefetch_test();
  sim_test();
  
  // validate_random_shuffle();
  
  // std::vector<int> v;
  // for (int i = 1; i < 10; i++)
  //   v.push_back(i);

  // std::random_shuffle(v.begin(), v.end() );
  // std::cout << "shuffled v= " << patch_pre::vec_to_str<int>(v) << "\n";
  
  // float lambda = 0.1;
  // int num_exp = 1000;
  // float sum = 0;
  // srand(time(NULL) );
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
