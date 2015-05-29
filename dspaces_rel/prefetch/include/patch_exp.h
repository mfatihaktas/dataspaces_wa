#ifndef _PATCH_EXP_H_
#define _PATCH_EXP_H_

#include <boost/math/distributions/normal.hpp>

#include "gnuplot-iostream.h"

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
    gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key left top\n";
  gp << "set title '" << plot_title << "'\n";
  gp << "set style line 1 lc rgb '#7F7F7F' lt 1 lw 2 pt 5 ps 1.5\n";
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
  std::string color_code_[] = {"#7F7F7F", "#0060ad", "#D2691E", "#DC143C", "#556B2F", "#DAA520"};
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
    gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key left top\n";
  gp << "set title '" << plot_title << "'\n";
  gp << "set xrange [" << min_x << ":" << max_x*1.2 << "]\nset yrange [" << min_y << ":" << max_y*1.2 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set grid\n";
  
  
  for (int i = 0; i < x_v_v.size(); i++)
    gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' lt 1 lw 2 pt 5 ps 1.5\n";
  
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

// ----------------------------------------  palgo_test  -----------------------------------------//
void gen_random_acc_seq(size_t alphabet_size, size_t num_acc, std::vector<ACC_T>& acc_v)
{
  srand(time(NULL) );
  
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
  
  srand(time(NULL) );
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
    
    acc_step_v.push_back(std::make_pair(*it, acc_step_map[*it]) );
    acc_step_map[*it] += 1;
  }
}

// ----------------------------------------  sim_test  -------------------------------------------//
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

#endif // _PATCH_EXP_H_