#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>

#include "plot.h"
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
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv, "s", long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c) {
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

#define RANDOM_INT_RANGE 100

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  // 
  if (str_cstr_equals(opt_map["type"], "profiler") ) {
    TProfiler<int> t_profiler;
    for (int i = 0; i < 10; i++) {
      t_profiler.add_event(i, "event_" + boost::lexical_cast<std::string>(i) );
      sleep(1);
    }
    
    for (int i = 0; i < 10; i++)
      t_profiler.end_event(i);
    
    log_(INFO, "t_profiler= \n" << t_profiler.to_str() )
  }
  else if (str_cstr_equals(opt_map["type"], "plot") ) {
    // float x1_[] = {0.125, 1, 8, 64, 512};
    // float y1_[] = {29.4, 29.1, 31.5, 37.4, 150.7};
    // float x2_[] = {0.125, 1, 8, 64, 512};
    // float y2_[] = {11.5, 11.7, 12.7, 20.2, 73.7};
    
    // std::vector<float> x1_v;
    // arr_to_vec<float>(sizeof(x1_) / sizeof(*x1_), x1_, x1_v);
    // std::cout << "x1_v= " << patch::vec_to_str<>(x1_v) << "\n";
    
    // std::vector<float> y1_v;
    // arr_to_vec<float>(sizeof(y1_) / sizeof(*y1_), y1_, y1_v);
    // std::cout << "y1_v= " << patch::vec_to_str<>(y1_v) << "\n";
    
    // std::vector<float> x2_v;
    // arr_to_vec<float>(sizeof(x2_) / sizeof(*x2_), x2_, x2_v);
    // std::cout << "x2_v= " << patch::vec_to_str<>(x2_v) << "\n";
    
    // std::vector<float> y2_v;
    // arr_to_vec<float>(sizeof(y2_) / sizeof(*y2_), y2_, y2_v);
    // std::cout << "y2_v= " << patch::vec_to_str<>(y2_v) << "\n";
    
    // std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/plot/pngs/fig_lan_exp_w_slack.png";
    
    // make_plot(x1_v, y1_v, "w/o prefetching",
    //           x2_v, y2_v, "w/ prefetching",
    //           "Datasize of each data item (MB)", "Total Get Time (s)",
    //           "Get time w/ and w/o prefetching for 10 data items of same size", 
    //           out_url);
    srand(time(NULL) );
    std::vector<float> x_v;
    for (int i = 0; i < 10000; i++) {
      float random = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX);
      x_v.push_back(roundf(random * 100) / 100.0);
      // x_v.push_back(rand() % 10);
    }
    
    std::string out_url = "";
    // out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_hist.png";
    patch::make_hist<float>(x_v, "x", "Frequency",
                            "Empirical distribution of x", out_url);
  }
  
  return 0;
}
