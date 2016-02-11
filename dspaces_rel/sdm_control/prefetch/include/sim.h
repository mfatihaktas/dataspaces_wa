#ifndef _SIM_H_
#define _SIM_H_

// #include <boost/math/distributions/exponential.hpp>
// #include <math.h>
#include <cstdlib>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <glog/logging.h>

#include "prefetch.h"

template <typename PALGO>
void sim_prefetch_accuracy(PALGO& palgo,
                           int cache_size, std::vector<acc_step_pair> acc_step_v,
                           float& hit_rate, std::vector<char>& accuracy_v)
{
  Cache<ACC_T, acc_step_pair> cache(cache_size, boost::function<void(acc_step_pair)>() );
  int num_miss = 0;
  
  std::map<ACC_T, int> acc__last_acced_step_map;
  
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    // std::cout << "is <" << it->first << ", " << it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    acc__last_acced_step_map[it->first] = it->second;
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wa-dataspaces scenario data is used only once
    cache.del(it->first, *it);
    
    palgo.add_access(it->first); // Reg only the acc
    
    int num_acc = cache_size;
    std::vector<ACC_T> cached_acc_v = cache.get_cached_acc_v();
    std::vector<ACC_T> acc_v, eacc_v;
    palgo.get_to_prefetch(num_acc, acc_v, cached_acc_v, eacc_v);
    // log_(INFO, "acc_step= <" << it->first << ", " << it->second << "> \n"
    //           << "cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) )
    for (std::vector<ACC_T>::iterator iit = acc_v.begin(); iit != acc_v.end(); iit++) {
      // cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
      if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *iit) == cached_acc_v.end() )
        cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    for (std::vector<ACC_T>::iterator iit = eacc_v.begin(); iit != eacc_v.end(); iit++) {
      if (cache.size() >= cache.get_max_size() )
        break;
      cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    // std::cout << "after update; cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) << "\n";
  }
  
  hit_rate = 1.0 - (float)num_miss/acc_step_v.size();
}

template <typename PALGO>
void sim_prefetch_accuracy(PALGO& palgo,
                           Cache<ACC_T, acc_step_pair>& cache, std::map<ACC_T, int>& acc__last_acced_step_map,
                           std::vector<acc_step_pair> acc_step_v,
                           float& hit_rate, std::vector<char>& accuracy_v)
{
  int num_miss = 0;
  
  // std::map<ACC_T, int> acc__last_acced_step_map;
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    // std::cout << "is <" << it->first << ", " << it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    acc__last_acced_step_map[it->first] = it->second;
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wa-dataspaces scenario data is used only once
    cache.del(it->first, *it);
    
    palgo.add_access(it->first); // Reg only the acc
    
    int num_acc = cache.get_max_size();
    std::vector<ACC_T> cached_acc_v = cache.get_cached_acc_v();
    std::vector<ACC_T> acc_v, eacc_v;
    palgo.get_to_prefetch(num_acc, acc_v, cached_acc_v, eacc_v);
    // log_(INFO, "acc_step= <" << it->first << ", " << it->second << "> \n"
    //           << "cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) )
    for (std::vector<ACC_T>::iterator iit = acc_v.begin(); iit != acc_v.end(); iit++) {
      // cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
      if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *iit) == cached_acc_v.end() )
        cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    for (std::vector<ACC_T>::iterator iit = eacc_v.begin(); iit != eacc_v.end(); iit++) {
      if (cache.size() >= cache.get_max_size() )
        break;
      cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    // std::cout << "after update; cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) << "\n";
  }
  
  hit_rate = 1.0 - (float)num_miss/acc_step_v.size();
}

template <typename PALGO>
void sim_prefetch_accuracy(PALGO& palgo,
                           int cache_size, std::vector<arr_time__acc_pair> arr_time__acc_pair_v,
                           float& hit_rate, std::vector<char>& accuracy_v)
{
  Cache<ACC_T, acc_step_pair> cache(cache_size, boost::function<void(acc_step_pair)>() );
  int num_miss = 0;
  
  std::map<ACC_T, int> acc__last_acced_step_map;
  
  for (std::vector<arr_time__acc_pair>::iterator it = arr_time__acc_pair_v.begin(); it != arr_time__acc_pair_v.end(); it++) {
    // std::cout << "is <" << it->first << ", " << it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    if (acc__last_acced_step_map.count(it->second) == 0)
      acc__last_acced_step_map[it->second] = -1;
    
    acc_step_pair acc_step = std::make_pair(it->second, ++acc__last_acced_step_map[it->second] );
    if (!cache.contains(acc_step) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // std::cout << "acc_step= " << PAIR_TO_STR(acc_step) << "\n";
    // In wa-dataspaces scenario data is used only once
    cache.del(it->second, acc_step);
    
    palgo.add_access(it->first, it->second);
    
    // Note: Relatively large num_acc does more harm then benefit for large cache_size
    int num_acc = std::min(3, cache_size);
    std::vector<ACC_T> cached_acc_v = cache.get_cached_acc_v();
    std::vector<ACC_T> acc_v, eacc_v;
    palgo.get_to_prefetch(it->first + 0.01, num_acc, acc_v, cached_acc_v, eacc_v);
    // log_(INFO, "\n"
    //           // << "palgo= \n" << palgo.to_str() << "\n"
    //           << "arr_time= " << it->first << ", acc_step= " << PAIR_TO_STR(acc_step) << "\n"
    //           << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n"
    //           << "eacc_v= " << patch::vec_to_str<>(eacc_v) << "\n"
    //           << "cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) )
    for (std::vector<ACC_T>::iterator iit = acc_v.begin(); iit != acc_v.end(); iit++) {
      // cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
      if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *iit) == cached_acc_v.end() )
        cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    for (std::vector<ACC_T>::iterator iit = eacc_v.begin(); iit != eacc_v.end(); iit++) {
      if (cache.size() >= cache.get_max_size() )
        break;
      cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
    }
    // std::cout << "after update; cache= \n" << patch::pvec_to_str<>(cache.get_content_v() ) << "\n";
  }
  
  hit_rate = 1.0 - (float)num_miss/arr_time__acc_pair_v.size();
}

template <typename SALGO>
void sim_prefetch_accuracy(SALGO& salgo,
                           std::vector<lcoor_ucoor_pair>& lucoor_to_acc_v,
                           float& hit_rate, std::vector<char>& accuracy_v)
{
  boost::shared_ptr<QTable<int> > qtable_ = boost::make_shared<RTable<int> >();
  int num_miss = 0;
  
  for (std::vector<lcoor_ucoor_pair>::iterator it = lucoor_to_acc_v.begin(); it != lucoor_to_acc_v.end(); it++) {
    std::vector<int> ds_id_v;
    if (qtable_->query("dummy", 0, it->first, it->second, ds_id_v) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    std::vector<lcoor_ucoor_pair> lucoor_to_fetch_v;
    salgo.get_to_fetch(it->first, it->second, lucoor_to_fetch_v);
    
    for (std::vector<lcoor_ucoor_pair>::iterator sub_it = lucoor_to_fetch_v.begin(); sub_it != lucoor_to_fetch_v.end(); sub_it++)
      qtable_->add("dummy", 0, sub_it->first, sub_it->second, 0);
  }
  // log_(INFO, "qtable= \n" << qtable_->to_str() )
  
  hit_rate = 1.0 - (float)num_miss/lucoor_to_acc_v.size();
}

/************************************************  PCSim  *****************************************/
class PCSim { // P-C Simulator
  protected:
    bool w_prefetch;
    int num_p, num_c;
    std::vector<int> p_id__ds_id_v, c_id__ds_id_v;
    std::vector<int> p_id__num_put_v, c_id__num_get_v;
    std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
    std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;

    COOR_T *lcoor_, *ucoor_;
    boost::shared_ptr<WASpace> wa_space_;
    
    std::map<int, std::vector<char> > c_id__get_type_v_map;
    std::vector<boost::shared_ptr<boost::thread> > thread_v;
  public:
    PCSim(std::vector<int> ds_id_v, bool w_prefetch,
          int num_p, int num_c,
          std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
          std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
          std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
          std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v);
    ~PCSim();
    virtual std::string to_str();
    std::string to_str_end();
    
    void sim_all();
    virtual void sim_p(int p_id) = 0;
    virtual void sim_c(int c_id, bool blocking_get) = 0;
    
    std::map<int, float> get_c_id__get_lperc_map();
    
    void handle_data_act(PREFETCH_DATA_ACT_T data_act_t, int ds_id, key_ver_pair kv, lcoor_ucoor_pair lucoor);
};

/************************************************  MPCSim  ****************************************/
class MPCSim : public PCSim { // Markov
  private:
    patch::syncer<key_ver_pair> bget_syncer;
    
  public:
    MPCSim(std::vector<int> ds_id_v, bool w_prefetch,
           int num_p, int num_c,
           std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
           std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
           std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
           std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
           PALGO_T palgo_t, int max_num_key_ver_in_mpbuffer);
    ~MPCSim();
    std::string to_str();
    
    void sim_p(int p_id);
    void sim_c(int c_id, bool blocking_get);
};

/*******************************************  SPCSim  **********************************************/
typedef std::pair<int, lcoor_ucoor_pair> app_id__lcoor_ucoor_pair;

class SPCSim : public PCSim { // Spatial
  private:
    spatial_syncer s_syncer;
  public:
    SPCSim(std::vector<int> ds_id_v, int num_p, int num_c,
           std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
           std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
           std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
           std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
           SALGO_T salgo_t, int sexpand_length, COOR_T* lcoor_, COOR_T* ucoor_, bool w_prefetch);
    ~SPCSim() {}
    std::string to_str();
    
    void sim_p(int p_id);
    void sim_c(int c_id, bool blocking_get);
    void get_lucoor_to_acc_v(std::vector<lcoor_ucoor_pair>& lucoor_v);
};

#endif // _SIM_H_
