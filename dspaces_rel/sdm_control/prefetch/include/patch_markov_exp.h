#ifndef _PATCH_MARKOV_EXP_H_
#define _PATCH_MARKOV_EXP_H_

#include <algorithm>

#include <string>
#include <cstdlib>
#include <ctime>

#include <boost/assign/list_of.hpp>
#include <boost/math/distributions/normal.hpp>

#include "gnuplot_iostream.h"

template<typename T>
void make_plot(std::vector<T> x1_v, std::vector<T> y1_v, std::string title1,
               std::vector<T> x2_v, std::vector<T> y2_v, std::string title2,
               std::string x_label, std::string y_label, 
               std::string plot_title,
               std::string out_url)
{
  Gnuplot gp;
  T min_x = std::min<T>(*std::min_element(x1_v.begin(), x1_v.end() ), *std::min_element(x2_v.begin(), x2_v.end() ) );
  T max_x = std::max<T>(*std::max_element(x1_v.begin(), x1_v.end() ), *std::max_element(x2_v.begin(), x2_v.end() ) );
  
  T min_y = std::min<T>(*std::min_element(y1_v.begin(), y1_v.end() ), *std::min_element(y2_v.begin(), y2_v.end() ) );
  T max_y = std::max<T>(*std::max_element(y1_v.begin(), y1_v.end() ), *std::max_element(y2_v.begin(), y2_v.end() ) );
  
  if (out_url.compare("") != 0) {
    gp << "set term png large enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key left top\n";
  gp << "set title '" << plot_title << "'\n";
  gp << "set style line 1 lc rgb '#7F7F7F' lt 1 lw 2 pt 4 ps 1.5\n";
  gp << "set style line 2 lc rgb '#0060ad' lt 1 lw 2 pt 5 ps 1.5\n";
  gp << "set xrange [" << min_x << ":" << max_x*1.2 << "]\nset yrange [" << min_y << ":" << max_y*1.2 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set grid\n";
  
  gp << "set logscale xy\n";
  // typename std::vector<T>::iterator it_y1, it_y2; // supposedly vectors of same size
  // for (it_y1 = y1_v.begin(), it_y2 = y2_v.begin(); 
  //     it_y1 != y1_v.end(), it_y2 != y2_v.end(); it_y1++, it_y2++) {
  //   gp << "set ytics add (" << boost::lexical_cast<std::string>(*it_y1) << ")\n";
  //   gp << "set ytics add (" << boost::lexical_cast<std::string>(*it_y2) << ")\n";
  // }
  // gp << "set ytics add (" << boost::lexical_cast<std::string>(min_y) << ")\n";
  // for (T f = 0; f < max_y; f += 20) {
  //   if (f > min_y)
  //     gp << "set ytics add (" << boost::lexical_cast<std::string>(f) << ")\n";
  // }
  
  gp << "plot '-' u 1:2 title '" << title1 << "' w linespoints ls 1, "
          << "'-' u 1:2 title '" << title2 << "' w linespoints ls 2\n";
  gp.send1d(boost::make_tuple(x1_v, y1_v) );
  gp.send1d(boost::make_tuple(x2_v, y2_v) );
}

template<typename T>
void make_plot(std::vector<std::vector<T> > x_v_v, std::vector<std::vector<T> > y_v_v, std::vector<std::string> title_v,
               std::string x_label, std::string y_label,
               std::string plot_title,
               std::string out_url)
{
  std::cout << "x_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
    std::cout << patch_all::vec_to_str<>(*it) << "\n";
  
  std::cout << "y_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
    std::cout << patch_all::vec_to_str<>(*it) << "\n";
  // 
  std::string color_code_[] = {"#DAA520", "#7F7F7F", "#0060ad", "#D2691E", "#556B2F", "#DC143C", "#DA70D6", "#8B008B", "#1E90FF"};
  Gnuplot gp;
  
  std::vector<T> x_v;
  for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
    x_v.insert(x_v.end(), it->begin(), it->end() );
  
  T min_x = *std::min_element(x_v.begin(), x_v.end() );
  T max_x = *std::max_element(x_v.begin(), x_v.end() );
  
  std::vector<T> y_v;
  for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
    y_v.insert(y_v.end(), it->begin(), it->end() );
  
  T min_y = *std::min_element(y_v.begin(), y_v.end() );
  T max_y = *std::max_element(y_v.begin(), y_v.end() );
  
  if (out_url.compare("") != 0) {
    gp << "set term png size 1000,600 enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key right top\n";
  gp << "set title '" << plot_title << "'\n";
  gp << "set xrange [" << min_x << ":" << max_x*1.4 << "]\nset yrange [" << min_y << ":" << max_y*1.2 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set grid\n";
  
  
  for (int i = 0; i < x_v_v.size(); i++)
    gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' " << "pt " << boost::lexical_cast<std::string>(i + 1) << " lt 1 lw 1 ps 1\n";
    // gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' lt 1 lw 2 pt 5 ps 1.5\n";
  
  // gp << "set logscale xy\n";
  
  gp << "plot";
  for (int i = 0; i < x_v_v.size(); i++) {
    gp << "'-' u 1:2 title '" << title_v[i] << "' w linespoints ls " << boost::lexical_cast<std::string>(i + 1);
    if (i == x_v_v.size() - 1)
      gp << "\n";
    else
      gp << ", ";
  }
  
  typename std::vector<std::vector<T> >::iterator it_x_v, it_y_v;
  for (it_x_v = x_v_v.begin(), it_y_v = y_v_v.begin(); it_x_v != x_v_v.end(); it_x_v++, it_y_v++)
    gp.send1d(boost::make_tuple(*it_x_v, *it_y_v) );
}

template<typename T>
void random_partial_shuffle(float shuffle_prob, int shuffle_width, std::vector<int> shuffle_indices, 
                            std::vector<T>& v)
{
  for (std::vector<int>::iterator i_it = shuffle_indices.begin(); i_it != shuffle_indices.end(); i_it++) {
    float rand_num = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX);
    if (rand_num < shuffle_prob) {
      typename std::vector<T>::iterator it_l = v.begin() + *i_it;
      typename std::vector<T>::iterator it_u = v.begin() + *i_it;
      for (int i = 0; i < shuffle_width; i++) {
        if (it_l != v.begin() )
          it_l--;
        if(it_u != v.end() )
          it_u++;
      }
      
      std::random_shuffle(it_l, it_u);
    }
  }
}

// void validate_random_shuffle()
// {
//   std::vector<int> v;
//   for (int i = 0; i < 10; i++)
//     v.push_back(rand() % 10);
  
//   std::cout << "v= " << patch_all::vec_to_str<>(v) << "\n";
  
//   std::vector<std::map<int, float> > i__int_freq_map_v(v.size() );
//   int num_run = 10000;
//   for (int r = 0; r < num_run; r++) {
//     std::random_shuffle(v.begin(), v.end() );
//     for (int i = 0; i < v.size(); i++) {
//       if (i__int_freq_map_v[i].count(v[i] ) == 0)
//         i__int_freq_map_v[i][v[i] ] = 0;
//       i__int_freq_map_v[i][v[i] ] += 1;
//     }
//   }
  
//   for (std::vector<std::map<int, float> >::iterator it = i__int_freq_map_v.begin(); it != i__int_freq_map_v.end(); it++) {
//     for (std::map<int, float>::iterator map_it = it->begin(); map_it != it->end(); map_it++)
//       map_it->second = map_it->second / num_run;
//   }
  
//   for (int i = 0; i < v.size(); i++)
//     std::cout << "i= " << i << ", int_num_map= \n" << patch_all::map_to_str<>(i__int_freq_map_v[i] ) << "\n";
// }

// ----------------------------------------  test_malgo  -----------------------------------------//
void gen_random_acc_seq(size_t alphabet_size, size_t num_acc, std::vector<ACC_T>& acc_v)
{
  for (int i = 0; i < num_acc; i++)
    acc_v.push_back(rand() % alphabet_size);
}

void get_emprical_dist(size_t alphabet_size, std::vector<ACC_T>& acc_v, std::map<ACC_T, float>& acc__emp_prob_map)
{
  int num_acc = acc_v.size();
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (acc__emp_prob_map.count(*it) == 0)
      acc__emp_prob_map[*it] = 0;
    acc__emp_prob_map[*it] += (float) 1 / num_acc;
  }
}

void gen_poisson_acc_seq(size_t alphabet_size, size_t num_acc,
                         std::map<ACC_T, float> acc__arr_rate_map, std::vector<ACC_T>& acc_v)
{
  float sum_arr_rate = 0;
  for (std::map<ACC_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++)
    sum_arr_rate += it->second;
  
  std::map<ACC_T, float> acc__lower_lim_map;
  float base_lower_lim = 0;
  for (std::map<ACC_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++) {
    acc__lower_lim_map[it->first] = base_lower_lim;
    base_lower_lim += it->second / sum_arr_rate;
  }
  
  for (int i = 0; i < num_acc; i++) {
    float rand_num = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX);
    for (std::map<ACC_T, float>::reverse_iterator rit = acc__lower_lim_map.rbegin(); rit != acc__lower_lim_map.rend(); rit++) {
      if (rand_num > rit->second) {
        acc_v.push_back(rit->first);
        break;
      }
    }
  }
}

void gen_intermittent_poisson_acc_seq(size_t alphabet_size, size_t num_acc, 
                                      std::map<ACC_T, float> acc__arr_rate_map, std::vector<ACC_T>& acc_v)
{
  // Generates scenario where the process for last key will appear only in the middle section of whole process
  std::vector<ACC_T> intermittent_acc_v;
  intermittent_acc_v.push_back(alphabet_size - 1);
  // intermittent_acc_v.push_back(alphabet_size - 2);
  
  std::map<ACC_T, float> acc__arr_rate_map_;
  for (std::map<ACC_T, float>::iterator it = acc__arr_rate_map.begin(); it != acc__arr_rate_map.end(); it++) {
    if (std::find(intermittent_acc_v.begin(), intermittent_acc_v.end(), it->first) != intermittent_acc_v.end() )
      continue;
    acc__arr_rate_map_[it->first] = it->second;
  }
  
  size_t first_size = num_acc/3;
  gen_poisson_acc_seq(alphabet_size, first_size, acc__arr_rate_map_, acc_v);
  
  std::vector<ACC_T> t_acc_v;
  size_t second_size = num_acc/3;
  gen_poisson_acc_seq(alphabet_size, second_size, acc__arr_rate_map, t_acc_v);
  acc_v.insert(acc_v.end(), t_acc_v.begin(), t_acc_v.end() );
  
  t_acc_v.clear();
  size_t third_size = num_acc - first_size - second_size;
  gen_poisson_acc_seq(alphabet_size, third_size, acc__arr_rate_map_, t_acc_v);
  acc_v.insert(acc_v.end(), t_acc_v.begin(), t_acc_v.end() );
}

typedef std::pair<ACC_T, int> acc_step_pair;
void acc_v_to_acc_step_v(std::vector<ACC_T>& acc_v, std::vector<acc_step_pair>& acc_step_v)
{
  std::map<ACC_T, int> acc_step_map;
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (acc_step_map.count(*it) == 0)
      acc_step_map[*it] = 0;
    
    acc_step_v.push_back(std::make_pair(*it, acc_step_map[*it] ) );
    acc_step_map[*it] += 1;
  }
}

// ----------------------------------------  sim_test  -------------------------------------------//
#define VARIANCE 2
void gen_scenario(int num_ds, std::vector<int>& ds_id_v,
                  int max_num_p, int max_num_c, int num_putget_mean, float put_rate_mean, float get_rate_mean,
                  int& num_p, int& num_c,
                  std::vector<int>& p_id__ds_id_v, std::vector<int>& c_id__ds_id_v,
                  std::vector<int>& p_id__num_put_v, std::vector<int>& c_id__num_get_v,
                  std::vector<float>& p_id__put_rate_vec, std::vector<float>& c_id__get_rate_v,
                  std::vector<std::vector<float> >& p_id__inter_arr_time_v_v, std::vector<std::vector<float> >& c_id__inter_arr_time_v_v )
{
  for (int i = 0; i < num_ds; i++)
    ds_id_v.push_back(i);
  
  num_p = max_num_p; // rand() % max_num_p + 1;
  for (int i = 0; i < num_p; i++) {
    boost::math::normal_distribution<float> num_putget_dist(num_putget_mean, VARIANCE);
    int num_put = (int) std::abs(quantile(num_putget_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
    
    // TODO: may be randomized
    p_id__ds_id_v.push_back(ds_id_v[0] ); 
    
    p_id__num_put_v.push_back(num_put);
    
    boost::math::normal_distribution<float> put_rate_dist(put_rate_mean, VARIANCE);
    p_id__put_rate_vec.push_back(
      std::abs(quantile(put_rate_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
    );
  }
  
  num_c = num_p; // rand() % max_num_p + 1;
  for (int i = 0; i < num_c; i++) {
    // TODO: may be randomized
    c_id__ds_id_v.push_back(ds_id_v[1] );
    
    // int num_get = rand() % num_putget_mean + 1;
    // c_id__num_get_v.push_back(num_get);
    c_id__num_get_v.push_back(p_id__num_put_v[i] );
    
    boost::math::normal_distribution<float> get_rate_dist(get_rate_mean, VARIANCE);
    c_id__get_rate_v.push_back(
      std::abs(quantile(get_rate_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
    );
  }
  
  // 
  for (int p_id = 0; p_id < num_p; p_id++) {
    std::vector<float> inter_arr_time_vec;
    for (int i = 0; i < p_id__num_put_v[p_id]; i++) {
      // boost::math::exponential_distribution exp_dist(p_id__put_rate_vec[p_id] );
      inter_arr_time_vec.push_back(
        -1 * log(1.0 - std::abs(static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) - 0.001) ) / p_id__put_rate_vec[p_id] );
    }
    p_id__inter_arr_time_v_v.push_back(inter_arr_time_vec);
  }

  boost::math::normal_distribution<float> inter_arr_time_dist(1, VARIANCE);
  for (int c_id = 0; c_id < num_c; c_id++) {
    std::vector<float> inter_arr_time_vec;
    for (int i = 0; i < c_id__num_get_v[c_id]; i++) {
      inter_arr_time_vec.push_back(
        // -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / c_id__get_rate_v[p_id] )
        // 2 * (1.0 - (float)(rand() % 10) / 10) 
        std::abs(quantile(inter_arr_time_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) )
      );
    }
    c_id__inter_arr_time_v_v.push_back(inter_arr_time_vec);
  }
}

// ------------------------------------------------------------------------------------------------/
void test_rand_shuffle_train()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<MAlgo> > malgo_v;
  malgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  malgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  // malgo_v.push_back(boost::make_shared<PPMAlgo>(5) );
  // title_v.push_back("ppm_5");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  // malgo_v.push_back(boost::make_shared<PPMAlgo>(8) );
  // title_v.push_back("ppm order 8");
  
  std::vector<boost::shared_ptr<MMAlgo> > mmalgo_v;
  std::vector<malgo_t__context_size_pair> malgo_t__context_size_v;
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  mmalgo_v.push_back(boost::make_shared<WMMAlgo>(malgo_t__context_size_v, malgo_id__weight_v) );
  title_v.push_back("mixed-blended");
  
  mmalgo_v.push_back(boost::make_shared<MMMAlgo>(malgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  mmalgo_v.push_back(boost::make_shared<BMMAlgo>(malgo_t__context_size_v, 4) );
  title_v.push_back("mixed-best wnd 4");
  
  int num_algo = malgo_v.size() + mmalgo_v.size();
  
  int alphabet_size = 10;
  int num_acc = 50;
  std::vector<int> acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  std::cout << "acc_v= " << patch_all::vec_to_str<>(acc_v) << "\n";
  // acc_v_to_acc_step_v(acc_v, acc_step_v);
  
  int shuffle_width = 2;
  float shuffle_prob = 0.5;
  std::vector<int> shuffle_indices;
  for (int i = 0; i < num_acc/10; i++)
    shuffle_indices.push_back(rand() % num_acc);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_run = 40;
  for (int i = 1; i <= num_run; i++) {
    shuffle_indices.clear();
    for (int j = 0; j < num_acc/10; j++)
      shuffle_indices.push_back(rand() % num_acc);
    random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
    
    std::map<ACC_T, float> acc__emp_prob_map;
    get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
    std::cout << "run_i= " << i << ", acc__emp_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
    // std::cout << "for training; acc_v= " << patch_all::vec_to_str<>(acc_v) << "\n";
    
    for (std::vector<boost::shared_ptr<MAlgo> >::iterator it = malgo_v.begin(); it != malgo_v.end(); it++)
      (*it)->train(acc_v);
    for (std::vector<boost::shared_ptr<MMAlgo> >::iterator it = mmalgo_v.begin(); it != mmalgo_v.end(); it++)
      (*it)->train(acc_v);
        
    // std::random_shuffle(acc_v_.begin(), acc_v_.end() );
    // for (std::vector<boost::shared_ptr<MAlgo> >::iterator it = malgo_v.begin(); it != malgo_v.end(); it++)
    //   (*it)->train(acc_v_);
    // for (std::vector<boost::shared_ptr<MMAlgo> >::iterator it = mmalgo_v.begin(); it != mmalgo_v.end(); it++)
    //   (*it)->train(acc_v_);
    
    // std::random_shuffle(acc_v.begin(), acc_v.end() );
    shuffle_indices.clear();
    for (int j = 0; j < num_acc/10; j++)
      shuffle_indices.push_back(rand() % num_acc);
    random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
    acc_step_v.clear();
    acc_v_to_acc_step_v(acc_v, acc_step_v);
    std::cout << "for prediction; acc_v= " << patch_all::vec_to_str<>(acc_v) << "\n";
    
    int algo_id = 0;
    for (std::vector<boost::shared_ptr<MAlgo> >::iterator it = malgo_v.begin(); it != malgo_v.end(); it++, algo_id++) {
      accuracy_v.clear();
      sim_prefetch_accuracy<MAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(2*i);
      // (*it)->reset();
    }
  
    for (std::vector<boost::shared_ptr<MMAlgo> >::iterator it = mmalgo_v.begin(); it != mmalgo_v.end(); it++, algo_id++) {
      accuracy_v.clear();
      sim_prefetch_accuracy<MMAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(2*i);
      // (*it)->reset();
    }
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Hit rate after training with noisy pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern length= " << num_acc;
  
  std::string out_url = ""; // "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/img/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of repetitions of the noisy pattern observed", "Hit rate",
                   plot_title_ss.str(), out_url);

  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of repetitions of the noisy pattern observed", "Hit rate",
                   plot_title_ss.str(), out_url);
  
  std::cout << "acc_step_v= " << patch_all::pvec_to_str<>(acc_step_v) << "\n";
}

void test_fixed_train()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<MAlgo> > malgo_v;
  malgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  malgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  // malgo_v.push_back(boost::make_shared<PPMAlgo>(5) );
  // title_v.push_back("ppm_5");
  malgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  // malgo_v.push_back(boost::make_shared<PPMAlgo>(8) );
  // title_v.push_back("ppm order 8");
  
  std::vector<boost::shared_ptr<MMAlgo> > mmalgo_v;
  std::vector<malgo_t__context_size_pair> malgo_t__context_size_v;
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  // std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.4)(0.15)(0.15)(0.15)(0.15);
  // std::vector<float> malgo_id__weight_v = boost::assign::list_of(1)(0)(0)(0)(0);
  mmalgo_v.push_back(boost::make_shared<WMMAlgo>(malgo_t__context_size_v, malgo_id__weight_v) );
  title_v.push_back("mixed-blended");
  
  mmalgo_v.push_back(boost::make_shared<MMMAlgo>(malgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  mmalgo_v.push_back(boost::make_shared<BMMAlgo>(malgo_t__context_size_v, 10) );
  title_v.push_back("mixed-best wnd 10");
  
  int num_algo = malgo_v.size() + mmalgo_v.size();
  
  int alphabet_size = 10;
  int num_acc = 50;
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
  
  int num_run = 50;
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
    std::cout << "run_i= " << i << ", acc__emp_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
    
    // std::cout << "acc_v= \n" << patch_all::vec_to_str(acc_v) << "\n";
    // acc_v_to_acc_step_v(acc_v, acc_step_v);
    
    int algo_id = 0;
    for (std::vector<boost::shared_ptr<MAlgo> >::iterator it = malgo_v.begin(); it != malgo_v.end(); it++, algo_id++) {
      // (*it)->reset();
      accuracy_v.clear();
      sim_prefetch_accuracy<MAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      // std::cout << "title= " << title_v[algo_id] << ", hit_rate= " << hit_rate << "\n";
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(2*i);
    }

    for (std::vector<boost::shared_ptr<MMAlgo> >::iterator it = mmalgo_v.begin(); it != mmalgo_v.end(); it++, algo_id++) {
      // (*it)->reset();
      accuracy_v.clear();
      sim_prefetch_accuracy<MMAlgo>(**it, 1, acc_step_v, hit_rate, accuracy_v);
      // std::cout << "title= " << title_v[algo_id] << ", hit_rate= " << hit_rate << "\n";
      hit_rate_v_v[algo_id].push_back(hit_rate);
      run_i_v_v[algo_id].push_back(2*i);
    }
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Hit rate after training with fixed pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern length= " << num_acc;
  
  std::string out_url = ""; //"/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/fig_hit_rate_w_fixed.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of repetitions of the fixed pattern observed", "Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_fixed.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of repetitions of the fixed pattern observed", "Hit rate",
                   plot_title_ss.str(), out_url);

  std::cout << "acc_step_v= " << patch_all::pvec_to_str<>(acc_step_v) << "\n";
  std::cout << "mixed-best wnd 4= \n" << mmalgo_v[mmalgo_v.size() - 1]->to_str() << "\n";
}

void test_malgo()
{
  // boost::shared_ptr<MAlgo> palgo_ = boost::make_shared<LZAlgo>();
  // boost::shared_ptr<MAlgo> palgo_ = boost::make_shared<ALZAlgo>();
  // boost::shared_ptr<MAlgo> palgo_ = boost::make_shared<PPMAlgo>(4);
  // boost::shared_ptr<MAlgo> palgo_ = boost::make_shared<POAlgo>();
  
  // std::map<MALGO_T, float> prefetch_t__weight_map;
  // prefetch_t__weight_map[MALGO_W_LZ] = 0.5;
  // prefetch_t__weight_map[MALGO_W_PPM] = 0.5;
  // boost::shared_ptr<MMAlgo> palgo_ = boost::make_shared<MMAlgo>(Mixed-blended, prefetch_t__weight_map);
  // boost::shared_ptr<MMAlgo> palgo_ = boost::make_shared<MMAlgo>(Mixed-most confident, prefetch_t__weight_map);
  
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
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  // gen_intermittent_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  
  std::map<ACC_T, float> acc__emp_prob_map;
  get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
  std::cout << "acc__emp_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
  std::cout << "acc_v= \n" << patch_all::vec_to_str(acc_v) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  int cache_size = 1;
  std::vector<char> accuracy_v;
  
  boost::shared_ptr<MAlgo> lz_algo_ = boost::make_shared<LZAlgo>();
  float lz_hit_rate;
  sim_prefetch_accuracy<MAlgo>(*lz_algo_, cache_size, acc_step_v, lz_hit_rate, accuracy_v);
  std::cout << "LZ_ALGO: \n"
            << "hit_rate= " << lz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << lz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_v= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MAlgo> alz_algo_ = boost::make_shared<ALZAlgo>();
  float alz_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MAlgo>(*alz_algo_, cache_size, acc_step_v, alz_hit_rate, accuracy_v);
  std::cout << "ALZ_ALGO: \n"
            << "hit_rate= " << alz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << alz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MAlgo> ppm_1_algo_ = boost::make_shared<PPMAlgo>(1);
  float ppm_1_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MAlgo>(*ppm_1_algo_, cache_size, acc_step_v, ppm_1_hit_rate, accuracy_v);
  std::cout << "PPM_1_ALGO: \n"
            << "hit_rate= " << ppm_1_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_1_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MAlgo> ppm_2_algo_ = boost::make_shared<PPMAlgo>(2);
  float ppm_2_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MAlgo>(*ppm_2_algo_, cache_size, acc_step_v, ppm_2_hit_rate, accuracy_v);
  std::cout << "PPM order 2_ALGO: \n"
            << "hit_rate= " << ppm_2_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_2_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MAlgo> ppm_3_algo_ = boost::make_shared<PPMAlgo>(3);
  float ppm_3_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MAlgo>(*ppm_3_algo_, cache_size, acc_step_v, ppm_3_hit_rate, accuracy_v);
  std::cout << "PPM_3_ALGO: \n"
            << "hit_rate= " << ppm_3_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_3_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MAlgo> po_algo_ = boost::make_shared<POAlgo>();
  float po_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MAlgo>(*po_algo_, cache_size, acc_step_v, po_hit_rate, accuracy_v);
  std::cout << "PO_ALGO: \n"
            << "hit_rate= " << po_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  std::vector<malgo_t__context_size_pair> malgo_t__context_size_v;
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PO, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  
  std::vector<float> malgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  boost::shared_ptr<MMAlgo> wmmalgo_ = boost::make_shared<WMMAlgo>(malgo_t__context_size_v, malgo_id__weight_v);
  float wmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MMAlgo>(*wmmalgo_, cache_size, acc_step_v, wmmalgo_hit_rate, accuracy_v);
  std::cout << "WMMALGO: \n"
            << "hit_rate= " << wmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MMAlgo> mmmalgo_ = boost::make_shared<MMMAlgo>(malgo_t__context_size_v);
  float mmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MMAlgo>(*mmmalgo_, cache_size, acc_step_v, mmmalgo_hit_rate, accuracy_v);
  std::cout << "MMMALGO: \n"
            << "hit_rate= " << mmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MMAlgo> bmmalgo_ = boost::make_shared<BMMAlgo>(malgo_t__context_size_v, 4);
  float bmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MMAlgo>(*bmmalgo_, cache_size, acc_step_v, bmmalgo_hit_rate, accuracy_v);
  std::cout << "BMMALGO_WND_4: \n"
            << "bmmalgo= \n" << bmmalgo_->to_str() << "\n"
            << "hit_rate= " << bmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  
  std::cout << "lz_hit_rate= " << lz_hit_rate << "\n"
            << "alz_hit_rate= " << alz_hit_rate << "\n"
            << "ppm_1_hit_rate= " << ppm_1_hit_rate << "\n"
            << "ppm_2_hit_rate= " << ppm_2_hit_rate << "\n"
            << "ppm_3_hit_rate= " << ppm_3_hit_rate << "\n"
            << "po_hit_rate= " << po_hit_rate << "\n"
            << "wmmalgo_hit_rate= " << wmmalgo_hit_rate << "\n"
            << "mmmalgo_hit_rate= " << mmmalgo_hit_rate << "\n"
            << "bmmalgo_hit_rate= " << bmmalgo_hit_rate << "\n";
}

void test_bmmalgo()
{
  int alphabet_size = 4;
  int num_acc = 1000;
  std::map<ACC_T, float> acc__arr_rate_map;
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  
  std::vector<int> acc_v;
  // gen_random_acc_seq(alphabet_size, num_acc, acc_v);
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  // gen_intermittent_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  
  std::map<ACC_T, float> acc__emp_prob_map;
  get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
  std::cout << "acc__emp_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
  std::cout << "acc_v= \n" << patch_all::vec_to_str(acc_v) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  int cache_size = 1;
  std::vector<char> accuracy_v;
  
  std::vector<malgo_t__context_size_pair> malgo_t__context_size_v;
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PO, 0) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  malgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  
  boost::shared_ptr<MMAlgo> bmmalgo_ = boost::make_shared<BMMAlgo>(malgo_t__context_size_v, 10);
  float bmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MMAlgo>(*bmmalgo_, cache_size, acc_step_v, bmmalgo_hit_rate, accuracy_v);
  std::cout << "BMMALGO_WND_4: \n"
            << "bmmalgo= \n" << bmmalgo_->to_str() << "\n"
            << "hit_rate= " << bmmalgo_hit_rate << "\n";
}

void handle_mpbuffer_data_act(PREFETCH_DATA_ACT_T data_act_t, int ds_id, key_ver_pair kv)
{
  // LOG(INFO) << "handle_mpbuffer_data_act:: data_act_t= " << data_act_t << ", ds_id= " << ds_id << KV_TO_STR(key, ver);
}

void add_access(MPBuffer* mpbuffer, key_ver_pair kv) { mpbuffer->add_access(kv); }

void m_prefetch_test()
{
  int buffer_size = 10;
  MPBuffer pbuffer('a', buffer_size, MALGO_W_PPM, //Mixed-most confident, // MALGO_W_LZ
                   true, boost::bind(handle_mpbuffer_data_act, _1, _2, _3) );
  
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
  //   add_access(&pbuffer, *kv_it);
  // }
  // std::cout << "m_prefetch_test:: p_id_v= " << patch_all::vec_to_str<int>(p_id_v) << "\n";
  // std::cout << "m_prefetch_test:: key_ver_v= \n" << patch_all::pvec_to_str<key_ver_pair>(key_ver_v) << "\n";
  
  float hit_rate;
  std::vector<char> accuracy_v;
  pbuffer.sim_prefetch_accuracy(p_id_v, key_ver_v, hit_rate, accuracy_v);
  std::cout << "accuracy_v= " << patch_all::vec_to_str<char>(accuracy_v) << "\n";
  std::cout << "hit_rate= " << hit_rate << "\n";
  
  // 
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
  
  std::cout << "pbuffer= \n" << pbuffer.to_str();
}

void mpcsim_test()
{
  int num_p, num_c;
  std::vector<int> p_id__ds_id_v, c_id__ds_id_v;
  std::vector<int> p_id__num_put_v, c_id__num_get_v;
  std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
  std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;
  
  int num_ds = 2;
  int max_num_p = 15;
  int max_num_c = 15;
  int num_putget_mean = 5;
  float put_rate_mean = 0.2;
  float get_rate_mean = 0.2;
  
  std::vector<int> ds_id_v;
  gen_scenario(num_ds, ds_id_v,
               max_num_p, max_num_c, num_putget_mean, put_rate_mean, get_rate_mean,
               num_p, num_c,
               p_id__ds_id_v, c_id__ds_id_v,
               p_id__num_put_v, c_id__num_get_v,
               p_id__put_rate_v, c_id__get_rate_v,
               p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v);
  
  bool w_prefetch = true;
  int max_num_key_ver_in_mpbuffer = 100;
  
  MPCSim mpc_sim(ds_id_v, w_prefetch,
                 num_p, num_c,
                 p_id__ds_id_v, c_id__ds_id_v,
                 p_id__num_put_v, c_id__num_get_v,
                 p_id__put_rate_v, c_id__get_rate_v,
                 p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v,
                 MALGO_W_PPM, max_num_key_ver_in_mpbuffer);
  
  mpc_sim.sim_all();
  // 
  std::string temp;
  std::cout << "Enter\n";
  getline(std::cin, temp);
}

#endif // _PATCH_MARKOV_EXP_H_