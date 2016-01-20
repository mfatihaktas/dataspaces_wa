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

#define RANDOM_INT_RANGE 100

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  // 
  std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/profiler/plot.png";
  
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
  
  // t_profiler.make_plot_(out_url);
  
  // for(int i = 0; i < 10; i++) {
  //   t_profiler.add_event(i, "event_" + boost::lexical_cast<std::string>(i) );
  //   sleep(1);
  // }
  
  // for(int i = 0; i < 10; i++) {
  //   t_profiler.end_event(i);
  // }
  
  // std::cout << "t_profiler= \n" << t_profiler.to_str();
  
  return 0;
}
