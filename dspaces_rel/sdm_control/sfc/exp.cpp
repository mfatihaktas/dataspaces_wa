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
  int volume = 1000;
  int dim_length = (int) pow(volume, (float)1/NDIM);
  std::cout << "sim:: dim_length= " << dim_length << "\n";
  
  std::vector<char> ds_id_v = boost::assign::list_of('a')('b');
  int pbuffer_size = 0;
  int pexpand_length = 1;
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  std::vector<char> p_id__ds_id_v = boost::assign::list_of('a');
  std::vector<char> c_id__ds_id_v = boost::assign::list_of('b');
  PCSim pc_sim(HILBERT_PREDICTOR, ds_id_v,
               pbuffer_size, pexpand_length,
               lcoor_, ucoor_,
               p_id__ds_id_v, c_id__ds_id_v);
  
  std::vector<app_id__lcoor_ucoor_pair> p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v;
  
  COOR_T put_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T put_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  p_id__lcoor_ucoor_pair_v.push_back(std::make_pair(0, std::make_pair(put_lcoor_, put_ucoor_) ) );
  // MULTI_FOR(put_lcoor_, put_ucoor_)
  //   COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
  //   COOR_T walk_ucoor_[NDIM];
  //   for (int i = 0; i < NDIM; i++)
  //     walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
  //   COOR_T* dwalk_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  //   COOR_T* dwalk_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  //   for (int i = 0; i < NDIM; i++) {
  //     dwalk_lcoor_[i] = walk_lcoor_[i];
  //     dwalk_ucoor_[i] = walk_ucoor_[i];
  //   }
    
  //   p_id__lcoor_ucoor_pair_v.push_back(std::make_pair(0, std::make_pair(dwalk_lcoor_, dwalk_ucoor_) ) );
  // END_MULTI_FOR()
  
  COOR_T get_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T get_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  MULTI_FOR(get_lcoor_, get_ucoor_)
    COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    
    COOR_T* dwalk_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    COOR_T* dwalk_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    for (int i = 0; i < NDIM; i++) {
      dwalk_lcoor_[i] = walk_lcoor_[i];
      dwalk_ucoor_[i] = walk_lcoor_[i] + 1;
    }
    
    c_id__lcoor_ucoor_pair_v.push_back(std::make_pair(1, std::make_pair(dwalk_lcoor_, dwalk_ucoor_) ) );
    // std::cout << "d0= " << d0 << ", d1= " << d1 << "\n";
    // std::cout << "put_test:: " << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
      // wa_space.put(0, walk_lcoor_, walk_ucoor_);
  END_MULTI_FOR()
  
  pc_sim.sim(p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v);
  std::cout << "sim:: pc_sim.get_c_id__get_lperc_map= \n" << patch_sfc::map_to_str<>(pc_sim.get_c_id__get_lperc_map() ) << "\n";
  
  std::cout << "sim:: pc_sim= \n" << pc_sim.to_str();
}

void test_hpredictor()
{
  char ds_id_[] = {'a', 'b', 'c'};
  srand(time(NULL) );
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 16) };
  HPredictor hpredictor(1, lcoor_, ucoor_);
  
  std::cout << "test_hpredictor:: index_interval_set= " << *(hpredictor.coor_to_index_interval_set_(lcoor_, ucoor_) ) << "\n";
  
  // MULTI_FOR(lcoor_, ucoor_)
  //   COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
  //   COOR_T walk_ucoor_[NDIM];
  //   for (int i = 0; i < NDIM; i++)
  //     walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
  //   std::cout << "test_hpredictor:: ----------- \n";
  //   std::cout << "walk_lucoor_: \n" << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
    
  //   std::vector<lcoor_ucoor_pair> next_lcoor_ucoor_pair_v;
  //   // hpredictor.add_acc__predict_next_acc(ds_id_[rand() % sizeof(ds_id_)/sizeof(*ds_id_) ], walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  //   hpredictor.add_acc__predict_next_acc('a', walk_lcoor_, walk_ucoor_, 
  //                                       1, next_lcoor_ucoor_pair_v);
  //   for (int i = 0; i < next_lcoor_ucoor_pair_v.size(); i++) {
  //     lcoor_ucoor_pair lu_pair = next_lcoor_ucoor_pair_v[i];
  //     std::cout << "next_lucoor_: \n" << LUCOOR_TO_STR(lu_pair.first, lu_pair.second) << "\n";
  //   }
  // END_MULTI_FOR()
  std::cout << "hpredictor= \n" << hpredictor.to_str();
}

void test_rtable()
{
  RTable<char> rtable;
  
  COOR_T lcoor_1_[] = {0, 0};
  COOR_T ucoor_1_[] = {5, 5};
  rtable.add("", 0, lcoor_1_, ucoor_1_, 'a');
  
  COOR_T lcoor_2_[] = {2, 2};
  COOR_T ucoor_2_[] = {7, 7};
  rtable.add("", 0, lcoor_2_, ucoor_2_, 'b');
  
  COOR_T lcoor_[] = {3, 3};
  COOR_T ucoor_[] = {5, 5};
  std::vector<char> ds_id_v;
  rtable.query("", 0, lcoor_, ucoor_, ds_id_v);
  std::cout << "test_rtable:: ds_id_v= " << patch_sfc::vec_to_str<>(ds_id_v) << "\n";
  
  std::cout << "test_rtable:: rtable= \n" << rtable.to_str() << "\n";
}

int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  // test_rtable();
  // check_boost_geometry_api();
  // check_hilbert_api();
  // test_hpredictor();
  // check_3d_hilbert_curve();
  sim();
  // COOR_T get_lcoor_[] = {2, 2};
  // COOR_T get_ucoor_[] = {2, 3};
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
