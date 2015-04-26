#include "sim.h"

PCSim::PCSim(int num_ds, char* ds_id_,
             int num_p, std::vector<char> p_id__ds_id_vec, std::vector<int> p_id__num_put_vec, std::vector<float> p_id__put_rate_vec, 
             int num_c, std::vector<char> c_id__ds_id_vec, std::vector<int> c_id__num_get_vec, std::vector<float> c_id__get_rate_vec )
: num_p(num_p), num_c(num_c),
  p_id__ds_id_vec(p_id__ds_id_vec), c_id__ds_id_vec(c_id__ds_id_vec),
  p_id__num_put_vec(p_id__num_put_vec), c_id__num_get_vec(c_id__num_get_vec),
  p_id__put_rate_vec(p_id__put_rate_vec), c_id__get_rate_vec(c_id__get_rate_vec),
  wa_space_(boost::make_shared<WASpace<key_ver_pair> >(num_ds, ds_id_) )
{
  for (int p_id = 0; p_id < num_p; p_id++)
    wa_space_->reg_app(p_id, p_id__ds_id_vec[p_id] );
  
  for (int c_id = 0; c_id < num_c; c_id++)
    wa_space_->reg_app(num_p + c_id, c_id__ds_id_vec[c_id] );
  // 
  srand(time(NULL) );
  // LOG(INFO) << "PCSim:: constructed= \n" << to_str();
  LOG(INFO) << "PCSim:: constructed.";
}

PCSim::~PCSim() 
{ 
  for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = thread_vec.begin(); it != thread_vec.end(); it++) {
    (*it)->join();
  }
  // 
  LOG(INFO) << "PCSim:: destructed; \n" << to_str();
};

std::string PCSim::to_str()
{
  std::stringstream ss;
  ss << "num_p= " << boost::lexical_cast<std::string>(num_p) << "\n";
  ss << "p_id__ds_id_vec= " << patch_pre::vector_to_str<char>(p_id__ds_id_vec) << "\n";
  ss << "p_id__num_put_vec= " << patch_pre::vector_to_str<int>(p_id__num_put_vec) << "\n";
  ss << "p_id__put_rate_vec= " << patch_pre::vector_to_str<float>(p_id__put_rate_vec) << "\n";
  
  ss << "num_c= " << boost::lexical_cast<std::string>(num_c) << "\n";
  ss << "c_id__ds_id_vec= " << patch_pre::vector_to_str<char>(c_id__ds_id_vec) << "\n";
  ss << "c_id__num_get_vec= " << patch_pre::vector_to_str<int>(c_id__num_get_vec) << "\n";
  ss << "c_id__get_rate_vec= " << patch_pre::vector_to_str<float>(c_id__get_rate_vec) << "\n";
  
  ss << "c_id__get_type_vec_map= \n";
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_vec_map.begin(); map_it != c_id__get_type_vec_map.end(); map_it++) {
    ss << "c_id= " << boost::lexical_cast<std::string>(map_it->first) << ": ";
    for (std::vector<char>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      ss << boost::lexical_cast<std::string>(*vec_it) << ", ";
    }
    ss << "\n";
  }
  
  return ss.str();
}

int PCSim::sim_all()
{
  for (int i = 0; i < num_p; i++) {
    thread_vec.push_back( boost::make_shared<boost::thread>(&PCSim::sim_p, this, i) );
  }
  
  for (int i = 0; i < num_c; i++) {
    thread_vec.push_back( boost::make_shared<boost::thread>(&PCSim::sim_c, this, i) );
  }
  
  return 0;
}

int PCSim::sim_p(int p_id)
{
  for (int i = 0; i < p_id__num_put_vec[p_id]; i++) {
    // boost::math::exponential_distribution exp_dist(p_id__put_rate_vec[p_id] );
    // float sleep_time = -logf(1.0f - (float) random() / (RAND_MAX + 1) ) / p_id__put_rate_vec[p_id];
    float sleep_time = -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / p_id__put_rate_vec[p_id];
    LOG(INFO) << "sim_p:: p_id= " << p_id << ", sleep_time= " << sleep_time << "\n";
    sleep(sleep_time);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(i), 0);
    if (wa_space_->put(p_id, kv) ) {
      LOG(INFO) << "sim_p:: wa_space_->put failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
  
  return 0;
}

int PCSim::sim_c(int c_id)
{
  for (int i = 0; i < c_id__num_get_vec[c_id]; i++) {
    // boost::math::exponential_distribution exp_dist(p_id__put_rate_vec[p_id] );
    // float sleep_time = -logf(1.0f - (float) random() / (RAND_MAX + 1) ) / c_id__get_rate_vec[c_id];
    
    float sleep_time = -1 * log(1.0 - (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) ) ) / c_id__get_rate_vec[c_id];
    LOG(INFO) << "sim_c:: c_id= " << c_id << ", sleep_time= " << sleep_time << "\n";
    sleep(sleep_time);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(c_id) + "_" + boost::lexical_cast<std::string>(i), 0);
    char get_type;
    if (wa_space_->get(true, num_p + c_id, kv, get_type) ) {
      LOG(INFO) << "sim_c:: wa_space_->get failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    c_id__get_type_vec_map[c_id].push_back(get_type);
    
    LOG(INFO) << "sim_c:: c_id= " << c_id << ", got <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
  
  return 0;
}