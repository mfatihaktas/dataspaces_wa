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
  
  return 0;
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
    ss << "\t acc= " << map_it->first << "\n";
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
  
  if (acc___arr__inter_time_pair_v_map.count(acc) == 0)
    acc___arr__inter_time_pair_v_map[acc] = std::vector<time_time_pair>();
  
  std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[acc];
  float inter_time = NO_INTER_TIME;
  if (!ai_v.empty() ) {
    time_time_pair last_ai = ai_v.back();
    inter_time = arr_time - last_ai.first;
    return_if_err(inter_time < 0, err)
  }
  ai_v.push_back(std::make_pair(arr_time, inter_time) );
  if (inter_time == NO_INTER_TIME)
    return 0;
  // Updating mean, var
  int num_sample = ai_v.size() - 1; // First inter_arr_time is always 0
  
  float sum = 0;
  for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++)
    sum += it->second;
  float mean = sum / num_sample;
  
  float var = MIN_VARIANCE;
  if (num_sample > 2) {
    sum = 0;
    for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++) {
      if (it->second != NO_INTER_TIME)
        sum += pow(it->second - mean, 2);
    }
    var = std::max(sum / (num_sample - 1), MIN_VARIANCE);
  }
  
  acc__mean_var_pair_map[acc] = std::make_pair(mean, var);
  
  return 0;
}

int TAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                           const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  std::map<float, ACC_T> prob_acc_map;
  std::vector<ACC_T> skipped_v;
  for (std::map<ACC_T, mean_var_pair>::iterator it = acc__mean_var_pair_map.begin(); it != acc__mean_var_pair_map.end(); it++) {
    std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[it->first];
    mean_var_pair mv = it->second;
    
    float time_passed_since_last_arr = _time - ai_v.back().first;
    return_if_err(time_passed_since_last_arr < 0, err)
    
    if (time_passed_since_last_arr < MIN_INTER_TIME_FOR_PREDICTION) {
      // log_(WARNING, "skipping acc= " << it->first << "\n"
      //               << "\t time_passed_since_last_arr= " << time_passed_since_last_arr << " < MIN_INTER_TIME_FOR_PREDICTION= " << MIN_INTER_TIME_FOR_PREDICTION)
      skipped_v.push_back(it->first);
      continue;
    }
    else if (time_passed_since_last_arr > 2*mv.first) {
    // if (time_passed_since_last_arr > 4*pow(mv.second, (float)0.5) ) {
      // log_(WARNING, "skipping acc= " << it->first << "\n"
      //               << "\t time_passed_since_last_arr= " << time_passed_since_last_arr  << " >> 3*mean= " << 3*mv.first)
                    // << " >> 4*std= " << 4*pow(mv.second, (float)0.5) << "; acc= " << it->first)
      continue;
    }
    
    boost::math::normal_distribution<float> inter_arr_dist(mv.first, pow(mv.second, 0.5) );
    float prob = boost::math::cdf(boost::math::complement(inter_arr_dist, time_passed_since_last_arr) );
    while (prob_acc_map.count(prob) != 0)
      prob += 0.0001;
    // log_(INFO, ">>>>> " << "acc= " << it->first << "\n"
    //           << "\t time_passed_since_last_arr= " << time_passed_since_last_arr << "\n"
    //           << "\t prob= " << prob)
    prob_acc_map[prob] = it->first;
  }
  
  // for (std::map<float, ACC_T>::reverse_iterator rit = prob_acc_map.rbegin(); rit != prob_acc_map.rend(); rit++)
  for (std::map<float, ACC_T>::iterator it = prob_acc_map.begin(); it != prob_acc_map.end(); it++) {
    acc_v.push_back(it->second);
    if (acc_v.size() == num_acc)
      break;
  }
  while (acc_v.size() < num_acc && !skipped_v.empty() ) {
    acc_v.push_back(skipped_v.back() );
    skipped_v.pop_back();
  }
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

