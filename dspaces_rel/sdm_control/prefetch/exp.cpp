#include "prefetch.h"
#include "sim.h"
#include "patch_markov_exp.h"
#include "patch_sfc_exp.h"

#include <getopt.h>

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] = {
    {"type", optional_argument, NULL, 0},
    {0, 0, 0, 0}
  };
  
  while (1) {
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
  std::cout << "parse_opts:: opt_map= \n" << patch::map_to_str<>(opt_map);
  
  return opt_map;
}

spatial_syncer s_syncer;
void notify_s_syncer(int sleep_time, box_t b)
{
  std::cout << "notify_s_syncer:: waiting for " << sleep_time << " secs.\n";
  sleep(sleep_time);
  
  std::cout << "notify_s_syncer:: notifying with box= " << boost::geometry::dsv(b) << "\n";
  s_syncer.notify(b);
}

int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  srand(time(NULL) );
  
  // RTable<int> rtable;
  
  // COOR_T lc_0_[] = {0, 0};
  // COOR_T uc_0_[] = {1, 1};
  // rtable.add("dummy", 0, lc_0_, uc_0_, 'a');
  
  // COOR_T lc_1_[] = {0, 0};
  // COOR_T uc_1_[] = {2, 2};
  // rtable.add("dummy", 0, lc_1_, uc_1_, 'a');
  
  // std::cout << "NDIM= " << NDIM << "\n"
  //           << "rtable= \n" << rtable.to_str() << "\n"
  //           << "rtable.get_bounds= " << boost::geometry::dsv(rtable.get_bounds() ) << "\n";
  // // 
  // CREATE_BOX(0, b, lc_0_, uc_0_)
  // CREATE_BOX(1, b, lc_1_, uc_1_)
  // std::cout << "b0= " << boost::geometry::dsv(b0) << "\n"
  //           << "b1= " << boost::geometry::dsv(b1) << "\n"
  //           << "b0 < b1= " << boost::geometry::within(b0, b1) << "\n";
  
  // // std::map<box_t, int, less_for_box> box_int_map;
  // patch::thread_safe_map<box_t, int, less_for_box> box_int_map;
  // box_int_map[b0] = 1;
  // box_int_map[b1] = 2; 
  
  // std::cout << "box_int_map[b0]= " << box_int_map[b0] << "\n";
  
  // std::cout << "box_int_map= \n"; 
  // for (std::map<box_t, int>::iterator it = box_int_map.begin(); it != box_int_map.end(); it++)
  //   std::cout << boost::geometry::dsv(it->first) << " : " << it->second << "\n";
  
  // boost::thread t(notify_s_syncer, 4, b1);
  
  // s_syncer.add_sync_point(b0, 1);
  // std::cout << "s_syncer will wait on box= " << boost::geometry::dsv(b0) << "\n";
  // s_syncer.wait(b0);
  // s_syncer.del_sync_point(b0);
  
  if (str_cstr_equals(opt_map["type"], "markov") ) {
    test_rand_shuffle_train(); 
    // test_fixed_train();
    // test_malgo();
    // test_bmmalgo();
    // m_prefetch_test();
    // mpcsim_test();
    
    // std::vector<int> ds_id_v = boost::assign::list_of(0)(1);
    // MWASpace mwa_space(ds_id_v, MALGO_W_PPM, 10, false, 0);
    // COOR_T lc_[] = {0, 0};
    // COOR_T uc_[] = {1, 1};
    
    // mwa_space.reg_app(0, 'a');
    // mwa_space.reg_app(1, 'b');
    
    // mwa_space.put(0, "d_0", 0, lc_, uc_);
    // std::vector<int> query_ds_id_v;
    // if (mwa_space.query("d_0", 0, lc_, uc_, query_ds_id_v) )
    //   log_(INFO, "mwa_space.query failed; " << KV_TO_STR("d_0", 0) )
    
    // key_ver_pair kv = std::make_pair("d_0", 0);
    // log_(INFO, "query_ds_id_v= " << patch::vec_to_str<>(query_ds_id_v) << "\n"
    //           << "mwa_space.contains('a', kv)= " << mwa_space.contains('a', kv) << "\n"
    //           << "mwa_space= \n" << mwa_space.to_str() )
    
    // validate_random_shuffle();
    
    // std::vector<int> v;
    // for (int i = 1; i < 10; i++)
    //   v.push_back(i);
  
    // std::random_shuffle(v.begin(), v.end() );
    // std::cout << "shuffled v= " << patch::vec_to_str<int>(v) << "\n";
    
    // float lambda = 0.1;
    // int num_exp = 1000;
    // float sum = 0;
    // srand(time(NULL) );
    // for (int i = 0; i < num_exp; i++) {
    //   // std::cout << "main:: random number= " << static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) << "\n";
    //   sum += -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / lambda;
    // }
    // std::cout << "main:: sample mean= " << sum/num_exp << "\n";
    
    // boost::math::normal_distribution<float> normal_dist(0, 1);
    // std::cout << "normal_dist.mean= " << normal_dist.mean() << "\n"
    //           << "normal_dist.standard_deviation= " << normal_dist.standard_deviation() << "\n";
    // std::cout << "normal_dist cdf= \n";
    // for (float x = -4; x < 4 + 0.5; x += 0.5)
    //   std::cout << "x= " << x << ", cdf(x)= " << cdf(normal_dist, x) << "\n";
  }
  else if (str_cstr_equals(opt_map["type"], "sfc") ) {
    // test_rtable();
    // check_boost_geometry_api();
    // check_hilbert_api();
    // test_hpredictor();
    // check_3d_hilbert_curve();
    
    // COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
    // COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 10) };
    // HSAlgo hsalgo(lcoor_, ucoor_, 1);
    
    // COOR_T exp_lcoor_[] = {4, 4};
    // COOR_T exp_ucoor_[] = {6, 5};
    // boost::shared_ptr<index_interval_set_t> ii_set_ = hsalgo.coor_to_index_interval_set_(exp_lcoor_, exp_ucoor_);
    // std::cout << "ii_set= " << *ii_set_ << "\n";
    
    // hsalgo.expand_interval_set(1, *ii_set_);
    
    // RTable<int> rtable;
    
    // std::vector<COOR_T*> lcoor_v;
    // hsalgo.index_interval_set_to_coor_v(*ii_set_, lcoor_v);
    // for (std::vector<COOR_T*>::iterator lcoor__ = lcoor_v.begin(); lcoor__ != lcoor_v.end(); lcoor__++) {
    //   COOR_T* ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    //   for (int i = 0; i < NDIM; i++)
    //     ucoor_[i] = (*lcoor__)[i] + 1;
  
    //   // lucoor_to_fetch_v.push_back(std::make_pair(*lcoor__, ucoor_) );
    //   rtable.add("", 0, *lcoor__, ucoor_, -1);
    // }
    
    // std::cout << "rtable= " << rtable.to_str() << "\n";
    // COOR_T* to_fetch_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    // COOR_T* to_fetch_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    // rtable.get_bound_lucoor(to_fetch_lcoor_, to_fetch_ucoor_);
    // std::cout << "to_fetch_lucoor_= " << LUCOOR_TO_STR(to_fetch_lcoor_, to_fetch_ucoor_) << "\n";
    // patch::free_all<COOR_T>(2, to_fetch_lcoor_, to_fetch_ucoor_);
    
    // test_locality_prefetching();
    // std::vector<std::vector<float> > ndim_v_v;
    // ndim_v_v.push_back(boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(8)(9)(10) );
    // // std::vector<float> ndim_v = boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(8)(9)(10);
    // std::vector<std::vector<float> > hit_rate_v_v;
    // hit_rate_v_v.push_back(boost::assign::list_of(0.99)(0.8748)(0.831021)(0.8021)(0.78947)(0.785147)(0.784252)(0.783218)(0.756847)(0.753909) );
    // // std::vector<float> hit_rate_v = boost::assign::list_of(0.99)(0.8748)(0.831021)(0.8021)(0.78947)(0.785147)(0.713115)(0.783218)(0.756847)(0.753909);
    // std::vector<std::string> title_v = boost::assign::list_of("");
    
    // // std::string out_url = "";
    // std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/fig_hit_rate_w_varying_ndim.png";
    // make_plot<float>(ndim_v_v, hit_rate_v_v, title_v,
    //                 "Number of dimensions", "Hit rate",
    //                 "Hit rate for deterministic access walk with varying dimensionality", out_url);
    
    
    // spcsim_test();
    
    // COOR_T get_lcoor_[] = {2, 2};
    // COOR_T get_ucoor_[] = {2, 3};
    // std::vector<char> get_v;
    // wa_space.get(get_lcoor_, get_ucoor_, get_v);
    // std::cout << "main:: get_v= " << patch_sfc::vec_to_str<char>(get_v) << "\n";
  }
  else
    log_(ERROR, "unknown type= " << opt_map["type"] )
  
  return 0;
}
