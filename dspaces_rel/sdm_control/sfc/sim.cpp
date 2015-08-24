#include "sim.h"

/*******************************************  PCSim  **********************************************/
PCSim::PCSim(char predictor_t, std::vector<char> ds_id_v,
             int pbuffer_size, int pexpand_length,
             COOR_T* lcoor_, COOR_T* ucoor_,
             std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v)
: p_id__ds_id_v(p_id__ds_id_v), c_id__ds_id_v(c_id__ds_id_v),
  wa_space(predictor_t, ds_id_v, pbuffer_size, pexpand_length, lcoor_, ucoor_)
{
  for (int p_id = 0; p_id < p_id__ds_id_v.size(); p_id++)
    wa_space.reg_app(p_id, p_id__ds_id_v[p_id] );
  
  for (int c_id = 0; c_id < c_id__ds_id_v.size(); c_id++)
    wa_space.reg_app(p_id__ds_id_v.size() + c_id, c_id__ds_id_v[c_id] );
  // 
  LOG(INFO) << "PCSim:: constructed.";
}

PCSim::~PCSim() { LOG(INFO) << "PCSim:: destructed."; }

std::string PCSim::to_str()
{
  std::stringstream ss;
  
  ss << "p_id__ds_id_v= " << patch_sfc::vec_to_str<>(p_id__ds_id_v) << "\n"
     << "c_id__ds_id_v= " << patch_sfc::vec_to_str<>(c_id__ds_id_v) << "\n"
     << "wa_space= \n" << wa_space.to_str() << "\n";
  
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

void PCSim::sim(std::vector<app_id__lcoor_ucoor_pair>& p_id__lcoor_ucoor_pair_v,
                std::vector<app_id__lcoor_ucoor_pair>& c_id__lcoor_ucoor_pair_v)
{
  for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = p_id__lcoor_ucoor_pair_v.begin(); it != p_id__lcoor_ucoor_pair_v.end(); it++) {
    wa_space.put(it->first, "dummy", 0, (it->second).first, (it->second).second);
    // LOG(INFO) << "sim:: put " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
  }
    
  for (std::vector<app_id__lcoor_ucoor_pair>::iterator it = c_id__lcoor_ucoor_pair_v.begin(); it != c_id__lcoor_ucoor_pair_v.end(); it++) {
    char c_ds_id = c_id__ds_id_v[it->first - p_id__ds_id_v.size() ];
    
    char get_type;
    std::vector<kv_lucoor_pair> next_kv_lucoor_pair_v;
    if (wa_space.get(it->first, "dummy", 0, (it->second).first, (it->second).second,
                     get_type, next_kv_lucoor_pair_v) )
      LOG(WARNING) << "sim:: could NOT get " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
    else {
      for (int i = 0; i < next_kv_lucoor_pair_v.size(); i++) {
        kv_lucoor_pair kv_lu_pair = next_kv_lucoor_pair_v[i];
        if (wa_space.prefetch(c_ds_id, (kv_lu_pair.first).first, (kv_lu_pair.first).second, (kv_lu_pair.second).first, (kv_lu_pair.second).second) ) {
          // LOG(WARNING) << "get:: prefetch failed! for " << LUCOOR_TO_STR(lu_pair.first, lu_pair.second) << "\n";
        }
        else {
          // LOG(WARNING) << "get:: prefetched " << LUCOOR_TO_STR(lu_pair.first, lu_pair.second) << "\n";
        }
      }
      
      // LOG(INFO) << "sim:: got get_type= " << get_type << "; " << LUCOOR_TO_STR((it->second).first, (it->second).second) << "\n";
      c_id__get_type_v_map[it->first].push_back(get_type);
    }
  }
  
}
