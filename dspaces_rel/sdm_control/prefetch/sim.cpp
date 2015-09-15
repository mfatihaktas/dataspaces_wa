#include "sim.h"

/********************************************  PCSim  *********************************************/
PCSim::PCSim(std::vector<char> ds_id_v, int num_p, int num_c,
               std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
               std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
               std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
               std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v)
: num_p(num_p), num_c(num_c),
  p_id__ds_id_v(p_id__ds_id_v), c_id__ds_id_v(c_id__ds_id_v),
  p_id__num_put_v(p_id__num_put_v), c_id__num_get_v(c_id__num_get_v),
  p_id__put_rate_v(p_id__put_rate_v), c_id__get_rate_v(c_id__get_rate_v),
  p_id__inter_arr_time_v_v(p_id__inter_arr_time_v_v), c_id__inter_arr_time_v_v(c_id__inter_arr_time_v_v)
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
  for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = thread_v.begin(); it != thread_v.end(); it++)
    (*it)->join();
  // 
  LOG(INFO) << "PCSim:: destructed= \n" << to_str_end();
};

std::string PCSim::to_str()
{
  std::stringstream ss;
  ss << "num_p= " << boost::lexical_cast<std::string>(num_p) << "\n";
  ss << "p_id__ds_id_v= " << patch_all::vec_to_str<char>(p_id__ds_id_v) << "\n";
  ss << "p_id__num_put_v= " << patch_all::vec_to_str<int>(p_id__num_put_v) << "\n";
  ss << "p_id__put_rate_v= " << patch_all::vec_to_str<float>(p_id__put_rate_v) << "\n";
  
  ss << "num_c= " << boost::lexical_cast<std::string>(num_c) << "\n";
  ss << "c_id__ds_id_v= " << patch_all::vec_to_str<char>(c_id__ds_id_v) << "\n";
  ss << "c_id__num_get_v= " << patch_all::vec_to_str<int>(c_id__num_get_v) << "\n";
  ss << "c_id__get_rate_v= " << patch_all::vec_to_str<float>(c_id__get_rate_v) << "\n";
  
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

void PCSim::sim_all()
{
  for (int i = 0; i < num_p; i++)
    thread_v.push_back(boost::make_shared<boost::thread>(&PCSim::sim_p, this, i) );
  
  sleep(2);
  
  for (int i = 0; i < num_c; i++)
    thread_v.push_back(boost::make_shared<boost::thread>(&PCSim::sim_c, this, i, true) );
}

/************************************************  MPCSim  ****************************************/
MPCSim::MPCSim(std::vector<char> ds_id_v, int num_p, int num_c,
               std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
               std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
               std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
               std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
               int max_num_key_ver_in_mpbuffer, MALGO_T malgo_t)
: PCSim(ds_id_v, num_p, num_c,
        p_id__ds_id_v, c_id__ds_id_v,
        p_id__num_put_v, p_id__num_get_v,
        p_id__put_rate_v, c_id__get_rate_v,
        p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v),
  wa_space_(boost::make_shared<MWASpace>(ds_id_v, malgo_t, max_num_key_ver_in_mpbuffer,
                                         true, boost::bind(&MPCSim::handle_data_act, this, _1, _2, _3) ) ),
  lcoor_({BOOST_PP_ENUM(NDIM, FIXED_REP, 0) } ), ucoor_({BOOST_PP_ENUM(NDIM, FIXED_REP, 1) } )
{
  // 
  LOG(INFO) << "PCSim:: constructed; \n" << to_str();
}

std::string MPCSim::to_str()
{
  std::stringstream ss;
  ss << "PCSim::to_str= \n" << PCSim::to_str() << "\n"
     << "malgo_t= " << malgo_t << "\n"
     << "max_num_key_ver_in_mpbuffer= " << max_num_key_ver_in_mpbuffer << "\n";
  
  return ss.str();
}

void MPCSim::sim_p(int p_id)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = p_id__inter_arr_time_v_v[p_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++, c++) {
    // LOG(INFO) << "sim_p:: p_id= " << p_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    std::string key = "d_" + boost::lexical_cast<std::string>(p_id) + "_" + boost::lexical_cast<std::string>(c);
    unsigned int ver = 0;
    if (wa_space_->put(p_id, key, ver, lcoor_, ucoor_) )
      LOG(ERROR) << "sim_p:: wa_space_->put failed; " << KV_TO_STR(key, ver);
    
    key_ver_pair kv = std::make_pair(key, ver);
    bget_syncer.notify(kv);
    
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put " << KV_TO_STR(key, ver);
  }
}

void MPCSim::sim_c(int c_id, bool blocking_get)
{
  int c = 0;
  std::vector<float> inter_arr_time_vec = c_id__inter_arr_time_v_v[c_id];
  for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end(); it++, c++) {
    // LOG(INFO) << "sim_c:: c_id= " << c_id << ", inter_arr_time= " << *it << "\n";
    sleep(*it);
    
    std::string key = "d_" + boost::lexical_cast<std::string>(c_id) + "_" + boost::lexical_cast<std::string>(c);
    unsigned int ver = 0;
    std::vector<char> ds_id_v;
    if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
      if (blocking_get) {
        key_ver_pair kv = std::make_pair(key, ver);
        bget_syncer.add_sync_point(kv, 1);
        bget_syncer.wait(kv);
        bget_syncer.del_sync_point(kv);
      }
      else {
        LOG(WARNING) << "sim_c:: wa_space does not contain " << KV_TO_STR(key, ver);
        continue;
      }
    }
    
    char ds_id = c_id__ds_id_v[c_id];
    if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() )
      c_id__get_type_v_map[c_id].push_back('l');
    else { // Remote fetch
      wa_space_->put(NULL_P_ID, key, ver, lcoor_, ucoor_, ds_id);
      c_id__get_type_v_map[c_id].push_back('r');
    }
    
    wa_space_->add_access(key, ver, lcoor_, ucoor_);
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; GOT " << KV_TO_STR(key, ver);
  }
}

void MPCSim::handle_data_act(DATA_ACT_T data_act_t, key_ver_pair kv, lcoor_ucoor_pair lucoor)
{
  if (data_act_t == DATA_ACT_DEL)
    wa_space_->del(kv.first, kv.second, lucoor.first, lucoor.second);
  else if (data_act_t == DATA_ACT_PREFETCH)
    wa_space_->put(kv.first, kv.second, lucoor.first, lucoor.second);
}

/*******************************************  PCSim  **********************************************/
SPCSim::SPCSim(std::vector<char> ds_id_v, int num_p, int num_c,
               std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
               std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
               std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
               std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
               SPREDICTOR_T spredictor_t, int sexpand_length, COOR_T* lcoor_, COOR_T* ucoor_)
: PCSim(ds_id_v, num_p, num_c,
        p_id__ds_id_v, c_id__ds_id_v,
        p_id__num_put_v, p_id__num_get_v,
        p_id__put_rate_v, c_id__get_rate_v,
        p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v),
  wa_space_(boost::make_shared<SWASpace>(ds_id_v, spredictor_t, sexpand_length, lcoor_, ucoor_,
                                         true, boost::bind(&SPCSim::handle_data_act, this, _1, _2, _3) ) ),
  lcoor_({BOOST_PP_ENUM(NDIM, FIXED_REP, 0) } ), ucoor_({BOOST_PP_ENUM(NDIM, FIXED_REP, 1) } )
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

