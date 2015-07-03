#include "hilbert.h"
#include "sim.h"
// 
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

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

void check_hilbert_curve()
{
  unsigned nDims = 2;
  unsigned nBits = 4;
  // bitmask_t coord_1_[] = {5, 2};
  // bitmask_t coord_2_[] = {5, 5};
  // std::cout << "index_1= " << hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // std::cout << "index_2= " << hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
  typedef std::pair<bitmask_t, bitmask_t> coor_p;
  std::vector<bitmask_t> index_v;
  std::map<bitmask_t, coor_p> index__coor_p_map;
  
  int up_limit = pow(2, nBits);
  for (int x = 0; x < up_limit; x++) {
    for (int y = 0; y < up_limit; y++) {
      bitmask_t coord_[] = {x, y};
      bitmask_t index = hilbert_c2i(nDims, nBits, coord_);
      
      index_v.push_back(index);
      index__coor_p_map[index] = std::make_pair(x, y);
    }
  }
  std::sort(index_v.begin(), index_v.end() );
  
  std::ofstream myfile("hilber_cpp.csv");
  if (myfile.is_open() ) {
    for (std::vector<bitmask_t>::iterator it = index_v.begin(); it != index_v.end(); it++) {
      coor_p p = index__coor_p_map[*it];
      myfile << p.first << "," << p.second << "\n";
      // std::cout << p.first << "," << p.second << "\n";
    }
    
    myfile.close();
  }
}

// Walk whole space with incremental boxes
void sim()
{
  #define VAR(d, n) BOOST_PP_CAT(d, n)
  #define VAR_REP(z, n, d) d ## n
  
  #define FOR_I(n) BOOST_PP_SUB(BOOST_PP_SUB(NDIM, n), 1)
  
  #define FOR_REP(z, n, ll_ul_) \
    for(int VAR(d, n) = BOOST_PP_TUPLE_ELEM(2, 0, ll_ul_)[n]; VAR(d, n) < BOOST_PP_TUPLE_ELEM(2, 1, ll_ul_)[n]; VAR(d, n)++) {
    // for(int VAR(d, FOR_I(n) ) = BOOST_PP_TUPLE_ELEM(2, 0, ll_ul_)[FOR_I(n) ]; \
    //     VAR(d, FOR_I(n) ) < BOOST_PP_TUPLE_ELEM(2, 1, ll_ul_)[FOR_I(n) ]; VAR(d, FOR_I(n) )++) {
  
  #define END_FOR_REP(z, n, data) }
  
  #define MULTI_FOR(ll_, ul_) \
    BOOST_PP_REPEAT(NDIM, FOR_REP, (ll_, ul_) )
  
  #define END_MULTI_FOR() \
    BOOST_PP_REPEAT(NDIM, END_FOR_REP, ~)
  // 
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
  
  sim();
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
