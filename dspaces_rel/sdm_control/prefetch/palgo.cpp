#include "palgo.h"
#include "patch_pre.h"

/**********************************************  PAlgo  *******************************************/
PAlgo::PAlgo() {}
PAlgo::~PAlgo() {}

std::string PAlgo::to_str()
{
  std::stringstream ss;
  ss << "acc_s= " << patch::set_to_str<>(acc_s) << "\n"
     << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n";
  
  return ss.str();
}

std::vector<ACC_T> PAlgo::get_acc_v() { return acc_v; }

void PAlgo::reset()
{
  acc_s.clear();
  acc_v.clear();
}

int PAlgo::add_access(ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
}

/**********************************************  TAlgo  *******************************************/
TAlgo::TAlgo() {}
TAlgo::~TAlgo() {}

std::string TAlgo::to_str()
{
  std::stringstream ss;
  ss << "acc___arr__inter_time_pair_v_map= \n";
  for (std::map<ACC_T, std::vector<time_time_pair> >::iterator map_it = acc___arr__inter_time_pair_v_map.begin();
       map_it != acc___arr__inter_time_pair_v_map.end(); map_it++) {
    ss << "acc= " << map_it->first << "\n";
    for (std::vector<time_time_pair>::iterator it = (map_it->second).begin(); it != (map_it->second).end(); it++)
      ss << "\t" << PAIR_TO_STR(*it) << "\n";
      
      // ss << "\t" << "\n";
  }
  
  ss << "acc__mean_var_pair_map= \n";
  for (std::map<ACC_T, mean_var_pair>::iterator it = acc__mean_var_pair_map.begin(); it != acc__mean_var_pair_map.end(); it++)
    ss << "\t acc= " << it->first << " : " << PAIR_TO_STR(it->second) << "\n";
  
  return ss.str();
}

void TAlgo::reset()
{
  PAlgo::reset();
  acc___arr__inter_time_pair_v_map.clear();
  acc__mean_var_pair_map.clear();
}

int TAlgo::train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v)
{
  int err;
  for (std::vector<arr_time__acc_pair>::iterator it = arr_time__acc_pair_v.begin(); it != arr_time__acc_pair_v.end(); it++) {
    return_if_err(add_access(it->first, it->second), err)
  }
  
  return 0;
}

int TAlgo::add_access(float arr_time, ACC_T acc)
{
  int err;
  return_if_err(PAlgo::add_access(acc), err)
  
  std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[acc];
  float inter_time = NO_INTER_TIME;
  if (!ai_v.empty() ) {
    time_time_pair last_ai = ai_v.back();
    inter_time = arr_time - last_ai.first;
    return_if_err(inter_time < 0, err)
  }
  time_time_pair ai = std::make_pair(arr_time, inter_time);
  ai_v.push_back(ai);
  if (inter_time == NO_INTER_TIME)
    return 0;
  // Updating mean, var
  float sum = 0;
  for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++)
    sum += it->second;
  float mean = sum / ai_v.size();
  
  sum = 0;
  for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++) {
    if (it->second != NO_INTER_TIME)
      sum += pow(it->second - mean, 2);
  }
  float var = sum / (ai_v.size() - 1);
  
  acc__mean_var_pair_map[acc] = std::make_pair(mean, var);
  
  return 0;
}

int TAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                           const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  std::map<float, ACC_T> prob_acc_map;
  for (std::map<ACC_T, mean_var_pair>::iterator it = acc__mean_var_pair_map.begin(); it != acc__mean_var_pair_map.end(); it++) {
    std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[it->first];
    // mean_var_pair mv = it->second;
    
    boost::math::normal_distribution<float> inter_arr_dist((it->second).first, (it->second).second);
    float time_passed_since_last_arr = _time - ai_v.back().first;
    return_if_err(time_passed_since_last_arr < 0, err)
    
    prob_acc_map[boost::math::cdf(boost::math::complement(inter_arr_dist, time_passed_since_last_arr) ) ] = it->first;
  }
  
  int counter = 0;
  for (std::map<float, ACC_T>::reverse_iterator rit = prob_acc_map.rbegin();
       rit = prob_acc_map.rend(), counter < num_acc; rit++, counter++)
    acc_v.push_back(rit->second);
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

