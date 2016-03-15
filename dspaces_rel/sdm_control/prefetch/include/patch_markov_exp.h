#ifndef _PATCH_MARKOV_EXP_H_
#define _PATCH_MARKOV_EXP_H_

#include <algorithm>

#include <string>
#include <cstdlib>
#include <ctime>
// For exec
#include <iostream>
#include <cstdio>
#include <memory>

#include <boost/assign/list_of.hpp>
// #include <boost/math/distributions/normal.hpp>
// #include <boost/math/distributions/exponential.hpp>
#include <boost/math/distributions.hpp>

#include "gnuplot_iostream.h"

std::string exec(std::string cmd)
{
  boost::shared_ptr<FILE> pipe_(popen(cmd.c_str(), "r"), pclose);
  if (!pipe_) {
    log_(ERROR, "opening pipe_ failed!")
    return "ERROR";
  }
  
  char buffer_[128];
  std::string result = "";
  while (!feof(pipe_.get() ) ) {
    if (fgets(buffer_, 128, pipe_.get() ) != NULL)
      result += buffer_;
  }
  return result;
}

// Note: enclosing any latex code ($code$) with '' is required
template<typename T>
void make_latex_plot(std::vector<std::vector<T> > x_v_v, std::vector<std::vector<T> > y_v_v, std::vector<std::string> title_v,
                     std::string x_label, std::string y_label,
                     std::string plot_title, std::string out_name)
{
  std::cout << "x_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
    std::cout << patch::vec_to_str<>(*it) << "\n";
  
  std::cout << "y_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
    std::cout << patch::vec_to_str<>(*it) << "\n";
  // 
  std::string color_code_[] = {"#0060ad", "#DAA520", "#7F7F7F", "#D2691E", "#556B2F", "#DC143C", "#DA70D6", "#8B008B", "#1E90FF", "#9ACD32"};
  
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
  
  // out_name = "gnuplot/" + out_name;
  {
    Gnuplot gp;
    gp << "set terminal epslatex standalone color colortext font ',8' size 20cm,15cm\n";
    gp << "set output '" << out_name << ".tex'\n";
    gp << "set key right bottom\n";
    // gp << "set key outside\n";
    // gp << "set key width -8\n";
    gp << "set title '" << plot_title << "'\n"; // offset 0,1 // to lift title up
    gp << "set xrange [" << min_x*0.9 << ":" << max_x*1.1 << "]\nset yrange [" << min_y*0.95 << ":" << max_y*1.1 << "]\n";
    gp << "set xlabel '" << x_label << "'\n";
    gp << "set ylabel '" << y_label << "'\n";
    gp << "set grid\n";
    
    for (int i = 0; i < x_v_v.size(); i++)
      gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' " << "pt " << boost::lexical_cast<std::string>(i + 1) << " lt 1 lw 2 ps 1\n";
    
    gp << "plot ";
    gp << "x lw 1 lt 3 title 'y=x',";
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
  // log_(INFO, "exec(latex " << out_name << ".tex) returned= \n"
  //           << exec("latex " + out_name + ".tex") << "\n"
  //           << "exec(dvips -o " << out_name << ".ps " << out_name << ".dvi) returned= \n"
  //           << exec("dvips -o " + out_name + ".ps " + out_name + ".dvi") )
  log_(INFO, "exec(latex " << out_name << ".tex) returned= \n"
             << exec("latex " + out_name + ".tex") )
  log_(INFO, "exec(dvips -o " << out_name << ".ps " << out_name << ".dvi) returned= \n"
             << exec("dvips -o " + out_name + ".ps " + out_name + ".dvi") )
}

template<typename T>
void make_plot(std::vector<std::vector<T> > x_v_v, std::vector<std::vector<T> > y_v_v, std::vector<std::string> title_v,
               std::string x_label, std::string y_label,
               std::string plot_title, std::string out_url)
{
  std::cout << "x_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
    std::cout << patch::vec_to_str<>(*it) << "\n";
  
  std::cout << "y_v_v= \n";
  for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
    std::cout << patch::vec_to_str<>(*it) << "\n";
  // 
  std::string color_code_[] = {"#0060ad", "#DAA520", "#7F7F7F", "#D2691E", "#556B2F", "#DC143C", "#DA70D6", "#8B008B", "#1E90FF", "#9ACD32"};
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
    // gp << "set term png size 1200,800 enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    // gp << "set term post eps enh \"Helvetica\" 12 size 5,4\n"; // For Symbols
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key right top\n";
  gp << "set key outside\n";
  // gp << "set title \"" << plot_title << "\"\n";
  gp << "set xrange [" << min_x*0.9 << ":" << max_x*1.05 << "]\nset yrange [" << min_y*0.95 << ":" << max_y*1.1 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set grid\n";
  
  for (int i = 0; i < x_v_v.size(); i++)
    gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' " << "pt " << boost::lexical_cast<std::string>(i + 1) << " lt 1 lw 2 ps 1\n";
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
  
//   std::cout << "v= " << patch::vec_to_str<>(v) << "\n";
  
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
//     std::cout << "i= " << i << ", int_num_map= \n" << patch::map_to_str<>(i__int_freq_map_v[i] ) << "\n";
// }

// ----------------------------------------  test_malgo  -----------------------------------------//
int gen_real_acc_seq(int alphabet_size, int num_acc,
                     float min_inter_acc_time, float max_inter_acc_time, float stdev,
                     std::vector<ACC_T>& acc_v,
                     std::vector<arr_time__acc_pair>& arr_time__acc_pair_v)
{
  std::map<float, ACC_T> time_acc_map;
  for (ACC_T acc = 1; acc <= alphabet_size; acc++) {
    float inter_acc_time_mean = rand() % (int)(max_inter_acc_time - min_inter_acc_time) + min_inter_acc_time;
    log_(INFO, "acc= " << acc << ", inter_acc_time_mean= " << inter_acc_time_mean)
    // boost::math::exponential_distribution<float> inter_acc_time_dist((float)1/inter_acc_time_mean);
    boost::math::normal_distribution<float> inter_acc_time_dist(inter_acc_time_mean, stdev);
    
    float last_acc_time = 0;
    for (int i = 0; i < num_acc; i++) {
      float inter_acc_time = (float)std::abs(quantile(inter_acc_time_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
      // float inter_acc_time = inter_acc_time_mean;
      last_acc_time += inter_acc_time;
      while (time_acc_map.count(last_acc_time) != 0)
        last_acc_time += 0.001;
      time_acc_map[last_acc_time] = acc;
    }
  }
  // log_(INFO, "time_acc_map= \n" << patch::map_to_str<>(time_acc_map) )
  
  for (std::map<float, ACC_T>::iterator it = time_acc_map.begin(); it != time_acc_map.end(); it++) {
    acc_v.push_back(it->second);
    arr_time__acc_pair_v.push_back(std::make_pair(it->first, it->second) );
  }
  
  return 0;
}

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
    acc__emp_prob_map[*it] += (float)1/num_acc;
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
void acc_v_to_acc_step_v(std::vector<ACC_T>& acc_v, std::vector<acc_step_pair>& acc_step_v,
                         std::map<ACC_T, int> acc__last_acced_step_map = std::map<ACC_T, int>() )
{
  // std::map<ACC_T, int> acc__last_acced_step_map;
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (acc__last_acced_step_map.count(*it) == 0)
      acc__last_acced_step_map[*it] = -1;
    
    acc__last_acced_step_map[*it] += 1;
    acc_step_v.push_back(std::make_pair(*it, acc__last_acced_step_map[*it] ) );
  }
}

// ----------------------------------------  sim_test  -------------------------------------------//
#define STDEV 2
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
    boost::math::normal_distribution<float> num_putget_dist(num_putget_mean, STDEV);
    int num_put = (int) std::abs(quantile(num_putget_dist, (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) );
    
    // TODO: may be randomized
    p_id__ds_id_v.push_back(ds_id_v[0] ); 
    
    p_id__num_put_v.push_back(num_put);
    
    boost::math::normal_distribution<float> put_rate_dist(put_rate_mean, STDEV);
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
    
    boost::math::normal_distribution<float> get_rate_dist(get_rate_mean, STDEV);
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

  boost::math::normal_distribution<float> inter_arr_time_dist(1, STDEV);
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
void test_malgo()
{
  // boost::shared_ptr<PAlgo> palgo_ = boost::make_shared<LZAlgo>();
  // boost::shared_ptr<PAlgo> palgo_ = boost::make_shared<ALZAlgo>();
  // boost::shared_ptr<PAlgo> palgo_ = boost::make_shared<POAlgo>();
  
  // std::map<MALGO_T, float> prefetch_t__weight_map;
  // prefetch_t__weight_map[MALGO_W_LZ] = 0.5;
  // prefetch_t__weight_map[MALGO_W_PPM] = 0.5;
  // boost::shared_ptr<MPAlgo> palgo_ = boost::make_shared<MPAlgo>(Mixed-blended, prefetch_t__weight_map);
  // boost::shared_ptr<MPAlgo> palgo_ = boost::make_shared<MPAlgo>(Mixed-most confident, prefetch_t__weight_map);
  
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
  std::cout << "acc__emp_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
  std::cout << "acc_v= \n" << patch::vec_to_str(acc_v) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  int cache_size = 1;
  std::vector<char> accuracy_v;
  
  boost::shared_ptr<PAlgo> lz_algo_ = boost::make_shared<LZAlgo>();
  float lz_hit_rate;
  sim_prefetch_accuracy<PAlgo>(*lz_algo_, cache_size, acc_step_v, lz_hit_rate, accuracy_v);
  std::cout << "LZ_ALGO: \n"
            << "hit_rate= " << lz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << lz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_v= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PAlgo> alz_algo_ = boost::make_shared<ALZAlgo>();
  float alz_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PAlgo>(*alz_algo_, cache_size, acc_step_v, alz_hit_rate, accuracy_v);
  std::cout << "ALZ_ALGO: \n"
            << "hit_rate= " << alz_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << alz_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PAlgo> ppm_1_algo_ = boost::make_shared<PPMAlgo>(1);
  float ppm_1_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PAlgo>(*ppm_1_algo_, cache_size, acc_step_v, ppm_1_hit_rate, accuracy_v);
  std::cout << "PPM_1_ALGO: \n"
            << "hit_rate= " << ppm_1_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_1_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PAlgo> ppm_2_algo_ = boost::make_shared<PPMAlgo>(2);
  float ppm_2_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PAlgo>(*ppm_2_algo_, cache_size, acc_step_v, ppm_2_hit_rate, accuracy_v);
  std::cout << "PPM_2_ALGO: \n"
            << "hit_rate= " << ppm_2_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_2_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PAlgo> ppm_3_algo_ = boost::make_shared<PPMAlgo>(3);
  float ppm_3_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PAlgo>(*ppm_3_algo_, cache_size, acc_step_v, ppm_3_hit_rate, accuracy_v);
  std::cout << "PPM_3_ALGO: \n"
            << "hit_rate= " << ppm_3_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << ppm_3_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<PAlgo> po_algo_ = boost::make_shared<POAlgo>();
  float po_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<PAlgo>(*po_algo_, cache_size, acc_step_v, po_hit_rate, accuracy_v);
  std::cout << "PO_ALGO: \n"
            << "hit_rate= " << po_hit_rate << "\n";
  // // std::cout << "parse_tree_to_pstr= \n" << po_algo_->parse_tree_to_pstr() << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PO, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  boost::shared_ptr<MPAlgo> wmmalgo_ = boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v);
  float wmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPAlgo>(*wmmalgo_, cache_size, acc_step_v, wmmalgo_hit_rate, accuracy_v);
  std::cout << "WMMALGO: \n"
            << "hit_rate= " << wmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MPAlgo> mmmalgo_ = boost::make_shared<MMPAlgo>(palgo_t__context_size_v);
  float mmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPAlgo>(*mmmalgo_, cache_size, acc_step_v, mmmalgo_hit_rate, accuracy_v);
  std::cout << "MMMALGO: \n"
            << "hit_rate= " << mmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
  boost::shared_ptr<MPAlgo> bmmalgo_ = boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4);
  float bmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPAlgo>(*bmmalgo_, cache_size, acc_step_v, bmmalgo_hit_rate, accuracy_v);
  std::cout << "BMMALGO_WND_4: \n"
            << "bmmalgo= \n" << bmmalgo_->to_str() << "\n"
            << "hit_rate= " << bmmalgo_hit_rate << "\n";
  // std::cout << "accuracy_seq= \n" << patch::vec_to_str<char>(accuracy_v) << "\n";
  
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
  std::cout << "acc__emp_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
  std::cout << "acc_v= \n" << patch::vec_to_str(acc_v) << "\n";
  
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  int cache_size = 1;
  std::vector<char> accuracy_v;
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PO, 0) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  
  boost::shared_ptr<MPAlgo> bmmalgo_ = boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 10);
  float bmmalgo_hit_rate;
  accuracy_v.clear();
  sim_prefetch_accuracy<MPAlgo>(*bmmalgo_, cache_size, acc_step_v, bmmalgo_hit_rate, accuracy_v);
  std::cout << "BMMALGO_WND_4: \n"
            << "bmmalgo= \n" << bmmalgo_->to_str() << "\n"
            << "hit_rate= " << bmmalgo_hit_rate << "\n";
}

void handle_mpbuffer_data_act(PREFETCH_DATA_ACT_T data_act_t, int ds_id, key_ver_pair kv)
{
  log_(INFO, "data_act_t= " << data_act_t << ", ds_id= " << ds_id << KV_TO_STR(kv.first, kv.second) )
}

void add_access(MPBuffer* mpbuffer, key_ver_pair kv) { mpbuffer->add_access(kv); }

void m_prefetch_test()
{
  int buffer_size = 10;
  MPBuffer pbuffer(0, buffer_size, MALGO_W_PPM, //Mixed-most confident, // MALGO_W_LZ
                   true, boost::bind(handle_mpbuffer_data_act, _1, _2, _3) );
  
  int p_id_[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int num_p = sizeof(p_id_)/sizeof(*p_id_);
  // int num_put = 20;
  
  int num_acc = 50; // 30 * (rand() % 10) / 10;
  std::vector<int> p_id_v;
  std::vector<key_ver_pair> key_ver_v;
  std::map<int, int> p_id__last_step_map;
  for (int i = 0; i < num_acc; i++) {
    int p_id = rand() % num_p;
    p_id_v.push_back(p_id);
    
    if (p_id__last_step_map.count(p_id) == 0)
      p_id__last_step_map[p_id] = -1;
    
    key_ver_v.push_back(std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(p_id__last_step_map[p_id] + 1), 0) );
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
  // std::cout << "m_prefetch_test:: p_id_v= " << patch::vec_to_str<int>(p_id_v) << "\n";
  // std::cout << "m_prefetch_test:: key_ver_v= \n" << patch::pvec_to_str<key_ver_pair>(key_ver_v) << "\n";
  
  float hit_rate;
  std::vector<char> accuracy_v;
  pbuffer.sim_prefetch_accuracy(p_id_v, key_ver_v, hit_rate, accuracy_v);
  std::cout << "accuracy_v= " << patch::vec_to_str<char>(accuracy_v) << "\n"
            << "hit_rate= " << hit_rate << "\n";
  
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

// ------------------------------------------------------------------------------------------------/
void test_fixed_train()
{
  std::vector<std::string> title_v;
  
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  palgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  title_v.push_back("ppm order 3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(6) );
  // title_v.push_back("ppm order 6");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(8) );
  // title_v.push_back("ppm order 8");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(10) );
  // title_v.push_back("ppm order 10");
  
    std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 2) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.5)(0.5);
  palgo_v.push_back(boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v) );
  title_v.push_back("mixed-blended");
  
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  palgo_v.push_back(boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4) );
  title_v.push_back("mixed-best wnd 4");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 10;
  int num_acc = 200; // 20;
  std::vector<int> acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  // gen_intermittent_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, acc_v);
  // acc_v_to_acc_step_v(acc_v, acc_step_v);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  std::vector<boost::shared_ptr<Cache<ACC_T, acc_step_pair> > > algo_id__cache_v;
  for (int i = 0; i < num_algo; i++)
    algo_id__cache_v.push_back(boost::make_shared<Cache<ACC_T, acc_step_pair> >(1, boost::function<void(acc_step_pair)>() ) );
  std::vector<std::map<ACC_T, int> > algo_id__acc__last_acced_step_map_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  int num_run = 50;
  
  int num_filtering_run = 20;
  for (int f = 0; f < num_filtering_run; f++) {
    for (int i = 1; i <= num_run; i++) {
      std::map<ACC_T, float> acc__emp_prob_map;
      get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
      std::cout << "f= " << f << ", i= " << i << ", acc__emp_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
      
      // acc_step_v.clear();
      // acc_v_to_acc_step_v(acc_v, acc_step_v);
      // std::cout << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n";
      
      int algo_id = 0;
      for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
        if (i == 1) {
          (*it)->reset();
          algo_id__cache_v[algo_id]->reset();
          (algo_id__acc__last_acced_step_map_v[algo_id] ).clear();
        }
        acc_step_v.clear();
        acc_v_to_acc_step_v(acc_v, acc_step_v, algo_id__acc__last_acced_step_map_v[algo_id] );
        
        accuracy_v.clear();
        sim_prefetch_accuracy<PAlgo>(**it, *(algo_id__cache_v[algo_id] ), algo_id__acc__last_acced_step_map_v[algo_id],
                                     acc_step_v, hit_rate, accuracy_v);
        if (f == 0) {
          hit_rate_v_v[algo_id].push_back(hit_rate);
          run_i_v_v[algo_id].push_back(i);
        }
        else {
          hit_rate_v_v[algo_id][i - 1] += hit_rate;
        }
      }
    }
  }
  for (int algo_id = 0; algo_id < num_algo; algo_id++) {
    for (int i = 0; i < num_run; i++)
      hit_rate_v_v[algo_id][i] = (float)hit_rate_v_v[algo_id][i] / num_filtering_run;
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Avg Hit rate after training with fixed pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern size= " << num_acc;
  
  std::string out_url = ""; //"/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/fig_hit_rate_w_fixed.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the fixed pattern", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_fixed.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the fixed pattern", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  std::cout << "acc_step_v= " << patch::pvec_to_str<>(acc_step_v) << "\n";
}

void check_acc_step_v(std::vector<acc_step_pair> acc_step_v)
{
  std::map<ACC_T, int> acc__last_acced_step_map;
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    if (acc__last_acced_step_map.count(it->first) == 0) {
      acc__last_acced_step_map[it->first] = it->second;
    }
    else {
      if (it->second > acc__last_acced_step_map[it->first] + 1) {
        log_(ERROR, "*** wrong ver; step= " << it->second << ", last_acced_step= " << acc__last_acced_step_map[it->first] )
      }
      acc__last_acced_step_map[it->first] = it->second;
    }
  }
}

void test_rand_shuffle_train()
{
  std::vector<std::string> title_v;
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  palgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  title_v.push_back("ppm order 3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(6) );
  // title_v.push_back("ppm order 6");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(8) );
  // title_v.push_back("ppm order 8");
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 3) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.5)(0.5);
  // palgo_v.push_back(boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v) );
  // title_v.push_back("mixed-blended");
  
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  // palgo_v.push_back(boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4) );
  // title_v.push_back("mixed-best wnd 4");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 10;
  int num_acc = 200; // 50;
  std::vector<int> initial_acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, initial_acc_v);
  std::cout << "initial_acc_v= " << patch::vec_to_str<>(initial_acc_v) << "\n";
  
  int shuffle_width = 4;
  float shuffle_prob = 1;
  std::vector<int> shuffle_indices;
  for (int i = 0; i < num_acc/10; i++)
    shuffle_indices.push_back(rand() % num_acc);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  int cache_size = 1;
  std::vector<boost::shared_ptr<Cache<ACC_T, acc_step_pair> > > algo_id__cache_v;
  for (int i = 0; i < num_algo; i++)
    algo_id__cache_v.push_back(boost::make_shared<Cache<ACC_T, acc_step_pair> >(cache_size, boost::function<void(acc_step_pair)>() ) );
  std::vector<std::map<ACC_T, int> > algo_id__acc__last_acced_step_map_v(num_algo);
  std::vector<std::vector<acc_step_pair> > algo_id__complete_acc_step_v_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  int num_run = 20;
  
  int num_filtering_run = 10;
  for (int f = 0; f < num_filtering_run; f++) {
    for (int i = 1; i <= num_run; i++) {
      // shuffle_indices.clear();
      // for (int j = 0; j < num_acc/10; j++)
      //   shuffle_indices.push_back(rand() % num_acc);
      std::vector<ACC_T> acc_v(initial_acc_v.begin(), initial_acc_v.end() );
      random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
      
      std::map<ACC_T, float> acc__emp_prob_map;
      get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
      std::cout << "f= " << f << ", i= " << i << ", acc__emp_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
      
      // acc_step_v.clear();
      // acc_v_to_acc_step_v(acc_v, acc_step_v);
      // std::cout << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n";
      
      int algo_id = 0;
      for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
        if (i == 1) {
          (*it)->reset();
          algo_id__cache_v[algo_id]->reset();
          (algo_id__acc__last_acced_step_map_v[algo_id] ).clear();
        }
        acc_step_v.clear();
        acc_v_to_acc_step_v(acc_v, acc_step_v, algo_id__acc__last_acced_step_map_v[algo_id] );
        // DEBUG
        std::vector<acc_step_pair>& complete_acc_step_v = algo_id__complete_acc_step_v_v[algo_id];
        complete_acc_step_v.insert(complete_acc_step_v.end(), acc_step_v.begin(), acc_step_v.end() );
        
        accuracy_v.clear();
        sim_prefetch_accuracy<PAlgo>(**it, *(algo_id__cache_v[algo_id] ), algo_id__acc__last_acced_step_map_v[algo_id],
                                     acc_step_v, hit_rate, accuracy_v);
        if (f == 0) {
          hit_rate_v_v[algo_id].push_back(hit_rate);
          run_i_v_v[algo_id].push_back(i);
        }
        else {
          hit_rate_v_v[algo_id][i - 1] += hit_rate;
        }
      }
    }
  }
  for (int algo_id = 0; algo_id < num_algo; algo_id++) {
    for (int i = 0; i < num_run; i++)
      hit_rate_v_v[algo_id][i] = (float)hit_rate_v_v[algo_id][i] / num_filtering_run;
    // DEBUG
    check_acc_step_v(algo_id__complete_acc_step_v_v[algo_id] );
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Avg Hit rate after training with noisy pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern size= " << num_acc;
  
  std::string out_url = ""; // "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/img/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the noisy pattern", "Avg Hit rate, Cache size " + boost::lexical_cast<std::string>(cache_size),
                   plot_title_ss.str(), out_url);

  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the noisy pattern", "Avg Hit rate, Cache size " + boost::lexical_cast<std::string>(cache_size),
                   plot_title_ss.str(), out_url);
  
  std::cout << "acc_step_v= " << patch::pvec_to_str<>(acc_step_v) << "\n";
}

void test_rand_shuffle_train_w_varying_cache()
{
  std::vector<int> cache_size_v;
  std::vector<std::string> title_v;
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  // palgo_v.push_back(boost::make_shared<LZAlgo>() );
  // title_v.push_back("lz");
  // palgo_v.push_back(boost::make_shared<POAlgo>() );
  // cache_size_v.push_back(1);
  // title_v.push_back("poisson cache 1");
  // palgo_v.push_back(boost::make_shared<POAlgo>() );
  // cache_size_v.push_back(4);
  // title_v.push_back("poisson cache 4");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  // title_v.push_back("ppm order 1");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  // title_v.push_back("ppm order 2");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  // title_v.push_back("ppm order 3");
  // palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  // title_v.push_back("ppm order 4");
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 3) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.5)(0.5);
  // palgo_v.push_back(boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v) );
  // title_v.push_back("mixed-blended");
  
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  cache_size_v.push_back(1);
  title_v.push_back("mixed-most confident cache 1");
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  cache_size_v.push_back(4);
  title_v.push_back("mixed-most confident cache 4");
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  cache_size_v.push_back(10);
  title_v.push_back("mixed-most confident cache 10");
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  cache_size_v.push_back(15);
  title_v.push_back("mixed-most confident cache 15");
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  cache_size_v.push_back(20);
  title_v.push_back("mixed-most confident cache 20");
  
  // palgo_v.push_back(boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4) );
  // title_v.push_back("mixed-best wnd 4");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 10;
  int num_acc = 200; // 50;
  std::vector<int> initial_acc_v;
  std::vector<acc_step_pair> acc_step_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, initial_acc_v);
  std::cout << "initial_acc_v= " << patch::vec_to_str<>(initial_acc_v) << "\n";
  
  int shuffle_width = 4;
  float shuffle_prob = 1;
  std::vector<int> shuffle_indices;
  for (int i = 0; i < num_acc/10; i++)
    shuffle_indices.push_back(rand() % num_acc);
  // 
  std::vector<std::vector<float> > run_i_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  std::vector<boost::shared_ptr<Cache<ACC_T, acc_step_pair> > > algo_id__cache_v;
  for (int i = 0; i < num_algo; i++)
    algo_id__cache_v.push_back(boost::make_shared<Cache<ACC_T, acc_step_pair> >(cache_size_v[i], boost::function<void(acc_step_pair)>() ) );
  std::vector<std::map<ACC_T, int> > algo_id__acc__last_acced_step_map_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  int num_run = 50;
  
  int num_filtering_run = 10;
  for (int f = 0; f < num_filtering_run; f++) {
    for (int i = 1; i <= num_run; i++) {
      // shuffle_indices.clear();
      // for (int j = 0; j < num_acc/10; j++)
      //   shuffle_indices.push_back(rand() % num_acc);
      std::vector<ACC_T> acc_v(initial_acc_v.begin(), initial_acc_v.end() );
      random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, acc_v);
      
      std::map<ACC_T, float> acc__emp_prob_map;
      get_emprical_dist(alphabet_size, acc_v, acc__emp_prob_map);
      std::cout << "f= " << f << ", i= " << i << ", acc__emp_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__emp_prob_map) << "\n";
      
      // acc_step_v.clear();
      // acc_v_to_acc_step_v(acc_v, acc_step_v);
      // std::cout << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n";
      
      int algo_id = 0;
      for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++, algo_id++) {
        if (i == 1) {
          (*it)->reset();
          algo_id__cache_v[algo_id]->reset();
          (algo_id__acc__last_acced_step_map_v[algo_id] ).clear();
        }
        acc_step_v.clear();
        acc_v_to_acc_step_v(acc_v, acc_step_v, algo_id__acc__last_acced_step_map_v[algo_id] );
        
        accuracy_v.clear();
        sim_prefetch_accuracy<PAlgo>(**it, *(algo_id__cache_v[algo_id] ), algo_id__acc__last_acced_step_map_v[algo_id],
                                     acc_step_v, hit_rate, accuracy_v);
        if (f == 0) {
          hit_rate_v_v[algo_id].push_back(hit_rate);
          run_i_v_v[algo_id].push_back(i);
        }
        else {
          hit_rate_v_v[algo_id][i - 1] += hit_rate;
        }
      }
    }
  }
  for (int algo_id = 0; algo_id < num_algo; algo_id++) {
    for (int i = 0; i < num_run; i++)
      hit_rate_v_v[algo_id][i] = (float)hit_rate_v_v[algo_id][i] / num_filtering_run;
  }
  
  std::stringstream plot_title_ss;
  plot_title_ss << "Avg Hit rate after training with noisy pattern; "
                << "alphabet size= " << alphabet_size
                << ", pattern size= " << num_acc;
  
  std::string out_url = ""; // "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/prefetch/img/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the noisy pattern", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_rand_partial_shuffle.png";
  make_plot<float>(run_i_v_v, hit_rate_v_v, title_v,
                   "Number of observed repetitions of the noisy pattern", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  std::cout << "acc_step_v= " << patch::pvec_to_str<>(acc_step_v) << "\n";
}

void plot_hit_rate_vs_cache_size()
{
  std::vector<std::string> title_v;
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  palgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  title_v.push_back("ppm order 3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 3) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.5)(0.5);
  // palgo_v.push_back(boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v) );
  // title_v.push_back("mixed-blended");
  
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  // palgo_v.push_back(boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4) );
  // title_v.push_back("mixed-best wnd 4");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 10;
  int num_acc = 200; // 50;
  std::vector<int> initial_acc_v;
  std::map<ACC_T, float> acc__arr_rate_map;
  // 
  for (ACC_T a = 0; a < alphabet_size; a++)
    acc__arr_rate_map[a] = 1 + static_cast<float>(rand() ) / static_cast<float>(RAND_MAX); // (float) 1 / alphabet_size;
  gen_poisson_acc_seq(alphabet_size, num_acc, acc__arr_rate_map, initial_acc_v);
  std::cout << "initial_acc_v= " << patch::vec_to_str<>(initial_acc_v) << "\n";
  
  int shuffle_width = 4;
  float shuffle_prob = 1;
  std::vector<int> shuffle_indices;
  for (int i = 0; i < num_acc/10; i++)
    shuffle_indices.push_back(rand() % num_acc);
  // 
  std::vector<std::vector<float> > cache_size_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  std::vector<boost::shared_ptr<Cache<ACC_T, acc_step_pair> > > algo_id__cache_v;
  std::vector<std::map<ACC_T, int> > algo_id__acc__last_acced_step_map_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  int num_run = 20;
  std::vector<ACC_T> acc_v;
  std::vector<acc_step_pair> acc_step_v;
  for (int i = 1; i <= num_run; i++) {
    // shuffle_indices.clear();
    // for (int j = 0; j < num_acc/10; j++)
    //   shuffle_indices.push_back(rand() % num_acc);
    std::vector<ACC_T> t_acc_v(initial_acc_v.begin(), initial_acc_v.end() );
    random_partial_shuffle<ACC_T>(shuffle_prob, shuffle_width, shuffle_indices, t_acc_v);
    acc_v.insert(acc_v.end(), t_acc_v.begin(), t_acc_v.end() );
  }
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  
  int num_filtering_run = 10;
  for (int cache_size = 1; cache_size <= alphabet_size; cache_size++) {
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
        sim_prefetch_accuracy<PAlgo>(**it, cache_size, acc_step_v, hit_rate, accuracy_v);
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
                   "Cache size", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_vs_cache_size.png";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  // std::cout << "acc_step_v= " << patch::pvec_to_str<>(acc_step_v) << "\n";
}

void plot_hit_rate_w_real()
{
  std::vector<std::string> title_v;
  std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  palgo_v.push_back(boost::make_shared<LZAlgo>() );
  title_v.push_back("lz");
  palgo_v.push_back(boost::make_shared<POAlgo>() );
  title_v.push_back("poisson");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(1) );
  title_v.push_back("ppm order 1");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(2) );
  title_v.push_back("ppm order 2");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(3) );
  title_v.push_back("ppm order 3");
  palgo_v.push_back(boost::make_shared<PPMAlgo>(4) );
  title_v.push_back("ppm order 4");
  
  std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_LZ, 0) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 1) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 3) );
  palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 4) );
  // palgo_t__context_size_v.push_back(std::make_pair(MALGO_W_PPM, 8) );
  
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.2)(0.2)(0.2)(0.2)(0.2);
  // std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.25)(0.25)(0.25)(0.25);
  std::vector<float> palgo_id__weight_v = boost::assign::list_of(0.5)(0.5);
  // palgo_v.push_back(boost::make_shared<WMPAlgo>(palgo_t__context_size_v, palgo_id__weight_v) );
  // title_v.push_back("mixed-blended");
  
  palgo_v.push_back(boost::make_shared<MMPAlgo>(palgo_t__context_size_v) );
  title_v.push_back("mixed-most confident");
  
  // palgo_v.push_back(boost::make_shared<BMPAlgo>(palgo_t__context_size_v, 4) );
  // title_v.push_back("mixed-best wnd 4");
  
  int num_algo = palgo_v.size();
  
  int alphabet_size = 20; // 10;
  int num_acc = 200; // 50;
  
  std::vector<ACC_T> acc_v;
  std::vector<arr_time__acc_pair> arr_time__acc_pair_v;
  gen_real_acc_seq(alphabet_size, num_acc, 20, 100, 1, acc_v, arr_time__acc_pair_v);
  log_(INFO, "acc_v= " << patch::vec_to_str<>(acc_v) )
  std::vector<acc_step_pair> acc_step_v;
  acc_v_to_acc_step_v(acc_v, acc_step_v);
  // log_(INFO, "acc_v= " << patch::vec_to_str<>(acc_v) )
  // 
  std::vector<std::vector<float> > cache_size_v_v(num_algo);
  std::vector<std::vector<float> > hit_rate_v_v(num_algo);
  // 
  float hit_rate;
  std::vector<char> accuracy_v;
  
  int num_filtering_run = 3;
  // for (int cache_size = 1; cache_size <= alphabet_size; cache_size++) {
  for (int cache_size = 1; cache_size <= 3; cache_size++) {
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
        sim_prefetch_accuracy<PAlgo>(**it, cache_size, acc_step_v, hit_rate, accuracy_v);
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
                   "Cache size", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_vs_cache_size.png";
  make_plot<float>(cache_size_v_v, hit_rate_v_v, title_v,
                   "Cache size", "Avg Hit rate",
                   plot_title_ss.str(), out_url);
  
  // std::cout << "acc_step_v= " << patch::pvec_to_str<>(acc_step_v) << "\n";
}

#endif // _PATCH_MARKOV_EXP_H_