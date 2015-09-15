#include "prefetch.h"
#include "sim.h"

#include "patch_m_exp.h"
#include "patch_s_exp.h"

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
  std::cout << "parse_opts:: opt_map= \n" << patch_all::map_to_str<>(opt_map);
  
  return opt_map;
}

int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  srand(time(NULL) );
  
  if (str_cstr_equals(opt_map["type"], "markov") ) {
    // test_rand_shuffle_train();
    // plot_malgo_comparison();
    // malgo_test();
    // m_prefetch_test();
    m_sim_test();
    
    // validate_random_shuffle();
    
    // std::vector<int> v;
    // for (int i = 1; i < 10; i++)
    //   v.push_back(i);
  
    // std::random_shuffle(v.begin(), v.end() );
    // std::cout << "shuffled v= " << patch_all::vec_to_str<int>(v) << "\n";
    
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
  else if (str_cstr_equals(opt_map["type"], "hilbert") ) {
    // test_rtable();
    // check_boost_geometry_api();
    // check_hilbert_api();
    // test_hpredictor();
    // check_3d_hilbert_curve();
    s_sim_test();
    // COOR_T get_lcoor_[] = {2, 2};
    // COOR_T get_ucoor_[] = {2, 3};
    // std::vector<char> get_v;
    // wa_space.get(get_lcoor_, get_ucoor_, get_v);
    // std::cout << "main:: get_v= " << patch_sfc::vec_to_str<char>(get_v) << "\n";
  }
  else
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  
  return 0;
}
