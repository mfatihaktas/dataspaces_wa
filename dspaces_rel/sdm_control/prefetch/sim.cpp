#include "sim.h"

/********************************************  PCSim  *********************************************/
PCSim::PCSim(std::vector<int> ds_id_v, bool w_prefetch,
             int num_p, int num_c,
             std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
             std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
             std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
             std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v)
: w_prefetch(w_prefetch), num_p(num_p), num_c(num_c),
  p_id__ds_id_v(p_id__ds_id_v), c_id__ds_id_v(c_id__ds_id_v),
  p_id__num_put_v(p_id__num_put_v), c_id__num_get_v(c_id__num_get_v),
  p_id__put_rate_v(p_id__put_rate_v), c_id__get_rate_v(c_id__get_rate_v),
  p_id__inter_arr_time_v_v(p_id__inter_arr_time_v_v), c_id__inter_arr_time_v_v(c_id__inter_arr_time_v_v)
{
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
  ss << "w_prefetch= " << w_prefetch << "\n"
     << "num_p= " << num_p << "\n"
     << "p_id__ds_id_v= " << patch_all::vec_to_str<int>(p_id__ds_id_v) << "\n"
     << "p_id__num_put_v= " << patch_all::vec_to_str<int>(p_id__num_put_v) << "\n"
     << "p_id__put_rate_v= " << patch_all::vec_to_str<float>(p_id__put_rate_v) << "\n"
     << "p_id__inter_arr_time_v_v= \n";
  for (std::vector<std::vector<float> >::iterator it = p_id__inter_arr_time_v_v.begin(); it != p_id__inter_arr_time_v_v.end(); it++)
    ss << "\t" << patch_all::vec_to_str<>(*it) << "\n";
     
  ss << "num_c= " << num_c << "\n"
     << "c_id__ds_id_v= " << patch_all::vec_to_str<int>(c_id__ds_id_v) << "\n"
     << "c_id__num_get_v= " << patch_all::vec_to_str<int>(c_id__num_get_v) << "\n"
     << "c_id__get_rate_v= " << patch_all::vec_to_str<float>(c_id__get_rate_v) << "\n"
     << "c_id__inter_arr_time_v_v= \n";
  for (std::vector<std::vector<float> >::iterator it = c_id__inter_arr_time_v_v.begin(); it != c_id__inter_arr_time_v_v.end(); it++)
    ss << "\t" << patch_all::vec_to_str<>(*it) << "\n";
  
  return ss.str();
}

std::string PCSim::to_str_end()
{
  std::stringstream ss;
  ss << "c_id__get_type_v_map= \n";
  for (std::map<int, std::vector<char> >::iterator it = c_id__get_type_v_map.begin(); it != c_id__get_type_v_map.end(); it++)
    ss << "\t" << it->first << " : " << patch_all::vec_to_str<>(it->second) << "\n";
  
  ss << "c_id__get_lperc_map= \n" << patch_all::map_to_str<>(get_c_id__get_lperc_map() ) << "\n"
     << "wa_space= \n" << wa_space_->to_str() << "\n";
  
  return ss.str();
}

std::map<int, float> PCSim::get_c_id__get_lperc_map()
{
  std::map<int, float> c_id__get_lperc_map;
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
    thread_v.push_back(boost::make_shared<boost::thread>(&PCSim::sim_c, this, i, w_prefetch) );
}

void PCSim::handle_data_act(PREFETCH_DATA_ACT_T data_act_t, int ds_id, key_ver_pair kv, lcoor_ucoor_pair lucoor)
{
  // if (data_act_t == PREFETCH_DATA_ACT_DEL)
  //   wa_space_->del(kv.first, kv.second, lucoor.first, lucoor.second, ds_id);
  // else if (data_act_t == PREFETCH_DATA_ACT_PREFETCH) {
  //   LOG(INFO) << "handle_data_act:: before wa_space_->put; wa_space= \n" << wa_space_->to_str();
  //   if (wa_space_->put(NULL_P_ID, kv.first, kv.second, lucoor.first, lucoor.second, ds_id) )
  //     LOG(ERROR) << "handle_data_act:: wa_space_->put failed; " << KV_LUCOOR_TO_STR(kv.first, kv.second, lucoor.first, lucoor.second);
  // }
}

/************************************************  MPCSim  ****************************************/
MPCSim::MPCSim(std::vector<int> ds_id_v, bool w_prefetch,
               int num_p, int num_c,
               std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
               std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
               std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
               std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
               MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer)
: PCSim(ds_id_v, w_prefetch,
        num_p, num_c,
        p_id__ds_id_v, c_id__ds_id_v,
        p_id__num_put_v, c_id__num_get_v,
        p_id__put_rate_v, c_id__get_rate_v,
        p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v)
{
  wa_space_ = boost::make_shared<MWASpace>(ds_id_v, malgo_t, max_num_key_ver_in_mpbuffer,
                                           w_prefetch, boost::bind(&PCSim::handle_data_act, this, _1, _2, _3, _4) );
  for (int p_id = 0; p_id < num_p; p_id++)
    wa_space_->reg_app(p_id, p_id__ds_id_v[p_id] );
  
  for (int c_id = 0; c_id < num_c; c_id++) {
    wa_space_->reg_app(num_p + c_id, c_id__ds_id_v[c_id] );
    
    std::vector<char> get_type_v;
    c_id__get_type_v_map[c_id] = get_type_v;
  }
  // 
  COOR_T t_lcoor_[] = {BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T t_ucoor_[] = {BOOST_PP_ENUM(NDIM, FIXED_REP, 1) };
  
  lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  for (int i = 0; i < NDIM; i++) {
    lcoor_[i] = t_lcoor_[i];
    ucoor_[i] = t_ucoor_[i];
  }
  // 
  LOG(INFO) << "MPCSim:: constructed= \n" << to_str();
}

MPCSim::~MPCSim()
{
  patch_all::free_all<COOR_T>(2, lcoor_, ucoor_);
  // 
  LOG(INFO) << "MPCSim:: destructed.";
}

std::string MPCSim::to_str()
{
  std::stringstream ss;
  // ss << "PCSim::to_str= \n" << PCSim::to_str() << "\n"
  // ss << "wa_space= \n" << wa_space_->to_str() << "\n";

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
    
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
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
    std::vector<int> ds_id_v;
    // LOG(INFO) << "sim_c:: before query wa_space= \n" << wa_space_->to_str() << "\n";
    if (wa_space_->query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
      if (blocking_get) {
        key_ver_pair kv = std::make_pair(key, ver);
        LOG(INFO) << "sim_c:: c_id= " << c_id << ", blocking_get= " << blocking_get << ", waits on " << KV_TO_STR(key, ver);
        bget_syncer.add_sync_point(kv, 1);
        bget_syncer.wait(kv);
        bget_syncer.del_sync_point(kv);
      }
      else {
        LOG(WARNING) << "sim_c:: wa_space does not contain " << KV_TO_STR(key, ver);
        continue;
      }
    }
    
    int ds_id = c_id__ds_id_v[c_id];
    if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() ) {
      LOG(INFO) << "sim_c:: c_id= " << c_id << ", found in local ds_id= " << ds_id << "; " << KV_TO_STR(key, ver);
      c_id__get_type_v_map[c_id].push_back('l');
    }
    else { // Remote fetch
      // LOG(INFO) << "sim_c:: wa_space= \n" << wa_space_->to_str();
      LOG(INFO) << "sim_c:: c_id= " << c_id << ", found in remote ds_id_v= " << patch_all::vec_to_str<>(ds_id_v) << "; " << KV_TO_STR(key, ver);
      if (wa_space_->put(NULL_P_ID, key, ver, lcoor_, ucoor_, ds_id) )
        LOG(ERROR) << "sim_c:: wa_space_->put failed; ds_id= " << ds_id << ", " << KV_TO_STR(key, ver);
      // LOG(INFO) << "sim_c:: c_id= " << c_id << ", done with wa_space_->put; " << KV_TO_STR(key, ver);
      
      c_id__get_type_v_map[c_id].push_back('r');
    }
    
    // LOG(INFO) << "sim_c:: c_id= " << c_id << ", will call wa_space_->add_access; " << KV_TO_STR(key, ver);
    wa_space_->add_access(num_p + c_id, key, ver, lcoor_, ucoor_);
    // LOG(INFO) << "sim_c:: c_id= " << c_id << ", done with wa_space_->add_access; " << KV_TO_STR(key, ver);
    
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; GOT " << KV_TO_STR(key, ver);
  }
}

/*******************************************  SPCSim  *********************************************/
SPCSim::SPCSim(std::vector<int> ds_id_v, int num_p, int num_c,
               std::vector<int> p_id__ds_id_v, std::vector<int> c_id__ds_id_v,
               std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
               std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
               std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
               SALGO_T salgo_t, int sexpand_length, COOR_T* lcoor_, COOR_T* ucoor_, bool w_prefetch)
: PCSim(ds_id_v, w_prefetch,
        num_p, num_c,
        p_id__ds_id_v, c_id__ds_id_v,
        p_id__num_put_v, c_id__num_get_v,
        p_id__put_rate_v, c_id__get_rate_v,
        p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v)
{
  wa_space_ = boost::make_shared<SWASpace>(ds_id_v, salgo_t, lcoor_, ucoor_, sexpand_length,
                                           w_prefetch, boost::bind(&PCSim::handle_data_act, this, _1, _2, _3, _4) );
  for (int p_id = 0; p_id < num_p; p_id++)
    wa_space_->reg_app(p_id, p_id__ds_id_v[p_id] );
  
  for (int c_id = 0; c_id < num_c; c_id++) {
    wa_space_->reg_app(num_p + c_id, c_id__ds_id_v[c_id] );
    
    std::vector<char> get_type_v;
    c_id__get_type_v_map[c_id] = get_type_v;
  }
  // 
  this->lcoor_ = lcoor_;
  this->ucoor_ = ucoor_;
  // 
  LOG(INFO) << "SPCSim:: constructed= \n" << to_str();
}

std::string SPCSim::to_str()
{
  std::stringstream ss;
  // ss << "PCSim::to_str= \n" << PCSim::to_str() << "\n"
  // ss << "wa_space= \n" << wa_space_->to_str() << "\n";

  return ss.str();
}

void SPCSim::sim_p(int p_id)
{
  std::string key = "d_" + boost::lexical_cast<std::string>(p_id);
  
  if (wa_space_->put(p_id, key, 0, lcoor_, ucoor_) )
    LOG(ERROR) << "sim_p:: wa_space_->put failed; " << LUCOOR_TO_STR(lcoor_, ucoor_);
  else
    LOG(INFO) << "sim_p:: p_id= " << p_id << "; put " << LUCOOR_TO_STR(lcoor_, ucoor_);
  
  // std::vector<float> inter_arr_time_vec = p_id__inter_arr_time_v_v[p_id];
  // for (std::vector<float>::iterator it = inter_arr_time_vec.begin(); it != inter_arr_time_vec.end() ; it++, c++) {
  //   // LOG(INFO) << "sim_p:: p_id= " << p_id << ", inter_arr_time= " << *it << "\n";
  //   sleep(*it);
    
  //   if (wa_space_->put(p_id, key, ver, lcoor_, ucoor_) )
  //     LOG(ERROR) << "sim_p:: wa_space_->put failed; " << LUCOOR_TO_STR(lcoor_, ucoor_);
  //   else
  //     LOG(INFO) << "sim_p:: p_id= " << p_id << "; put " << LUCOOR_TO_STR(lcoor_, ucoor_);
  // }
}

void SPCSim::sim_c(int c_id, bool blocking_get)
{
  std::string key = "d_" + boost::lexical_cast<std::string>(c_id);
  std::vector<lcoor_ucoor_pair> lucoor_v;
  get_lucoor_to_acc_v(lucoor_v);
  
  int c = 0;
  std::vector<float> inter_arr_time_vec = c_id__inter_arr_time_v_v[c_id];
  for (std::vector<lcoor_ucoor_pair>::iterator it = lucoor_v.begin(); it != lucoor_v.end(); it++, c++) {
    // LOG(INFO) << "sim_c:: c_id= " << c_id << ", inter_arr_time= " << inter_arr_time_vec[c];
    sleep(inter_arr_time_vec[c] );
    
    std::vector<int> ds_id_v;
    if (wa_space_->query(key, 0, it->first, it->second, ds_id_v) ) {
      if (blocking_get) {
        CREATE_BOX(0, b, it->first, it->second)
        s_syncer.add_sync_point(b0, 1);
        s_syncer.wait(b0);
        s_syncer.del_sync_point(b0);
      }
      else {
        LOG(WARNING) << "sim_c:: wa_space does not contain " << LUCOOR_TO_STR(it->first, it->second);
        continue;
      }
    }
    
    int ds_id = c_id__ds_id_v[c_id];
    if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() )
      c_id__get_type_v_map[c_id].push_back('l');
    else { // Remote fetch
      // Note: if w_prefetch, queried + predicted data will be prefetched by salgo
      if (!w_prefetch)
        wa_space_->put(NULL_P_ID, key, 0, it->first, it->second, ds_id);
      c_id__get_type_v_map[c_id].push_back('r');
    }
    
    wa_space_->add_access(c_id, key, 0, lcoor_, ucoor_);
    LOG(INFO) << "sim_c:: c_id= " << c_id << "; GOT " << LUCOOR_TO_STR(it->first, it->second);
  }
}

// Walk whole space with incremental boxes
void SPCSim::get_lucoor_to_acc_v(std::vector<lcoor_ucoor_pair>& lucoor_v)
{
  MULTI_FOR(lcoor_, ucoor_)
    COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    COOR_T walk_ucoor_[NDIM];
    for (int i = 0; i < NDIM; i++)
      walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
    COOR_T* dwalk_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    COOR_T* dwalk_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    for (int i = 0; i < NDIM; i++) {
      dwalk_lcoor_[i] = walk_lcoor_[i];
      dwalk_ucoor_[i] = walk_ucoor_[i];
    }
    
    lucoor_v.push_back(std::make_pair(dwalk_lcoor_, dwalk_ucoor_) );
  END_MULTI_FOR()
}
