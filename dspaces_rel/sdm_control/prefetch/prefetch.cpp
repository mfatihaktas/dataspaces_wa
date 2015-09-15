#include "prefetch.h"

/******************************************  MPBuffer  *********************************************/
MPBuffer::MPBuffer(char ds_id, int max_num_key_ver, MALGO_T malgo_t,
                   bool w_prefetch, func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb = 0)
: ds_id(ds_id), max_num_key_ver(max_num_key_ver),
  w_prefetch(w_prefetch), handle_mpbuffer_data_act_cb(handle_mpbuffer_data_act_cb),
  cache(max_num_key_ver, boost::bind($MPBuffer::handle_data_del, this, _1) )
{
  if (malgo_t == MALGO_W_LZ)
    malgo_to_pick_app_ = boost::make_shared<LZAlgo>();
  else if (malgo_t == MALGO_W_ALZ)
    malgo_to_pick_app_ = boost::make_shared<ALZAlgo>();
  else if (malgo_t == MALGO_W_PPM)
    malgo_to_pick_app_ = boost::make_shared<PPMAlgo>(2);
  else if (malgo_t == MALGO_W_PO)
    malgo_to_pick_app_ = boost::make_shared<POAlgo>();
  else {
    LOG(ERROR) << "MPBuffer:: unknown malgo_t= " << malgo_t;
    exit(1);
  }
  // std::map<MALGO_T, float> malgo_t__weight_map;
  // malgo_t__weight_map[MALGO_W_LZ] = 0.5;
  // malgo_t__weight_map[MALGO_W_PPM] = 0.5;
  // malgo_t__weight_map[MALGO_W_PO] = 0.33;
  // malgo_to_pick_app_ = boost::make_shared<MPrefetchAlgo>(malgo_t, malgo_t__weight_map);
  // 
  LOG(INFO) << "MPBuffer:: constructed.";
}

MPBuffer::~MPBuffer() { LOG(INFO) << "MPBuffer:: destructed."; }

std::string MPBuffer::to_str()
{
  std::stringstream ss;
  ss << "w_prefetch= " << w_prefetch << "\n";
     << "max_num_key_ver= " << max_num_key_ver << "\n";
  
  // ss << "acced_kv_v= \n";
  // for (std::vector<key_ver_pair>::iterator it = acced_kv_v.begin();
  //     it != acced_kv_v.end(); it++) {
  //   ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << ">\n";
  // }
  
  // ss << "app_id__acced_kv_v_map= \n";
  // for (std::map<int, std::vector<key_ver_pair> >::iterator map_it = app_id__acced_kv_v_map.begin();
  //     map_it != app_id__acced_kv_v_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::vector<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  // ss << "p_id__reged_kv_deq_map= \n";
  // for (std::map<int, std::deque<key_ver_pair> >::iterator map_it = p_id__reged_kv_deq_map.begin(); 
  //     map_it != p_id__reged_kv_deq_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::deque<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  ss << "cache= \n" << cache.to_str() << "\n";
  
  // ss << "malgo_to_pick_app_= \n"
  //   << "\t parse_tree_to_pstr= \n" << malgo_to_pick_app_->parse_tree_to_pstr() << "\n";
  
  return ss.str();
}

// ----------------------------------------  state rel  ----------------------------------------- //
int MPBuffer::reg_key_ver(int p_id, key_ver_pair kv)
{
  if (kv__p_id_map.contains(kv) ) {
    LOG(ERROR) << "reg_key_ver:: already registered; " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  kv__p_id_map[kv] = p_id;
  // 
  if (p_id__reged_kv_deq_map.contains(p_id) )
    p_id__reged_kv_deq_map[p_id].push_back(kv);
  else {
    std::deque<key_ver_pair> kv_deq;
    kv_deq.push_back(kv);
    p_id__reged_kv_deq_map[p_id] = kv_deq;
    p_id__front_step_in_deq_map[p_id] = 0;
  }
}

// ----------------------------------------  operational  --------------------------------------- //
int MPBuffer::add_access(key_ver_pair kv)
{
  if (!kv__p_id_map.contains(kv) ) {
    LOG(WARNING) << "add_access:: non-registered; " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  int p_id = kv__p_id_map[kv];
  if (!cache.del(p_id, kv) )
    handle_mpbuffer_data_act_cb(DATA_ACT_DEL, ds_id, kv);
  // 
  // acced_kv_v.push_back(kv);
  // app_id__acced_kv_v_map[p_id].push_back(kv);
  
  if (!p_id__last_acced_step_map.contains(p_id) )
    p_id__last_acced_step_map[p_id] = 0;
  else
    p_id__last_acced_step_map[p_id] += 1;
  // 
  int num_app = 1;
  std::vector<key_ver_pair> kv_v;
  {// Causes problems while building the parse tree for multi-threaded scenario
    boost::lock_guard<boost::mutex> guard(add_acc_mutex);
    
    if (malgo_to_pick_app_->add_access(p_id) )
      LOG(ERROR) << "add_access:: malgo_to_pick_app_->add_access failed for p_id= " << p_id;
  
    get_to_prefetch(num_app, kv_v);
    // LOG(INFO) << "add_access:: get_to_prefetch returned kv_v= " << patch_all::pvec_to_str<key_ver_pair>(kv_v) << "\n";
    // To avoid remote fetching while a kv is being prefetched -- assuming prefetching will never fail
    if (cache.size() + kv_v.size() != max_num_key_ver)
      std::cout << "ADD_ACCESS:: <" << kv.first << ", " << kv.second << "> \n"
                << "\t cache= \n" << patch_all::pvec_to_str<key_ver_pair>(cache.get_content_v() ) << "\n"
                << ">> Will prefetch kv_v= \n" << patch_all::pvec_to_str<key_ver_pair>(kv_v) << "\n";
    
    for (std::vector<key_ver_pair>::iterator it = kv_v.begin(); it != kv_v.end(); it++) {
      if (cache.push(p_id, *it) )
        LOG(ERROR) << "add_access:: cache.push failed for <" << it->first << ", " << it->second << "> \n"
                   << "\t cache= \n" << patch_all::pvec_to_str<key_ver_pair>(cache.get_content_v() );
    }
  }
  
  // Call for prefetching per access
  // TODO: Prefetching action is not put into above mutexed region to not make the other threads wait
  // for the action but this may affect the operational correctness in case of failure in handling the
  // prefetching action
  // Idea: A failure handler here fired by the action handler can be used to correct the cache content
  if (w_prefetch) {
    for (std::vector<key_ver_pair>::iterator it = kv_v.begin(); it != kv_v.end(); it++)
      handle_mpbuffer_data_act_cb(DATA_ACT_PREFETCH, ds_id, *it);
  }
    
  return 0;
}

int MPBuffer::get_to_prefetch(int& num_app, std::vector<key_ver_pair>& kv_v)
{
  // Pick app
  std::vector<ACC_T> p_id_v, ep_id_v;
  malgo_to_pick_app_->get_to_prefetch(num_app, p_id_v, cache.get_cached_acc_v(), ep_id_v);
  std::cout << "-------------------------------------------------\n"
            << "GET_TO_PREFETCH:: \n"
            << "p_id_v= " << patch_all::vec_to_str<ACC_T>(p_id_v) << "\n"
            << "ep_id_v= " << patch_all::vec_to_str<ACC_T>(ep_id_v) << "\n"
            << "\t cache= " << patch_all::vec_to_str<>(cache.get_cached_acc_v() ) << "\n";
  
  for (std::vector<ACC_T>::iterator jt = ep_id_v.begin(); jt != ep_id_v.end(); jt++)
    p_id_v.push_back(*jt);
  // 
  for (std::vector<ACC_T>::iterator it = p_id_v.begin(); it != p_id_v.end(); it++) {
    if (!p_id__last_cached_step_map.contains(*it) )
      p_id__last_cached_step_map[*it] = -1;

    // std::cout << "for p_id= " << *it << ":\n"
    //           << "\t p_id__last_cached_step_map= " << p_id__last_cached_step_map[*it] << "\n"
    //           << "\t p_id__last_acced_step_map= " << p_id__last_acced_step_map[*it] << "\n";
    if (p_id__last_cached_step_map[*it] < p_id__last_acced_step_map[*it] )
      p_id__last_cached_step_map[*it] = p_id__last_acced_step_map[*it];
    else if (p_id__last_cached_step_map[*it] > p_id__last_acced_step_map[*it] )
      continue;
    // 
    ACC_T step_to_prefetch = p_id__last_cached_step_map[*it] + 1;
    // std::cout << "step_to_prefetch= " << step_to_prefetch << "\n";
    std::deque<key_ver_pair>& kv_deq = p_id__reged_kv_deq_map[*it];
    // std::cout << "p_id = " << *it << ", kv_deq= \n";
    // for (std::deque<key_ver_pair>::iterator dit = kv_deq.begin(); dit != kv_deq.end(); dit++)
    //   std::cout << "\t <" << dit->first << "," << dit->second << "> \n";
    
    ACC_T front_step_in_deq = p_id__front_step_in_deq_map[*it];
    // std::cout << "front_step_in_deq= " << front_step_in_deq << "\n";
    while (!kv_deq.empty() && front_step_in_deq < step_to_prefetch) {
      kv_deq.pop_front();
      front_step_in_deq++;
    }
    
    if (kv_deq.empty() )
      continue;
    
    key_ver_pair kv = kv_deq.front();
    // LOG(INFO) << "will kv_v.push_back <" << kv.first << ", " << kv.second << ">";
    kv_v.push_back(kv_deq.front() );
    kv_deq.pop_front();
    front_step_in_deq++;
    
    p_id__last_cached_step_map[*it] = step_to_prefetch;
    p_id__front_step_in_deq_map[*it] = front_step_in_deq;
  }
}

bool MPBuffer::contains(key_ver_pair kv) { return cache.contains(kv); }

std::vector<key_ver_pair> MPBuffer::get_content_v() { return cache.get_content_v(); }

void MPBuffer::sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> kv_v, 
                                     float& hit_rate, std::vector<char>& accuracy_v)
{
  int num_miss = 0;
  
  std::vector<int>::iterator pid_it;
  std::vector<key_ver_pair>::iterator kv_it;
  for (pid_it = p_id_v.begin() , kv_it = kv_v.begin(); pid_it != p_id_v.end(), kv_it != kv_v.end(); pid_it++, kv_it++)
    reg_key_ver(*pid_it, *kv_it);
  
  for (pid_it = p_id_v.begin() , kv_it = kv_v.begin(); pid_it != p_id_v.end(), kv_it != kv_v.end(); pid_it++, kv_it++) {
    // std::cout << "sim_prefetch_accuracy:: is <" << kv_it->first << ", " << kv_it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    if (!cache.contains(*kv_it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    add_access(*kv_it);
  }
  
  hit_rate = 1.0 - (float)num_miss / kv_v.size();
}

int MPBuffer::handle_data_del(key_ver_pair kv)
{ 
  if (handle_mpbuffer_data_act_cb != 0)
    handle_mpbuffer_data_act_cb(DATA_ACT_DEL, ds_id, kv);
}

/*******************************************  WASpace  ********************************************/
WASpace::WASpace(std::vector<char> ds_id_v, func_handle_data_act_cb handle_data_act_cb = 0)
: ds_id_v(ds_id_v), handle_data_act_cb(handle_data_act_cb)
{}

std::string WASpace::to_str()
{
  std::stringstream ss;
  ss << "\t ds_id_v= " << patch_all::vec_to_str<>(ds_id_v) << "\n"
     << "\t app_id__ds_id_map= \n" << patch_all::map_to_str<>(app_id__ds_id_map) << "\n";
  return ss.str();
}

int WASpace::reg_ds(char ds_id)
{
  if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() ) {
    LOG(WARNING) << "reg_ds:: ds_id= " << ds_id << " is already reged!";
    return 1;
  }
  ds_id_v.push_back(ds_id);
  
  return 0;
}

int WASpace::reg_app(int app_id, char ds_id)
{
  if (app_id__ds_id_map.count(app_id) != 0) {
    LOG(ERROR) << "reg_app:: already reged app_id= " << app_id;
    return 1;
  }
  app_id__ds_id_map[app_id] = ds_id;
  
  return 0;
}

/******************************************  MWASpace  ********************************************/
MWASpace::MWASpace(std::vector<char> ds_id_v,
                   MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0)
: WASpace(ds_id_v, handle_data_act_cb)
{
  for (std::vector<char>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
    ds_id__kv_vp_map[*it] = boost::make_shared<patch_all::thread_safe_vector<key_ver_pair> >();
    
    ds_id__mpbuffer_map[*it] = boost::make_shared<MPBuffer>(*it, max_num_key_ver_in_mpbuffer, malgo_t,
                                                            w_prefetch, boost::bind(&MWASpace::handle_mpbuffer_data_act, this, _1, _2, _3) );
  }
  // 
  LOG(INFO) << "MWASpace:: constructed.";
}

std::string MWASpace::to_str()
{
  std::stringstream ss;
  
  ss << "ds_id__kv_vp_map= \n";
  for (std::map<char, boost::shared_ptr<(std::vector<key_ver_pair> ) >::iterator map_it = ds_id__kv_vp_map.begin(); map_it != ds_id__kv_vp_map.end(); map_it++) {
    ss << "ds_id= " << map_it->first << "\n";
    for (std::vector<T>::iterator vec_it = (map_it->second)->begin(); vec_it != (map_it->second)->end(); vec_it++) {
      ss << "\t <" << vec_it->first << ", " << vec_it->second << "> \n ";
    }
  }
  
  ss << "ds_id__mpbuffer_map= \n";
  for (std::map<char, boost::shared_ptr<MPBuffer> >::iterator it = ds_id__mpbuffer_map.begin(); it != ds_id__mpbuffer_map.end(); it++) {
    ss << ">>> ds_id= " << it->first << "\n"
       << "\t mpbuffer= \n" << (it->second)->to_str() << "\n";
  }
  
  for (std::vector<char>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
    patch_all::thread_safe_vector& ds_kv_v = *ds_id__kv_vp_map[*it];
    std::sort(ds_kv_v.begin(), ds_kv_v.end() );
    
    std::vector<key_ver_pair>& mpbuffer_kv_v = ds_id__mpbuffer_map[*it]->get_kv_v();
    std::sort(mpbuffer_kv_v.begin(), mpbuffer_kv_v.end() );
    
    std::vector<key_ver_pair> intersect_vec;
    std::set_intersection(ds_kv_v.begin(), ds_kv_v.end(),
                          mpbuffer_kv_v.begin(), mpbuffer_kv_v.end(), 
                          back_inserter(intersect_vec) );
    
    ss << "ds_id= " << *it << "; ";
    if (intersect_vec.empty() )
      ss << "No intersection between ds and mpbuffer.\n";
    else
      ss << "INTERSECTION between ds and mpbuffer.\n";
  }
  
  return ss.str();
}

int MWASpace::put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '')
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (kv_v.contains(kv) ) {
    LOG(ERROR) << "put:: already in; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  if (ds_id == '')
    ds_id = app_id__ds_id_map[p_id];
  ds_id__kv_vp_map[ds_id]->push_back(kv);
  // Immediately broadcast it to every ds peer so mpbuffers can be updated
  for (std::map<char, boost::shared_ptr<MPBuffer> >::iterator it = ds_id__mpbuffer_map.begin(); it != ds_id__mpbuffer_map.end(); it++)
    (it->second)->reg_key_ver(p_id, kv);
  
  kv__p_id_map[kv] = p_id;
  kv_v.push_back(kv);
  
  kv__lucoor_map[kv] = std::make_pair(lcoor_, ucoor_);
  
  return 0;
}

int MWASpace::query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!contains('*', kv) )
    return 1;
  
  ds_id_v.push_back(app_id__ds_id_map[kv__p_id_map[kv] ] );
  return 0;
}

int MWASpace::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  key_ver_pair kv = std::make_pair(key, ver);
  return ds_id__mpbuffer_map[app_id__ds_id_map[c_id] ]->add_access(kv);
}

bool MWASpace::contains(char ds_id, key_ver_pair kv)
{
  if (ds_id == '*')
    return kv_v.contains(kv);
  else
    return ds_id__kv_vp_map[ds_id]->contains(kv) || ds_id__mpbuffer_map[ds_id]->contains(kv);
}

void MWASpace::handle_mpbuffer_data_act(DATA_ACT_T data_act_t, char ds_id, key_ver_pair kv) {
  if (handle_data_act_cb != 0)
    handle_data_act_cb(data_act_t, ds_id, kv, kv__lucoor_map[kv] );
}

/******************************************  SWASpace  ********************************************/
SWASpace::SWASpace(std::vector<char> ds_id_v,
                   SPREDICTOR_T spredictor_t, int expand_length, COOR_T* lcoor_, COOR_T* ucoor_,
                   bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0)
: WASpace(ds_id_v, handle_data_act_cb),
  qtable_(boost::make_shared<RTable<char> >() )
{
  IS_VALID_BOX("SWASpace", lcoor_, ucoor_, exit(1) )
  
  if (spredictor_t == HILBERT_SPREDICTOR)
    predictor_ = boost::make_shared<HPredictor>(expand_length, lcoor_, ucoor_);
  else {
    LOG(ERROR) << "SWASpace:: unknown spredictor_t= " << spredictor_t;
    exit(1);
  }
  // 
  LOG(INFO) << "SWASpace:: constructed.";
}

std::string SWASpace::to_str()
{
  std::stringstream ss;
  ss << "predictor= \n" << predictor_->to_str() << "\n"
     << "WASpace::to_str= \n" << WASpace::to_str << "\n";
    // << "qtable= \n" << qtable_->to_str() << "\n";
  
  return ss.str();
}

int SWASpace::put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '')
{
  IS_VALID_BOX("put", lcoor_, ucoor_, return 1)
  
  if (p_id != NULL_P_ID) {
    if (app_id__ds_id_map.count(p_id) == 0) {
      LOG(ERROR) << "put:: non-reged p_id= " << p_id;
      return 1;
    }
  }
  
  if (ds_id == '')
    ds_id = app_id__ds_id_map[p_id];
  
  if (qtable_->add(key, ver, lcoor_, ucoor_, ds_id) ) {
    LOG(ERROR) << "put:: qtable_->add failed! for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) << "\n";
    return 1;
  }
  
  return 0;
}

int SWASpace::query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v)
{ 
  return qtable_->query(key, ver, lcoor_, ucoor_, ds_id_v);
}

int SWASpace::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  IS_VALID_BOX("add_access", lcoor_, ucoor_, return 1)
  
  char ds_id = app_id__ds_id_map[c_id];
  
  std::vector<kv_lucoor_pair> predicted_kv_lucoor_pair_v;
  if (predictor_->add_acc__predict_next_acc(ds_id, key, ver, lcoor_, ucoor_,
                                            predicted_kv_lucoor_pair_v) ) {
    LOG(ERROR) << "add_access:: predictor_->add_acc__predict_next_acc failed; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  if (w_prefetch) {
    for (std::vector<kv_lucoor_pair>::iterator it = predicted_kv_lucoor_pair_v.begin(); it != predicted_kv_lucoor_pair_v.end(); it++)
      handle_data_act_cb(DATA_ACT_PREFETCH, ds_id, it->first, it->second);
  }
  
  return 0;
}
