#include "prefetch.h"

/******************************************  MPBuffer  *********************************************/
MPBuffer::MPBuffer(int ds_id, int max_num_key_ver, MALGO_T malgo_t,
                   bool w_prefetch, func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb)
: ds_id(ds_id), max_num_key_ver(max_num_key_ver),
  w_prefetch(w_prefetch), handle_mpbuffer_data_act_cb(handle_mpbuffer_data_act_cb),
  cache(max_num_key_ver, boost::bind(&MPBuffer::handle_data_del, this, _1) )
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
  LOG(INFO) << "MPBuffer:: constructed; \n" << to_str();
}

MPBuffer::~MPBuffer() { LOG(INFO) << "MPBuffer:: destructed."; }

std::string MPBuffer::to_str()
{
  std::stringstream ss;
  ss << "ds_id= " << ds_id << "\n"
     << "w_prefetch= " << w_prefetch << "\n"
     << "max_num_key_ver= " << max_num_key_ver << "\n";
  
  // ss << "kv__p_id_map= \n";
  // for (std::map<key_ver_pair, int>::iterator it = kv__p_id_map.begin(); it != kv__p_id_map.end(); it++)
  //   ss << "\t" << KV_TO_STR((it->first).first, (it->first).second) << " : " << it->second << "\n";
  
  // ss << "acced_kv_v= " << patch_all::pvec_to_str<key_ver_pair>(acced_kv_v);
  
  // ss << "app_id__acced_kv_v_map= \n";
  // for (std::map<int, std::vector<key_ver_pair> >::iterator map_it = app_id__acced_kv_v_map.begin(); map_it != app_id__acced_kv_v_map.end(); map_it++)
  //   ss << "\t p_id= " << map_it->first << " : " << patch_all::pvec_to_str<key_ver_pair>(map_it->second);
  
  // ss << "p_id__reged_kv_deq_map= \n";
  // for (std::map<int, std::deque<key_ver_pair> >::iterator map_it = p_id__reged_kv_deq_map.begin(); map_it != p_id__reged_kv_deq_map.end(); map_it++) {
  //   ss << "\t p_id= " << map_it->first << " : ";
  //   for (std::deque<key_ver_pair>::iterator kv_it = (map_it->second).begin(); kv_it != (map_it->second).end(); kv_it++)
  //     ss << "\t\t " << KV_TO_STR(kv_it->first, kv_it->second) << "\n";
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
  // LOG(INFO) << "reg_key_ver:: reged p_id= " << p_id << ", " << KV_TO_STR(kv.first, kv.second);
}

// ----------------------------------------  operational  --------------------------------------- //
int MPBuffer::del(key_ver_pair kv)
{
  if (!kv__p_id_map.contains(kv) ) {
    LOG(ERROR) << "del:: kv__p_id_map does not contain; non-registered " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  int p_id = kv__p_id_map[kv];
  if (cache.del(p_id, kv) ) {
    LOG(WARNING) << "del:: cache.del failed for p_id= " << p_id << ", " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  // Note: Causes duplicate call on handle_mpbuffer_data_act_cb for deling the same data since cache.del does it already
  // if (handle_mpbuffer_data_act_cb != 0)
  //   handle_mpbuffer_data_act_cb(PREFETCH_DATA_ACT_DEL, ds_id, kv);
  
  return 0;
}

int MPBuffer::add_access(key_ver_pair kv)
{
  LOG(INFO) << "add_access:: started; " << KV_TO_STR(kv.first, kv.second);
  if (!kv__p_id_map.contains(kv) ) {
    LOG(ERROR) << "add_access:: kv__p_id_map does not contain; non-registered " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  if (del(kv) ) {
    LOG(WARNING) << "add_access:: del failed; " << KV_TO_STR(kv.first, kv.second);
  }
  // acced_kv_v.push_back(kv);
  // app_id__acced_kv_v_map[p_id].push_back(kv);
  
  int p_id = kv__p_id_map[kv];
  if (!p_id__last_acced_step_map.contains(p_id) )
    p_id__last_acced_step_map[p_id] = -1;
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
      std::cout << "ADD_ACCESS:: " << KV_TO_STR(kv.first, kv.second) << "\n"
                << "\t cache= \n" << patch_all::pvec_to_str<key_ver_pair>(cache.get_content_v() ) << "\n"
                << ">> Will prefetch kv_v= \n" << patch_all::pvec_to_str<key_ver_pair>(kv_v) << "\n";
    
    for (std::vector<key_ver_pair>::iterator it = kv_v.begin(); it != kv_v.end(); it++) {
      if (cache.push(kv__p_id_map[*it], *it) )
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
    for (std::vector<key_ver_pair>::iterator it = kv_v.begin(); it != kv_v.end(); it++) {
      if (handle_mpbuffer_data_act_cb != 0)
        handle_mpbuffer_data_act_cb(PREFETCH_DATA_ACT_PREFETCH, ds_id, *it);
    }
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

    // std::cout << "get_to_prefetch:: for p_id= " << *it << ":\n"
    //           << "\t p_id__last_cached_step_map= " << p_id__last_cached_step_map[*it] << "\n"
    //           << "\t p_id__last_acced_step_map= " << p_id__last_acced_step_map[*it] << "\n";
    if (p_id__last_cached_step_map[*it] < p_id__last_acced_step_map[*it] )
      p_id__last_cached_step_map[*it] = p_id__last_acced_step_map[*it];
    else if (p_id__last_cached_step_map[*it] > p_id__last_acced_step_map[*it] )
      continue;
    // 
    ACC_T step_to_prefetch = p_id__last_cached_step_map[*it] + 1;
    // std::cout << "get_to_prefetch:: step_to_prefetch= " << step_to_prefetch << "\n";
    std::deque<key_ver_pair>& kv_deq = p_id__reged_kv_deq_map[*it];
    // std::cout << "get_to_prefetch:: p_id = " << *it << ", kv_deq= \n";
    // for (std::deque<key_ver_pair>::iterator dit = kv_deq.begin(); dit != kv_deq.end(); dit++)
    //   std::cout << "\t <" << dit->first << "," << dit->second << "> \n";
    
    ACC_T& front_step_in_deq = p_id__front_step_in_deq_map[*it];
    // std::cout << "front_step_in_deq= " << front_step_in_deq << "\n";
    while (!kv_deq.empty() && front_step_in_deq < step_to_prefetch) {
      kv_deq.pop_front();
      front_step_in_deq++;
    }
    
    if (kv_deq.empty() )
      continue;
    
    // key_ver_pair kv = kv_deq.front();
    // LOG(INFO) << "get_to_prefetch:: will kv_v.push_back <" << kv.first << ", " << kv.second << ">";
    kv_v.push_back(kv_deq.front() );
    kv_deq.pop_front();
    front_step_in_deq++;
    
    p_id__last_cached_step_map[*it] = step_to_prefetch;
    p_id__front_step_in_deq_map[*it] = front_step_in_deq;
  }
}

bool MPBuffer::contains(key_ver_pair kv) { return cache.contains(kv); }

std::vector<key_ver_pair> MPBuffer::get_kv_v() { return cache.get_content_v(); }

int MPBuffer::handle_data_del(key_ver_pair kv)
{ 
  if (!kv__p_id_map.contains(kv) ) {
    LOG(ERROR) << "handle_data_del:: kv__p_id_map does not contain; non-registered " << KV_TO_STR(kv.first, kv.second);
    return 1;
  }
  
  if (handle_mpbuffer_data_act_cb != 0)
    handle_mpbuffer_data_act_cb(PREFETCH_DATA_ACT_DEL, ds_id, kv);
}

void MPBuffer::sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> kv_v, 
                                     float& hit_rate, std::vector<char>& accuracy_v)
{
  int num_miss = 0;
  
  std::vector<int>::iterator pid_it;
  std::vector<key_ver_pair>::iterator kv_it;
  for (pid_it = p_id_v.begin(), kv_it = kv_v.begin(); pid_it != p_id_v.end(), kv_it != kv_v.end(); pid_it++, kv_it++)
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
  
  hit_rate = 1.0 - (float)num_miss/kv_v.size();
}

/*******************************************  WASpace  ********************************************/
WASpace::WASpace(std::vector<int> ds_id_v, func_handle_data_act_cb handle_data_act_cb)
: ds_id_v(ds_id_v), handle_data_act_cb(handle_data_act_cb)
{
  // 
  LOG(INFO) << "WASpace:: constructed.";
}

std::string WASpace::to_str()
{
  std::stringstream ss;
  ss << "\t ds_id_v= " << patch_all::vec_to_str<>(ds_id_v) << "\n"
     << "\t app_id__ds_id_map= \n" << app_id__ds_id_map.to_str() << "\n";
  return ss.str();
}

int WASpace::reg_ds(int ds_id)
{
  if (std::find(ds_id_v.begin(), ds_id_v.end(), ds_id) != ds_id_v.end() ) {
    LOG(WARNING) << "reg_ds:: ds_id= " << ds_id << " is already reged!";
    return 1;
  }
  ds_id_v.push_back(ds_id);
  LOG(INFO) << "reg_ds:: reged ds_id= " << ds_id;
  
  return 0;
}

int WASpace::reg_app(int app_id, int ds_id)
{
  if (app_id__ds_id_map.contains(app_id) ) {
    // LOG(ERROR) << "reg_app:: already reged app_id= " << app_id;
    return 1;
  }
  app_id__ds_id_map[app_id] = ds_id;
  
  return 0;
}

/******************************************  MWASpace  ********************************************/
MWASpace::MWASpace(std::vector<int> ds_id_v,
                   MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb)
: WASpace(ds_id_v, handle_data_act_cb),
  max_num_key_ver_in_mpbuffer(max_num_key_ver_in_mpbuffer), malgo_t(malgo_t), w_prefetch(w_prefetch)
{
  for (std::vector<int>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
    ds_id__kv_map[*it] = boost::make_shared<patch_all::thread_safe_vector<key_ver_pair> >();
    
    ds_id__mpbuffer_map[*it] = boost::make_shared<MPBuffer>(*it, max_num_key_ver_in_mpbuffer, malgo_t,
                                                            w_prefetch, boost::bind(&MWASpace::handle_mpbuffer_data_act, this, _1, _2, _3) );
  }
  // 
  LOG(INFO) << "MWASpace:: constructed; " << to_str();
}

MWASpace::~MWASpace()
{
  LOG(INFO) << "MWASpace:: destructed; \n" << to_str_end();
}

std::string MWASpace::to_str()
{
  std::stringstream ss;
  ss << "WASpace::to_str= \n" << WASpace::to_str() << "\n"
     << "max_num_key_ver_in_mpbuffer= " << max_num_key_ver_in_mpbuffer << "\n"
     << "malgo_t= " << malgo_t << "\n"
     << "w_prefetch= " << w_prefetch << "\n";
  
  return ss.str();
}

std::string MWASpace::to_str_end()
{
  std::stringstream ss;
  ss << "kv_v= ";
  for (std::vector<key_ver_pair>::iterator it = kv_v.begin(); it != kv_v.end(); it++)
    ss << KV_TO_STR(it->first, it->second) << ", ";
  ss << "\n";
  
  ss << "ds_id__kv_map= \n";
  for (std::map<int, boost::shared_ptr<patch_all::thread_safe_vector<key_ver_pair> > >::iterator map_it = ds_id__kv_map.begin(); map_it != ds_id__kv_map.end(); map_it++) {
    ss << ">>> ds_id= " << map_it->first << " : ";
    for (std::vector<key_ver_pair>::iterator vec_it = (map_it->second)->begin(); vec_it != (map_it->second)->end(); vec_it++)
      ss << "\t <" << vec_it->first << ", " << vec_it->second << "> \n ";
    ss << "\n";
  }
  
  ss << "ds_id__mpbuffer_map= \n";
  for (std::map<int, boost::shared_ptr<MPBuffer> >::iterator it = ds_id__mpbuffer_map.begin(); it != ds_id__mpbuffer_map.end(); it++) {
    ss << ">>> ds_id= " << it->first << "\n"
       << "\t mpbuffer= \n" << (it->second)->to_str() << "\n";
  }
  
  for (std::vector<int>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
    patch_all::thread_safe_vector<key_ver_pair>& ds_kv_v = *ds_id__kv_map[*it];
    std::sort(ds_kv_v.begin(), ds_kv_v.end() );
    
    std::vector<key_ver_pair> mpbuffer_kv_v = ds_id__mpbuffer_map[*it]->get_kv_v();
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

int MWASpace::reg_ds(int ds_id)
{
  if (WASpace::reg_ds(ds_id) ) {
    LOG(ERROR) << "reg_ds:: WASpace::reg_ds failed; ds_id= " << ds_id;
    return 1;
  }
  ds_id__kv_map[ds_id] = boost::make_shared<patch_all::thread_safe_vector<key_ver_pair> >();
  ds_id__mpbuffer_map[ds_id] = boost::make_shared<MPBuffer>(ds_id, max_num_key_ver_in_mpbuffer, malgo_t,
                                                            w_prefetch, boost::bind(&MWASpace::handle_mpbuffer_data_act, this, _1, _2, _3) );
  // Note: DS joins dynamically, its MPBuffer needs to be updated
  for (std::map<key_ver_pair, int>::iterator it = kv__p_id_map.begin(); it != kv__p_id_map.end(); it++)
    ds_id__mpbuffer_map[ds_id]->reg_key_ver(it->second, it->first);
  
  LOG(INFO) << "reg_ds:: reged ds_id= " << ds_id;
  
  return 0;
}

int MWASpace::del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!kv_v.contains(kv) ) {
    LOG(ERROR) << "del:: does not contain; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  ds_id__kv_map[ds_id]->del(kv);
  ds_id__mpbuffer_map[ds_id]->del(kv);
  
  kv__p_id_map.del(kv);
  patch_all::free_all<COOR_T>(2, kv__lucoor_map[kv].first, kv__lucoor_map[kv].second);
  kv__lucoor_map.del(kv);
  kv_v.del(kv);
  
  return 0;
}

int MWASpace::put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id)
{
  // if (!p_id__key_map.contains(p_id) ) {
  //   std::vector<std::string> key_v;
  //   p_id__key_map[p_id] = key_v;
  // }
  // p_id__key_map[p_id].push_back(key);
  // 
  key_ver_pair kv = std::make_pair(key, ver);
  if (ds_id != -1 && contains(ds_id, kv) ) {
    LOG(ERROR) << "put:: already in ds_id= " << ds_id << "; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  else if (ds_id == -1 && contains('*', kv) ) {
    LOG(ERROR) << "put:: already in space; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  if (ds_id == -1) { // New data put
    if (!app_id__ds_id_map.contains(p_id) )
      return 1;
    ds_id = app_id__ds_id_map[p_id];
    
    // Immediately broadcast it to every ds peer so mpbuffers can be updated
    for (std::map<int, boost::shared_ptr<MPBuffer> >::iterator it = ds_id__mpbuffer_map.begin(); it != ds_id__mpbuffer_map.end(); it++)
      (it->second)->reg_key_ver(p_id, kv);
    
    kv__p_id_map[kv] = p_id;
    COOR_T* _lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    COOR_T* _ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    memcpy(_lcoor_, lcoor_, NDIM*sizeof(COOR_T) );
    memcpy(_ucoor_, ucoor_, NDIM*sizeof(COOR_T) );
    kv__lucoor_map[kv] = std::make_pair(_lcoor_, _ucoor_);
    kv_v.push_back(kv);
  }
  ds_id__kv_map[ds_id]->push_back(kv);
  
  return 0;
}

int MWASpace::query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<int>& ds_id_v)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!contains('*', kv) )
    return 1;
  
  for (std::vector<int>::iterator it = this->ds_id_v.begin(); it != this->ds_id_v.end(); it++) {
    if (contains(*it, kv) )
      ds_id_v.push_back(*it);
  }
  // ds_id_v.push_back(app_id__ds_id_map[kv__p_id_map[kv] ] );
  
  return 0;
}

int MWASpace::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
  return ds_id__mpbuffer_map[app_id__ds_id_map[c_id] ]->add_access(std::make_pair(key, ver) );
}

bool MWASpace::contains(int ds_id, key_ver_pair kv)
{
  if (ds_id == '*')
    return kv_v.contains(kv);
  else
    return ds_id__kv_map[ds_id]->contains(kv) || ds_id__mpbuffer_map[ds_id]->contains(kv);
}

void MWASpace::handle_mpbuffer_data_act(PREFETCH_DATA_ACT_T data_act_t, int ds_id, key_ver_pair kv) {
  if (handle_data_act_cb != 0) {
    LOG(INFO) << "handle_mpbuffer_data_act:: data_act_t= " << data_act_t << ", ds_id= " << ds_id
              << ", " << KV_LUCOOR_TO_STR(kv.first, kv.second, kv__lucoor_map[kv].first, kv__lucoor_map[kv].second);
    handle_data_act_cb(data_act_t, ds_id, kv, kv__lucoor_map[kv] );
  }
}

/******************************************  SWASpace  ********************************************/
SWASpace::SWASpace(std::vector<int> ds_id_v,
                   SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb)
: WASpace(ds_id_v, handle_data_act_cb),
  w_prefetch(w_prefetch),
  qtable_(boost::make_shared<RTable<int> >() )
{
  if (salgo_t == SALGO_H)
    salgo_ = boost::make_shared<HSAlgo>(lcoor_, ucoor_, sexpand_length);
  else {
    LOG(ERROR) << "SWASpace:: unknown salgo_t= " << salgo_t;
    exit(1);
  }
  // 
  LOG(INFO) << "SWASpace:: constructed; " << to_str();
}

std::string SWASpace::to_str()
{
  std::stringstream ss;
  ss << "WASpace::to_str= \n" << WASpace::to_str() << "\n"
     << "salgo= \n" << salgo_->to_str() << "\n";
    // << "qtable= \n" << qtable_->to_str() << "\n";
  
  return ss.str();
}

int SWASpace::put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id)
{
  if (p_id != NULL_P_ID) {
    if (!app_id__ds_id_map.contains(p_id) ) {
      LOG(ERROR) << "put:: non-reged p_id= " << p_id;
      return 1;
    }
  }
  
  if (ds_id == -1) // New data put
    ds_id = app_id__ds_id_map[p_id];
  
  if (qtable_->add(key, ver, lcoor_, ucoor_, ds_id) ) {
    LOG(ERROR) << "put:: qtable_->add failed! for " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_) << "\n";
    return 1;
  }
  
  return 0;
}

int SWASpace::del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id)
{
  std::vector<int> ds_id_v;
  if (query(key, ver, lcoor_, ucoor_, ds_id_v) ) {
    LOG(ERROR) << "del:: does not contain; " << KV_LUCOOR_TO_STR(key, ver, lcoor_, ucoor_);
    return 1;
  }
  
  return qtable_->del(key, ver, lcoor_, ucoor_);
}

int SWASpace::query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<int>& ds_id_v)
{ 
  return qtable_->query(key, ver, lcoor_, ucoor_, ds_id_v);
}

int SWASpace::get_to_fetch(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<lcoor_ucoor_pair>& lucoor_to_fetch_v)
{
  return salgo_->get_to_fetch(lcoor_, ucoor_, lucoor_to_fetch_v);
}

int SWASpace::add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_)
{
//   IS_VALID_BOX("add_access", lcoor_, ucoor_, return 1)
  
//   salgo_->add_access(lcoor_, ucoor_);
  
//   int ds_id = app_id__ds_id_map[c_id];
//   if (w_prefetch) {
//     std::vector<lcoor_ucoor_pair> lucoor_to_fetch_v;
//     salgo_->get_to_fetch(lcoor_, ucoor_, lucoor_to_fetch_v);
    
//     for (std::vector<lcoor_ucoor_pair>::iterator it = lucoor_to_fetch_v.begin(); it != lucoor_to_fetch_v.end(); it++)
//         handle_data_act_cb(PREFETCH_DATA_ACT_PREFETCH, ds_id, std::make_pair(key, ver), *it);
//   }
  
//   return 0;
}

