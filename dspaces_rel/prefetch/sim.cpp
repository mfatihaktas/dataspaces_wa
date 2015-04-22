#include "sim.h"

PCSim::PCSim(int num_p, std::vector<int> num_put_vec, std::vector<float> put_rate_vec, 
             int num_c, std::vector<int> num_get_vec, std::vector<float> get_rate_vec )
: num_p(num_p), num_c(num_c),
  num_put_vec(num_put_vec), num_get_vec(num_get_vec),
  put_rate_vec(put_rate_vec), get_rate_vec(get_rate_vec)
{
  srand(time(NULL) );
  
  LOG(INFO) << "PCSim:: constructed= \n" << to_str() << "\n";
}

PCSim::~PCSim() { LOG(INFO) << "PCSim:: destructed.\n"; };

std::string PCSim::to_str()
{
  std::stringstream ss;
  ss << "num_p= " << boost::lexical_cast<std::string>(num_p) << "\n";
  ss << "num_put_vec= " << patch_pre::vector_to_str<int>(num_put_vec) << "\n";
  ss << "put_rate_vec= " << patch_pre::vector_to_str<float>(put_rate_vec) << "\n";
  
  ss << "num_c= " << boost::lexical_cast<std::string>(num_c) << "\n";
  ss << "num_get_vec= " << patch_pre::vector_to_str<int>(num_get_vec) << "\n";
  ss << "get_rate_vec= " << patch_pre::vector_to_str<float>(get_rate_vec) << "\n";
  
  return ss.str();
}

int PCSim::sim_all()
{
  for (int i = 0; i < num_p; i++) {
    boost::thread t(&PCSim::sim_p, this, i);
  }
  
  for (int i = 0; i < num_c; i++) {
    boost::thread t(&PCSim::sim_c, this, i);
  }
  
  return 0;
}

int PCSim::sim_p(int p_id)
{
  for (int i = 0; i < num_put_vec[p_id]; i++) {
    // boost::math::exponential_distribution exp_dist(put_rate_vec[p_id] );
    // float sleep_time = -logf(1.0f - (float) random() / (RAND_MAX + 1) ) / put_rate_vec[p_id];
    float sleep_time = -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / put_rate_vec[p_id];
    LOG(INFO) << "sim_p:: p_id= " << p_id << ", sleep_time= " << sleep_time << "\n";
    sleep(sleep_time);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(i), 0);
    if (space.put(kv) ) {
      LOG(INFO) << "sim_p:: space.put failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
  
  return 0;
}

int PCSim::sim_c(int c_id)
{
  for (int i = 0; i < num_get_vec[c_id]; i++) {
    // boost::math::exponential_distribution exp_dist(put_rate_vec[p_id] );
    // float sleep_time = -logf(1.0f - (float) random() / (RAND_MAX + 1) ) / get_rate_vec[c_id];
    ;
    
    float sleep_time = -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / get_rate_vec[c_id];
    LOG(INFO) << "sim_c:: c_id= " << c_id << ", sleep_time= " << sleep_time << "\n";
    sleep(sleep_time);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(c_id) + "_" + boost::lexical_cast<std::string>(i), 0);
    if (space.get(true, kv) ) {
      LOG(INFO) << "sim_c:: space.get failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; get <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
  
  return 0;
}