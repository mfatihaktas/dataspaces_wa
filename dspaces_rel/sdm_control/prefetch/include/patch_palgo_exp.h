#ifndef _PATCH_PALGO_EXP_H_
#define _PATCH_PALGO_EXP_H_

#include "patch_markov_exp.h"
// #include "palgo.h"

void test_talgo()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<TAlgo>() );
  title_v.push_back("gaussian algo");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 4; // 20; // 10;
  int num_acc = 200; // 10; // 200; // 50;
  
  std::vector<ACC_T> acc_v;
  std::vector<arr_time__acc_pair> arr_time__acc_pair_v;
  gen_real_acc_seq(alphabet_size, num_acc, 20, 100, 10, acc_v, arr_time__acc_pair_v);
  // 
  std::vector<std::vector<float> > cache_size_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_filtering_run = 1;
  for (int cache_size = 1; cache_size <= alphabet_size; cache_size++) {
  // for (int cache_size = 1; cache_size <= 7; cache_size++) {
    std::vector<float> total_hit_rate_v(num_algo);
    for (int f = 0; f < num_filtering_run; f++) {
      std::cout << "cache_size= " << cache_size << ", f= " << f << "\n";
      
      int algo_id = 0;
      for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
        (*it)->reset();
        if (f == 0) {
          cache_size_v_v[algo_id].push_back(cache_size);
        }
        
        accuracy_v.clear();
        sim_prefetch_accuracy<PAlgo>(**it, cache_size, arr_time__acc_pair_v, hit_rate, accuracy_v);
        // log_(INFO, "arr_time__acc_pair_v= \n" << patch::pvec_to_str<>(arr_time__acc_pair_v) )
        // std::cout << "accuracy_v= " << patch::vec_to_str<>(accuracy_v) << "\n";
        total_hit_rate_v[algo_id] += hit_rate;
      }
    }
    for (int algo_id = 0; algo_id < num_algo; algo_id++)
      hit_rate_v_v[algo_id].push_back(total_hit_rate_v[algo_id] / num_filtering_run);
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Avg Hit rate after training with noisy pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern size= " << num_acc;
  
  std::string out_url = "";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size (Number of data items)", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_vs_cache_size.png";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size (Number of data items)", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
}

void plot_galgo_hit_rate_vs_stdev()
{
  std::vector<std::string> title_v;
  boost::shared_ptr<PAlgo> palgo_ = boost::make_shared<TAlgo>();
  
  int alphabet_size = 20; // 4; // 20; // 10;
  int num_acc = 200; // 10; // 200; // 50;
  // 
  std::vector<std::vector<float> > cache_size_v_v; // (num_algo);
  std::vector<std::vector<float> > hit_rate_v_v; // (num_algo);
  
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_filtering_run = 1;
  int stdev_id = 0;
  for (int stdev = 1; stdev <= 20; stdev += 5, stdev_id++) {
    std::vector<ACC_T> acc_v;
    std::vector<arr_time__acc_pair> arr_time__acc_pair_v;
    gen_real_acc_seq(alphabet_size, num_acc, 20, 100, stdev, acc_v, arr_time__acc_pair_v);
    
    title_v.push_back("gaussian algo {/Symbol s}= " + stdev);
    cache_size_v_v.push_back(std::vector<float>() );
    hit_rate_v_v.push_back(std::vector<float>() );
    for (int cache_size = 1; cache_size <= alphabet_size; cache_size += 4) {
    // for (int cache_size = 1; cache_size <= 7; cache_size++) {
      float total_hit_rate = 0;
      for (int f = 0; f < num_filtering_run; f++) {
        std::cout << "stdev= " << stdev << ", cache_size= " << cache_size << ", f= " << f << "\n";
        if (f == 0)
          cache_size_v_v[stdev_id].push_back(cache_size);
        
        palgo_->reset();
        accuracy_v.clear();
        sim_prefetch_accuracy<PAlgo>(*palgo_, cache_size, arr_time__acc_pair_v, hit_rate, accuracy_v);
        // log_(INFO, "arr_time__acc_pair_v= \n" << patch::pvec_to_str<>(arr_time__acc_pair_v) )
        // std::cout << "accuracy_v= " << patch::vec_to_str<>(accuracy_v) << "\n";
        
        total_hit_rate += hit_rate;
      }
      hit_rate_v_v[stdev_id].push_back(total_hit_rate / num_filtering_run);
    }
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Avg Hit rate for independent inter arrivals ~ N([20, 100], {/Symbol s}^2); "
                << "alphabet size= " << alphabet_size
                << ", pattern size= " << num_acc;
  
  std::string out_url = "";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size (Number of data items)", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_galgo_hit_rate_vs_stdev.eps";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size (Number of data items)", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
}

#endif // _PATCH_PALGO_EXP_H_