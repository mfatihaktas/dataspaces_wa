#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "hilbert.h"
#include "sim.h"
#include "patch_exp.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] = {
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
        opt_map["type"] = optarg;
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc) {
    std::cout << "parse_opts:: Non-option ARGV-elements= \n";
    while (optind < argc)
      std::cout << argv[optind++] << "\n";
  }
  // 
  std::cout << "parse_opts:: opt_map= \n";
  for (std::map<std::string, std::string>::iterator it = opt_map.begin(); it != opt_map.end(); ++it)
    std::cout << it->first << " : " << it->second << "\n";
  
  return opt_map;
}

// Walk whole space with incremental boxes
void sim()
{
  std::vector<char> ds_id_v = boost::assign::list_of('a')('b');
  int pbuffer_size = 0;
  int lcoor_[] = {0, 0};
  int ucoor_[] = {10, 10};
  std::vector<char> p_id__ds_id_v = boost::assign::list_of('a');
  std::vector<char> c_id__ds_id_v = boost::assign::list_of('b');
  PCSim pc_sim(ds_id_v, pbuffer_size,
               lcoor_, ucoor_,
               p_id__ds_id_v, c_id__ds_id_v);
  
  std::vector<app_id__lcoor_ucoor_pair> p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v;
  p_id__lcoor_ucoor_pair_v.push_back(std::make_pair(0, std::make_pair(lcoor_, ucoor_) ) );
  
  MULTI_FOR(lcoor_, ucoor_)
    int walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    int walk_ucoor_[NDIM];
    for (int i = 0; i < NDIM; i++)
      walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
    int* dwalk_lcoor_ = (int*)malloc(NDIM*sizeof(int) );
    int* dwalk_ucoor_ = (int*)malloc(NDIM*sizeof(int) );
    for (int i = 0; i < NDIM; i++) {
      dwalk_lcoor_[i] = walk_lcoor_[i];
      dwalk_ucoor_[i] = walk_ucoor_[i];
    }
    
    c_id__lcoor_ucoor_pair_v.push_back(std::make_pair(1, std::make_pair(dwalk_lcoor_, dwalk_ucoor_) ) );
    std::cout << "d0= " << d0 << ", d1= " << d1 << "\n";
    // std::cout << "put_test:: " << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
    // wa_space.put(0, walk_lcoor_, walk_ucoor_);
  END_MULTI_FOR()
  
  pc_sim.sim(p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v);
  std::cout << "sim:: pc_sim.get_c_id__get_lperc_map= \n" << patch_sfc::map_to_str<>(pc_sim.get_c_id__get_lperc_map() ) << "\n";

  // for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = p_id__lcoor_ucoor_pair_v.begin(); it != p_id__lcoor_ucoor_pair_v.end(); it++) {
  //   free((it->second).first);
  //   free((it->second).second);
  // }
  // for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = c_id__lcoor_ucoor_pair_v.begin(); it != c_id__lcoor_ucoor_pair_v.end(); it++) {
  //   free((it->second).first);
  //   free((it->second).second);
  // }
}



int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  
  
  // char ds_id_[] = {'a', 'b', 'c'};
  // srand (time(NULL) );
  // int lcoor_[] = {0, 0};
  // int ucoor_[] = {10, 10};
  // HPredictor hpredictor(lcoor_, ucoor_);
  // MULTI_FOR(lcoor_, ucoor_)
  //   int walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
  //   int walk_ucoor_[NDIM];
  //   for (int i = 0; i < NDIM; i++)
  //     walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
  //   std::vector<lcoor_ucoor_pair> next_lcoor_ucoor_pair_v;
  //   hpredictor.add_acc__predict_next_acc(ds_id_[rand() % sizeof(ds_id_)/sizeof(*ds_id_) ], walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  //   // hpredictor.add_acc__predict_next_acc(ds_id_[rand() % sizeof(ds_id_)/sizeof(*ds_id_) ], walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  //   hpredictor.add_acc__predict_next_acc('a', walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  // END_MULTI_FOR()
  
  // std::vector<lcoor_ucoor_pair> next_lcoor_ucoor_pair_v;
  // int walk_lcoor_[] = {0, 0};
  // int walk_ucoor_[] = {5, 5};
  // hpredictor.add_acc__predict_next_acc('a', walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  // int walk_2_lcoor_[] = {1, 1};
  // int walk_2_ucoor_[] = {7, 7};
  // hpredictor.add_acc__predict_next_acc('b', walk_2_lcoor_, walk_2_ucoor_, next_lcoor_ucoor_pair_v);
  
  // std::cout << "main:: hpredictor= \n" << hpredictor.to_str();
  
  // check_3d_hilbert_curve();
  // sim();
  // int get_lcoor_[] = {2, 2};
  // int get_ucoor_[] = {2, 3};
  // std::vector<char> get_v;
  // wa_space.get(get_lcoor_, get_ucoor_, get_v);
  // std::cout << "main:: get_v= " << patch_sfc::vec_to_str<char>(get_v) << "\n";
  
  // deneme();
  // if (opt_map["type"].compare("s") == 0) {
  //   SDMServer sdm_server("sdm_server", s_lip, s_lport, boost::bind(handle_char_recv, _1) );
  //   // 
  //   std::cout << "Enter \n";
  //   getline(std::cin, temp);
  // }
  // else {
  //   LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  // }
  
  return 0;
}
