#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>

#include "patch.h"
#include "profiler.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
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

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map[(std::string)"type"] = optarg;
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
  log_(INFO, "opt_map= \n" << patch::map_to_str<>(opt_map) )
  return opt_map;
}

template<typename T>
void make_plot(std::vector<T> x1_v, std::vector<T> y1_v, std::string title1,
               std::vector<T> x2_v, std::vector<T> y2_v, std::string title2,
               std::string x_label, std::string y_label, 
               std::string title,
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
  gp << "set title '" << title << "'\n";
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
  gp << "set ytics add (" << boost::lexical_cast<std::string>(min_y) << ")\n";
  for (T f = 0; f < max_y; f += 20) {
    if (f > min_y)
      gp << "set ytics add (" << boost::lexical_cast<std::string>(f) << ")\n";
  }
  
  gp << "plot '-' u 1:2 title '" << title1 << "' w linespoints ls 1, "
          << "'-' u 1:2 title '" << title2 << "' w linespoints ls 2\n";
  gp.send1d(boost::make_tuple(x1_v, y1_v) );
  gp.send1d(boost::make_tuple(x2_v, y2_v) );
}

#define RANDOM_INT_RANGE 100

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  // 
  if (str_cstr_equals(opt_map["type"], "plot") ) {
  // TProfiler<std::string> t_profiler();
  TProfiler<int> t_profiler;
  
  srand(time(NULL));
  size_t size = 10;
  std::vector<float> x1_v, y1_v, x2_v, y2_v;
  for (int i = 0; i < size; i++) {
    x1_v.push_back(i + 1);
    y1_v.push_back(1.0*i + 5);
    x2_v.push_back(i + 1);
    y2_v.push_back(1.0*i + 2.5);
    // x_v.push_back(3*(float)(rand() % RANDOM_INT_RANGE)/RANDOM_INT_RANGE);
    // y_v.push_back(3*(float)(rand() % RANDOM_INT_RANGE)/RANDOM_INT_RANGE);
  }
  
  // std::cout << "x1_v= " << patch::vector_to_str<float>(x1_v) << "\n"
  //           << "y1_v= " << patch::vector_to_str<float>(y1_v) << "\n"
  //           << "x2_v= " << patch::vector_to_str<float>(x2_v) << "\n"
  //           << "y2_v= " << patch::vector_to_str<float>(y2_v) << "\n";
  
  // make_plot<float>(x1_v, y1_v, "deneme1",
  //                 x2_v, y2_v, "deneme2", "");
  
  // for(int i = 0; i < 10; i++) {
  //   t_profiler.add_event(i, "event_" + boost::lexical_cast<std::string>(i) );
  //   sleep(1);
  // }
  
  // for(int i = 0; i < 10; i++) {
  //   t_profiler.end_event(i);
  // }
  
  // std::cout << "t_profiler= \n" << t_profiler.to_str();
  }
  else if (str_cstr_equals(opt_map["type"], "plot") ) {
    // w/o slack
    // float x1_[] = {0.125, 1, 8, 64, 512};
    // float y1_[] = {29.4, 29.1, 31.5, 37.4, 150.7};
    // float x2_[] = {0.125, 1, 8, 64, 512};
    // float y2_[] = {24.4, 25.6, 25.6, 33.1, 100.2};
    
    // w/ slack
    float x1_[] = {0.125, 1, 8, 64, 512};
    float y1_[] = {29.4, 29.1, 31.5, 37.4, 150.7};
    float x2_[] = {0.125, 1, 8, 64, 512};
    float y2_[] = {11.5, 11.7, 12.7, 20.2, 73.7};
    
    std::vector<float> x1_v;
    arr_to_vec<float>(sizeof(x1_) / sizeof(*x1_), x1_, x1_v);
    std::cout << "x1_v= " << vec_to_str(x1_v) << "\n";
    
    std::vector<float> y1_v;
    arr_to_vec<float>(sizeof(y1_) / sizeof(*y1_), y1_, y1_v);
    std::cout << "y1_v= " << vec_to_str(y1_v) << "\n";
    
    std::vector<float> x2_v;
    arr_to_vec<float>(sizeof(x2_) / sizeof(*x2_), x2_, x2_v);
    std::cout << "x2_v= " << vec_to_str(x2_v) << "\n";
    
    std::vector<float> y2_v;
    arr_to_vec<float>(sizeof(y2_) / sizeof(*y2_), y2_, y2_v);
    std::cout << "y2_v= " << vec_to_str(y2_v) << "\n";
    
    std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/plot/pngs/fig_lan_exp_w_slack.png";
    
    make_plot(x1_v, y1_v, "w/o prefetching",
              x2_v, y2_v, "w/ prefetching",
              "Datasize of each data item (MB)", "Total Get Time (s)",
              "Get time w/ and w/o prefetching for 10 data items of same size", 
               out_url);
  }
  
  return 0;
}
