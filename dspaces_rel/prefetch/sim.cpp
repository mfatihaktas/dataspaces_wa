#include "sim.h"

/***********************************************  PCSim  ************************************************/
PCSim::PCSim(int num_ds, char* ds_id_, size_t pbuffer_size, size_t app_context_size,
             int num_p, int num_c,
             std::vector<char> p_id__ds_id_vec, std::vector<char> c_id__ds_id_vec,
             std::vector<int> p_id__num_put_vec, std::vector<int> c_id__num_get_vec,
             std::vector<float> p_id__put_rate_vec, std::vector<float> c_id__get_rate_vec,
             std::vector<std::vector<float> > p_id__inter_arr_time_vec_vec, std::vector<std::vector<float> > c_id__inter_arr_time_vec_vec )
: num_p(num_p), num_c(num_c),
  p_id__ds_id_vec(p_id__ds_id_vec), c_id__ds_id_vec(c_id__ds_id_vec),
  p_id__num_put_vec(p_id__num_put_vec), c_id__num_get_vec(c_id__num_get_vec),
  p_id__put_rate_vec(p_id__put_rate_vec), c_id__get_rate_vec(c_id__get_rate_vec),
  p_id__inter_arr_time_vec_vec(p_id__inter_arr_time_vec_vec), c_id__inter_arr_time_vec_vec(c_id__inter_arr_time_vec_vec),
  wa_space_(boost::make_shared<WASpace<key_ver_pair> >(num_ds, ds_id_, pbuffer_size, app_context_size) )
{
  for (int p_id = 0; p_id < num_p; p_id++)
    wa_space_->reg_app(p_id, p_id__ds_id_vec[p_id] );
  
  for (int c_id = 0; c_id < num_c; c_id++) {
    wa_space_->reg_app(num_p + c_id, c_id__ds_id_vec[c_id] );
    
    std::vector<char> get_type_vec;
    c_id__get_type_vec_map[c_id] = get_type_vec;
  }
  // 
  srand(time(NULL) );
  LOG(INFO) << "PCSim:: constructed= \n" << to_str();
}

PCSim::~PCSim() 
{ 
  wait_for_threads();
  // 
  LOG(INFO) << "PCSim:: destructed= \n" << to_str_end();
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
  
  return ss.str();
}

std::string PCSim::to_str_end()
{
  std::stringstream ss;
  
  ss << "c_id__get_type_vec_map= \n";
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_vec_map.begin(); map_it != c_id__get_type_vec_map.end(); map_it++) {
    ss << "c_id= " << boost::lexical_cast<std::string>(map_it->first) << ": ";
    for (std::vector<char>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      ss << boost::lexical_cast<std::string>(*vec_it) << ", ";
    }
    ss << "\n";
  }
  
  ss << "c_id__get_lperc_map= \n";
  for (std::map<int, float>::iterator it = c_id__get_lperc_map.begin(); it != c_id__get_lperc_map.end(); it++) {
    ss << "\t c_id= " << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
  }
  ss << "-----------------\n";
  ss << "wa_space= \n" << wa_space_->to_str() << "\n";
  
  return ss.str();
}

std::map<int, float> PCSim::get_c_id__get_lperc_map()
{
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_vec_map.begin(); map_it != c_id__get_type_vec_map.end(); map_it++) {
    int num_l = 0;
    for (std::vector<char>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      if (*vec_it == 'l')
        ++num_l;
    }
    
    c_id__get_lperc_map[map_it->first] = (float) num_l / (map_it->second).size();
  }
  
  return c_id__get_lperc_map;
}

void PCSim::wait_for_threads()
{
  for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = thread_vec.begin(); it != thread_vec.end(); it++)
    (*it)->join();
}

void PCSim::sim_all()
{
  // gen_rand_scenario();
  for (int i = 0; i < num_p; i++) {
    thread_vec.push_back( boost::make_shared<boost::thread>(&PCSim::sim_p, this, i) );
  }
  
  sleep(2);
  
  for (int i = 0; i < num_c; i++) {
    thread_vec.push_back( boost::make_shared<boost::thread>(&PCSim::sim_c, this, i) );
  }
}

void PCSim::sim_p(int p_id)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = p_id__inter_arr_time_vec_vec[p_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++) {
    LOG(INFO) << "sim_p:: p_id= " << p_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(c), 0);
    if (wa_space_->put(p_id, kv) ) {
      LOG(INFO) << "sim_p:: wa_space_->put failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    c++;
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
}

void PCSim::sim_c(int c_id)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = c_id__inter_arr_time_vec_vec[c_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++) {
    LOG(INFO) << "sim_c:: c_id= " << c_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(c_id) + "_" + boost::lexical_cast<std::string>(c), 0);
    char get_type;
    if (wa_space_->get(true, num_p + c_id, kv, get_type) ) {
      LOG(INFO) << "sim_c:: wa_space_->get failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    c_id__get_type_vec_map[c_id].push_back(get_type);
    
    c++;
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; put <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
}
