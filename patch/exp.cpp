#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

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

void plot_results_fusion_ind_w_i()
{
  std::vector<std::vector<float> > x_v_v, y_v_v;
  std::vector<std::string> title_v;
  
  std::vector<float> chunk_size_v = boost::assign::list_of(12.5)(125)(250)(375)(500)(625)(750)(875)(1000);
  title_v.push_back("w/o prefetching");
  std::vector<float> wo_p__wo_slack__avg_get_time_v = boost::assign::list_of(7.6)(8)(13.31)(18.97)(27.2)(27.57)(33.22)(37.9)(42.44);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/ prefetching");
  std::vector<float> w_p__wo_slack__avg_get_time_v = boost::assign::list_of(7.11)(7.3)(11.38)(16.64)(25)(27.44)(32.87)(36.94)(38.12);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/o prefetching, +10s slack");
  std::vector<float> wo_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(1.55)(2.45)(5.87)(7.6)(16.65)(23)(24.68)(33.84)(34.74);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +10s slack");
  std::vector<float> w_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(0.55)(1.69)(5.02)(6.81)(13.24)(17.51)(18.8)(30.91)(32.3);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +20s slack");
  std::vector<float> wo_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(1.44)(2.36)(4.6)(5.58)(8.04)(14)(16.2)(25.25)(26.4);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +20s slack");
  std::vector<float> w_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(0.62)(1.18)(2.33)(3.17)(3.79)(6.74)(9.57)(10.67)(17.35);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +30s slack");
  std::vector<float> wo_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(1.25)(2.15)(4.16)(5.5)(7.16)(9.23)(10.24)(14.42)(17.58);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_30s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +30s slack");
  std::vector<float> w_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(0.11)(0.66)(1.27)(1.88)(2.46)(3.12)(3.68)(4.4)(5.1);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_30s__avg_get_time_v);
  
  std::string out_url = "";
  // patch::make_plot<float>(x_v_v, y_v_v, title_v,
  //                         "Chunk size (MB)", "Average chunk get time (s)",
  //                         "Fusion_i with RDMA", out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_results_fusion_ind_w_i.png";
  patch::make_plot<float>(x_v_v, y_v_v, title_v,
                          "Chunk size (MB)", "Average chunk get time (s)",
                          "Fusion_i with RDMA", out_url);
}

void plot_results_fusion_ind_w_t()
{
  std::vector<std::vector<float> > x_v_v, y_v_v;
  std::vector<std::string> title_v;
  
  std::vector<float> chunk_size_v = boost::assign::list_of(12.5)(125)(250)(375)(500)(625)(750)(875)(1000);
  title_v.push_back("w/o prefetching");
  std::vector<float> wo_p__wo_slack__avg_get_time_v = boost::assign::list_of(7.61)(8.69)(25.94)(29.75)(40.02)(49.8)(58.73)(67.73)(80.58);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/ prefetching");
  std::vector<float> w_p__wo_slack__avg_get_time_v = boost::assign::list_of(6.64)(8.46)(17.05)(24.96)(28.48)(38.22)(49.4)(52)(65.11);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/o prefetching, +10s slack");
  std::vector<float> wo_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(1.28)(3.08)(7.35)(19.64)(24.91)(39.8)(49.01)(52.48)(68.99);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +10s slack");
  std::vector<float> w_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(0.11)(0.72)(2.3)(13.35)(22.89)(31.72)(40.79)(43.12)(44.94);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +20s slack");
  std::vector<float> wo_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(1.28)(3.07)(6.1)(11.56)(17.19)(25.69)(32.18)(39.80)(49.67);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +20s slack");
  std::vector<float> w_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(0.17)(0.73)(1.39)(2.14)(7.71)(19.83)(24.02)(25.26)(25.99);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +30s slack");
  std::vector<float> wo_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(1.27)(3.09)(6.12)(9.71)(18.54)(20.02)(22.77)(38)(43.5);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_30s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +30s slack");
  std::vector<float> w_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(0.11)(0.73)(1.39)(2.05)(4.4)(6.77)(12.44)(18.14)(26.95);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_30s__avg_get_time_v);
  
  std::string out_url = "";
  // patch::make_plot<float>(x_v_v, y_v_v, title_v,
  //                         "Chunk size (MB)", "Average chunk get time (s)",
  //                         "Fusion_i with TCP", out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_results_fusion_ind_w_t.png";
  patch::make_plot<float>(x_v_v, y_v_v, title_v,
                          "Chunk size (MB)", "Average chunk get time (s)",
                          "Fusion_i with TCP", out_url);
}

void plot_results_fusion_sync_w_i()
{
  std::vector<std::vector<float> > x_v_v, y_v_v;
  std::vector<std::string> title_v;
  
  std::vector<float> chunk_size_v = boost::assign::list_of(12.5)(125)(250)(375)(500)(625)(750)(875)(1000);
  title_v.push_back("w/o prefetching");
  std::vector<float> wo_p__wo_slack__avg_get_time_v = boost::assign::list_of(8.9)(8.17)(17.24)(17.32)(25.31)(34.16)(34.51)(42.47)(44.52);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/ prefetching");
  std::vector<float> w_p__wo_slack__avg_get_time_v = boost::assign::list_of(6.0)(6.34)(14.78)(14.90)(18.66)(25.88)(26.29)(32.04)(34.15);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/o prefetching, +10s slack");
  std::vector<float> wo_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(7.22)(8.20)(16.24)(18.11)(25.27)(33.31)(34.46)(42.57)(43.95);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +10s slack");
  std::vector<float> w_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(0.13)(0.80)(2.12)(5.41)(6.95)(15.59)(19.85)(23.83)(24.07);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +20s slack");
  std::vector<float> wo_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(7.22)(8.11)(16.25)(17.01)(25.32)(33.13)(34.62)(42.37)(44.40);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +20s slack");
  std::vector<float> w_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(0.13)(1.22)(1.81)(2.66)(3.29)(6.59)(9.08)(15.35)(16.2);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +30s slack");
  std::vector<float> wo_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(7.23)(8.17)(16.22)(18.25)(25.21)(33.10)(34.37)(42.43)(43.84);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_30s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +30s slack");
  std::vector<float> w_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(0.13)(0.89)(1.60)(2.39)(3.50)(4.83)(4.55)(5.89)(6.85);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_30s__avg_get_time_v);
  
  std::string out_url = "";
  // patch::make_plot<float>(x_v_v, y_v_v, title_v,
  //                         "Chunk size (MB)", "Average chunk get time (s)",
  //                         "Fusion_s with RDMA", out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_results_fusion_sync_w_i.png";
  patch::make_plot<float>(x_v_v, y_v_v, title_v,
                          "Chunk size (MB)", "Average chunk get time (s)",
                          "Fusion_s with RDMA", out_url);
}

void plot_results_fusion_sync_w_t()
{
  std::vector<std::vector<float> > x_v_v, y_v_v;
  std::vector<std::string> title_v;
  
  std::vector<float> chunk_size_v = boost::assign::list_of(12.5)(125)(250)(375)(500)(625)(750)(875)(1000);
  title_v.push_back("w/o prefetching");
  std::vector<float> wo_p__wo_slack__avg_get_time_v = boost::assign::list_of(7.90)(10.42)(21.00)(33.22)(44.22)(54.36)(66.34)(76.80)(87.00);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/ prefetching");
  std::vector<float> w_p__wo_slack__avg_get_time_v = boost::assign::list_of(7.22)(8.17)(12.69)(21.38)(32.28)(38.17)(45.64)(50.34)(61.54);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__wo_slack__avg_get_time_v);
  title_v.push_back("w/o prefetching, +10s slack");
  std::vector<float> wo_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(7.34)(10.53)(20.92)(33.32)(44.52)(55.02)(66.86)(76.78)(88.02);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +10s slack");
  std::vector<float> w_p__w_slack_10s__avg_get_time_v = boost::assign::list_of(0.13)(0.86)(7.56)(15.72)(31.64)(22.75)(33.38)(42.74)(51.57);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_10s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +20s slack");
  std::vector<float> wo_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(7.26)(10.5)(21.05)(44.34)(44.34)(51.97)(65.91)(75.12)(87.45);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +20s slack");
  std::vector<float> w_p__w_slack_20s__avg_get_time_v = boost::assign::list_of(0.132)(0.78)(1.87)(2.75)(15.05)(22.98)(23.96)(29.73)(44.69);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_20s__avg_get_time_v);
  title_v.push_back("w/o prefetching, +30s slack");
  std::vector<float> wo_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(7.26)(10.34)(20.43)(30.53)(44.26)(52.00)(65.82)(76.76)(77.16);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(wo_p__w_slack_30s__avg_get_time_v);
  title_v.push_back("w/ prefetching, +30s slack");
  std::vector<float> w_p__w_slack_30s__avg_get_time_v = boost::assign::list_of(0.1266)(0.83)(1.73)(2.52)(6.02)(14.71)(21.64)(27.52)(37.91);
  x_v_v.push_back(chunk_size_v);
  y_v_v.push_back(w_p__w_slack_30s__avg_get_time_v);
  
  std::string out_url = "";
  // patch::make_plot<float>(x_v_v, y_v_v, title_v,
  //                         "Chunk size (MB)", "Average chunk get time (s)",
  //                         "Fusion_s with TCP", out_url);
  
  out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_results_fusion_sync_w_t.png";
  patch::make_plot<float>(x_v_v, y_v_v, title_v,
                          "Chunk size (MB)", "Average chunk get time (s)",
                          "Fusion_s with TCP", out_url);
}

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
    plot_results_fusion_ind_w_i();
    plot_results_fusion_ind_w_t();
    plot_results_fusion_sync_w_i();
    plot_results_fusion_sync_w_t();
    
    // srand(time(NULL) );
    // std::vector<float> x_v;
    // for (int i = 0; i < 10000; i++) {
    //   float random = static_cast<float>(rand() ) / static_cast<float>(RAND_MAX);
    //   x_v.push_back(roundf(random * 100) / 100.0);
    //   // x_v.push_back(rand() % 10);
    // }
    
    // std::string out_url = "";
    // // out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/patch/fig_hist.png";
    // patch::make_hist<float>(x_v, "x", "Frequency",
    //                         "Empirical distribution of x", out_url);
  }
  
  return 0;
}
