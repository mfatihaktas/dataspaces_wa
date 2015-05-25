#ifndef _EXP_PATCH_H_
#define _EXP_PATCH_H_

#include <boost/math/distributions/normal.hpp>

// ----------------------------------  palgo_test  ------------------------------------//
void gen_random_acc_seq(size_t alphabet_size, size_t acc_size, KEY_T*& acc_)
{
  srand(time(NULL) );
  
  // boost::math::normal_distribution<float> num_putget_dist(0, VARIANCE);
  // int num_put = abs(quantile(num_putget_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
  acc_ = (int*)malloc(acc_size*sizeof(int) );
  for (int i = 0; i < acc_size; i++)
    acc_[i] = rand() % alphabet_size;
}

void get_emprical_dist(size_t alphabet_size, size_t acc_size, int* acc_, std::map<KEY_T, float>& acc__emp_prob_map)
{
  for (int i = 0; i < acc_size; i++) {
    KEY_T acc = acc_[i];
    if (acc__emp_prob_map.count(acc) == 0)
      acc__emp_prob_map[acc] = 0;
    acc__emp_prob_map[acc] += (float) 1 / acc_size;
  }
}

void gen_semirandom_acc_seq(size_t alphabet_size, size_t order, size_t acc_size, KEY_T*& acc_)
{
  srand(time(NULL) );
  
  std::deque<int> context;
  acc_ = (int*)malloc(acc_size*sizeof(int) );
  for (int i = 0; i < acc_size; i++) {
    int context_sum = 0;
    for (std::deque<int>::iterator it = context.begin(); it != context.end(); it++)
      context_sum += *it;
    
    int acc;
    if (context.size() == 0)
      acc = rand() % alphabet_size;
    else
      acc = abs( (context_sum/context.size() - rand() % alphabet_size) % alphabet_size );
      // acc = context_sum/context.size();
    
    acc_[i] = acc;
    
    if (context.size() == order)
      context.pop_front();
    context.push_back(acc);
  }
}

void gen_poisson_acc_seq(size_t alphabet_size, size_t acc_size, 
                         std::map<KEY_T, float> acc__arr_rate_map, KEY_T*& acc_)
{
  float sum_arr_rate = 0;
  for (std::map<KEY_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++)
    sum_arr_rate += it->second;
  
  std::map<KEY_T, float> acc__lower_lim_map;
  float base_lower_lim = 0;
  for (std::map<KEY_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++) {
    acc__lower_lim_map[it->first] = base_lower_lim;
    base_lower_lim += it->second / sum_arr_rate;
  }
  
  srand(time(NULL) );
  if (acc_ == NULL)
    acc_ = (int*)malloc(acc_size*sizeof(int) );
  for (int i = 0; i < acc_size; i++) {
    float rand_num = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX);
    for (std::map<KEY_T, float>::reverse_iterator rit = acc__lower_lim_map.rbegin(); rit != acc__lower_lim_map.rend(); rit++) {
      if (rand_num > rit->second) {
        acc_[i] = rit->first;
        break;
      }
    }
  }
}

void gen_intermittent_poisson_acc_seq(size_t alphabet_size, size_t acc_size, 
                                      std::map<KEY_T, float> acc__arr_rate_map, KEY_T*& acc_)
{
  // Generates scenario where the process for last key will appear only in the middle section of whole process
  acc_ = (int*)malloc(acc_size*sizeof(int) );
  
  std::vector<KEY_T> intermittent_acc_v;
  intermittent_acc_v.push_back(alphabet_size - 1);
  intermittent_acc_v.push_back(alphabet_size - 2);
  
  std::map<KEY_T, float> acc__arr_rate_map_;
  for (std::map<KEY_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++) {
    if (std::find(intermittent_acc_v.begin(), intermittent_acc_v.end(), it->first) != intermittent_acc_v.end() )
      continue;
    acc__arr_rate_map_[it->first] = it->second;
  }
  
  size_t first_size = acc_size/3;
  gen_poisson_acc_seq(alphabet_size, first_size, acc__arr_rate_map_, acc_);
  
  KEY_T* acc__ = acc_ + first_size;
  size_t second_size = acc_size/3;
  gen_poisson_acc_seq(alphabet_size, second_size, acc__arr_rate_map, acc__);
  
  acc__ = acc_ + first_size + second_size;
  size_t third_size = acc_size - first_size - second_size;
  gen_poisson_acc_seq(alphabet_size, third_size, acc__arr_rate_map_, acc__);
}

typedef std::pair<KEY_T, int> acc_step_pair;
void acc_v_to_acc_step_v(std::vector<KEY_T> acc_v, std::vector<acc_step_pair>& acc_step_v)
{
  std::map<KEY_T, int> acc_step_map;
  for (std::vector<KEY_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (acc_step_map.count(*it) == 0)
      acc_step_map[*it] = 0;
    
    acc_step_v.push_back(std::make_pair(*it, acc_step_map[*it]) );
    acc_step_map[*it] += 1;
  }
}

// ----------------------------------  sim_test  ------------------------------------//
#define VARIANCE 2
void gen_scenario(int num_ds, char*& ds_id_,
                  int max_num_p, int max_num_c, int num_putget_mean, float put_rate_mean, float get_rate_mean,
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
    boost::math::normal_distribution<float> num_putget_dist(num_putget_mean, VARIANCE);
    int num_put = abs(quantile(num_putget_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
    
    // TODO: may be randomized
    p_id__ds_id_vec.push_back(ds_id_[0] ); 
    
    p_id__num_put_vec.push_back(num_put);
    
    boost::math::normal_distribution<float> put_rate_dist(put_rate_mean, VARIANCE);
    p_id__put_rate_vec.push_back(
      abs(quantile(put_rate_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
    );
  }
  
  num_c = num_p; // rand() % max_num_p + 1;
  for (int i = 0; i < num_c; i++) {
    // TODO: may be randomized
    c_id__ds_id_vec.push_back(ds_id_[1] );
    
    // int num_get = rand() % num_putget_mean + 1;
    // c_id__num_get_vec.push_back(num_get);
    c_id__num_get_vec.push_back(p_id__num_put_vec[i]);
    
    boost::math::normal_distribution<float> get_rate_dist(get_rate_mean, VARIANCE);
    c_id__get_rate_vec.push_back(
      abs(quantile(get_rate_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
    );
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

  boost::math::normal_distribution<float> inter_arr_time_dist(1, VARIANCE);
  for (int c_id = 0; c_id < num_c; c_id++) {
    std::vector<float> inter_arr_time_vec;
    for (int i = 0; i < c_id__num_get_vec[c_id]; i++) {
      inter_arr_time_vec.push_back(
        // -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / c_id__get_rate_vec[p_id] )
        // 2 * (1.0 - (float)(rand() % 10) / 10) 
        abs(quantile(inter_arr_time_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
        );
    }
    c_id__inter_arr_time_vec_vec.push_back(inter_arr_time_vec);
  }
}

#endif // _EXP_PATCH_H_