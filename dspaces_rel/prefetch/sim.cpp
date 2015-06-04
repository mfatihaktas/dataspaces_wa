#include "sim.h"

/***********************************************  PCSim  ************************************************/
PCSim::PCSim(std::vector<char> ds_id_v, int pbuffer_size, PREFETCH_T prefetch_t,
             int num_p, int num_c,
             std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
             std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
             std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
             std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v )
: num_p(num_p), num_c(num_c),
  p_id__ds_id_v(p_id__ds_id_v), c_id__ds_id_v(c_id__ds_id_v),
  p_id__num_put_v(p_id__num_put_v), c_id__num_get_v(c_id__num_get_v),
  p_id__put_rate_v(p_id__put_rate_v), c_id__get_rate_v(c_id__get_rate_v),
  p_id__inter_arr_time_v_v(p_id__inter_arr_time_v_v), c_id__inter_arr_time_v_v(c_id__inter_arr_time_v_v),
  wa_space_(boost::make_shared<WASpace<key_ver_pair> >(ds_id_v, pbuffer_size, prefetch_t) )
{
  for (int p_id = 0; p_id < num_p; p_id++)
    wa_space_->reg_app(p_id, p_id__ds_id_v[p_id] );
  
  for (int c_id = 0; c_id < num_c; c_id++) {
    wa_space_->reg_app(num_p + c_id, c_id__ds_id_v[c_id] );
    
    std::vector<char> get_type_v;
    c_id__get_type_v_map[c_id] = get_type_v;
  }
  // 
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
  ss << "p_id__ds_id_v= " << patch_pre::vec_to_str<char>(p_id__ds_id_v) << "\n";
  ss << "p_id__num_put_v= " << patch_pre::vec_to_str<int>(p_id__num_put_v) << "\n";
  ss << "p_id__put_rate_v= " << patch_pre::vec_to_str<float>(p_id__put_rate_v) << "\n";
  
  ss << "num_c= " << boost::lexical_cast<std::string>(num_c) << "\n";
  ss << "c_id__ds_id_v= " << patch_pre::vec_to_str<char>(c_id__ds_id_v) << "\n";
  ss << "c_id__num_get_v= " << patch_pre::vec_to_str<int>(c_id__num_get_v) << "\n";
  ss << "c_id__get_rate_v= " << patch_pre::vec_to_str<float>(c_id__get_rate_v) << "\n";
  
  return ss.str();
}

std::string PCSim::to_str_end()
{
  std::stringstream ss;
  
  ss << "c_id__get_type_v_map= \n";
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_v_map.begin(); map_it != c_id__get_type_v_map.end(); map_it++) {
    ss << "c_id= " << boost::lexical_cast<std::string>(map_it->first) << ": ";
    for (std::vector<char>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      ss << boost::lexical_cast<std::string>(*vec_it) << ", ";
    }
    ss << "\n";
  }
  
  // ss << "c_id__get_lperc_map= \n";
  // for (std::map<int, float>::iterator it = c_id__get_lperc_map.begin(); it != c_id__get_lperc_map.end(); it++) {
  //   ss << "\t c_id= " << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
  // }
  ss << "-----------------\n";
  ss << "wa_space= \n" << wa_space_->to_str() << "\n";
  
  return ss.str();
}

std::map<int, float> PCSim::get_c_id__get_lperc_map()
{
  for (std::map<int, std::vector<char> >::iterator map_it = c_id__get_type_v_map.begin(); map_it != c_id__get_type_v_map.end(); map_it++) {
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
  for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = thread_v.begin(); it != thread_v.end(); it++)
    (*it)->join();
}

void PCSim::sim_all()
{
  for (int i = 0; i < num_p; i++)
    thread_v.push_back(boost::make_shared<boost::thread>(&PCSim::sim_p, this, i) );
  
  sleep(2);
  
  for (int i = 0; i < num_c; i++)
    thread_v.push_back(boost::make_shared<boost::thread>(&PCSim::sim_c, this, i) );
}

void PCSim::sim_p(int p_id)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = p_id__inter_arr_time_v_v[p_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++, c++) {
    // LOG(INFO) << "sim_p:: p_id= " << p_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(c), 0);
    if (wa_space_->put(p_id, kv) )
      LOG(ERROR) << "sim_p:: wa_space_->put failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
}

void PCSim::sim_c(int c_id)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = c_id__inter_arr_time_v_v[c_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++, c++) {
    // LOG(INFO) << "sim_c:: c_id= " << c_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    key_ver_pair kv = std::make_pair("d_" + boost::lexical_cast<std::string>(c_id) + "_" + boost::lexical_cast<std::string>(c), 0);
    char get_type;
    if (wa_space_->get(true, num_p + c_id, kv, get_type) )
      LOG(ERROR) << "sim_c:: wa_space_->get failed! <key= " << kv.first << ", ver= " << kv.second << ">.";
    
    c_id__get_type_v_map[c_id].push_back(get_type);
    
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; GET <key= " << kv.first << ", ver= " << kv.second << ">.";
  }
}
