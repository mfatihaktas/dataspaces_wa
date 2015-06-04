#include "prefetch.h"

PBuffer::PBuffer(char ds_id, int buffer_size, PREFETCH_T prefetch_t,
                 bool w_prefetch, func_handle_prefetch_cb _handle_prefetch_cb,
                 func_handle_del_cb _handle_del_cb )
: ds_id(ds_id), buffer_size(buffer_size),
  w_prefetch(w_prefetch),
  _handle_prefetch_cb(_handle_prefetch_cb),
  _handle_del_cb(_handle_del_cb),
  cache(buffer_size, _handle_del_cb)
{
  if (prefetch_t == W_LZ)
    palgo_to_pick_app_ = boost::make_shared<LZAlgo>();
  else if (prefetch_t == W_ALZ)
    palgo_to_pick_app_ = boost::make_shared<ALZAlgo>();
  else if (prefetch_t == W_PPM)
    palgo_to_pick_app_ = boost::make_shared<PPMAlgo>(2);
  else if (prefetch_t == W_PO)
    palgo_to_pick_app_ = boost::make_shared<POAlgo>();
  
  // std::map<PREFETCH_T, float> prefetch_t__weight_map;
  // prefetch_t__weight_map[W_LZ] = 0.5;
  // prefetch_t__weight_map[W_PPM] = 0.5;
  // prefetch_t__weight_map[W_PO] = 0.33;
  // palgo_to_pick_app_ = boost::make_shared<MPrefetchAlgo>(prefetch_t, prefetch_t__weight_map);
  
  // 
  LOG(INFO) << "PBuffer:: constructed.";
}

PBuffer::~PBuffer() { LOG(INFO) << "PBuffer:: destructed."; }

std::string PBuffer::to_str()
{
  std::stringstream ss;
  ss << "w_prefetch= " << boost::lexical_cast<std::string>(w_prefetch) << "\n";
  ss << "buffer_size= " << boost::lexical_cast<std::string>(buffer_size) << "\n";
  // ss << "reged_key_ver_v= \n";
  // for (std::vector<key_ver_pair>::iterator it = reged_key_ver_v.begin();
  //     it != reged_key_ver_v.end(); it++) {
  //   ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << ">\n";
  // }
  
  // ss << "acced_key_ver_v= \n";
  // for (std::vector<key_ver_pair>::iterator it = acced_key_ver_v.begin();
  //     it != acced_key_ver_v.end(); it++) {
  //   ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << ">\n";
  // }
  
  // ss << "p_id__acced_key_ver_v_map= \n";
  // for (std::map<int, std::vector<key_ver_pair> >::iterator map_it = p_id__acced_key_ver_v_map.begin();
  //     map_it != p_id__acced_key_ver_v_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::vector<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  // ss << "p_id__reged_key_ver_deq_map= \n";
  // for (std::map<int, std::deque<key_ver_pair> >::iterator map_it = p_id__reged_key_ver_deq_map.begin(); 
  //     map_it != p_id__reged_key_ver_deq_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::deque<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  ss << "cache= \n" << cache.to_str() << "\n";
  
  // ss << "palgo_to_pick_app_= \n"
  //   << "\t parse_tree_to_pstr= \n" << palgo_to_pick_app_->parse_tree_to_pstr() << "\n";
  
  return ss.str();
}

// ****************************************  state rel  ***************************************** //
int PBuffer::reg_key_ver(int p_id, key_ver_pair kv)
{
  if (reged_key_ver_v.contains(kv) ) {
    LOG(ERROR) << "reg_key_ver:: already registered! <key= " << kv.first << ", ver= " << kv.second << ">.";
    return 1;
  }
  reged_key_ver_v.push_back(kv);
  // 
  if (p_id__reged_key_ver_deq_map.contains(p_id) )
    p_id__reged_key_ver_deq_map[p_id].push_back(kv);
  else {
    std::deque<key_ver_pair> key_ver_deq;
    key_ver_deq.push_back(kv);
    p_id__reged_key_ver_deq_map[p_id] = key_ver_deq;
    p_id__front_step_in_deq_map[p_id] = 0;
  }
}

// ****************************************  operational  *************************************** //
void PBuffer::sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> key_ver_v, 
                                    float& hit_rate, std::vector<char>& accuracy_v)
{
  int num_miss = 0;
  
  std::vector<int>::iterator pid_it;
  std::vector<key_ver_pair>::iterator kv_it;
  for (pid_it = p_id_v.begin() , kv_it = key_ver_v.begin(); pid_it != p_id_v.end(), kv_it != key_ver_v.end(); pid_it++, kv_it++)
    reg_key_ver(*pid_it, *kv_it);
  
  for (pid_it = p_id_v.begin() , kv_it = key_ver_v.begin(); pid_it != p_id_v.end(), kv_it != key_ver_v.end(); pid_it++, kv_it++) {
    // std::cout << "sim_prefetch_accuracy:: is <" << kv_it->first << ", " << kv_it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    if (!cache.contains(*kv_it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    add_access(*pid_it, *kv_it);
  }
  
  hit_rate = 1.0 - (float)num_miss / key_ver_v.size();
}

int PBuffer::add_access(int p_id, key_ver_pair kv)
{
  if (!reged_key_ver_v.contains(kv) ) {
    LOG(WARNING) << "add_access:: non-registered <key= " << kv.first << ", ver= " << kv.second << ">.";
    return 1;
  }
  
  if (!cache.del(p_id, kv) )
    _handle_del_cb(kv);
  // 
  // acced_key_ver_v.push_back(kv);
  // p_id__acced_key_ver_v_map[p_id].push_back(kv);
  
  if (!p_id__last_acced_step_map.contains(p_id) )
    p_id__last_acced_step_map[p_id] = 0;
  else
    p_id__last_acced_step_map[p_id] += 1;
  // 
  int num_app = 1;
  std::vector<key_ver_pair> key_ver_v;
  {// Causes problems while building the parse tree for multi-threaded scenario
    boost::lock_guard<boost::mutex> guard(add_acc_mutex);
    
    if (palgo_to_pick_app_->add_access(p_id) )
      LOG(ERROR) << "add_access:: palgo_to_pick_app_->add_access failed for p_id= " << p_id;
  
    get_to_prefetch(num_app, key_ver_v);
    // LOG(INFO) << "add_access:: get_to_prefetch returned key_ver_v= " << patch_pre::pvec_to_str<key_ver_pair>(key_ver_v) << "\n";
    // To avoid remote fetching while a kv is being prefetched -- assuming prefetching will never fail
    for (std::vector<key_ver_pair>::iterator it = key_ver_v.begin(); it != key_ver_v.end(); it++)
      cache.push(p_id, *it);
  }
  
  // Call for prefetching per access
  if (w_prefetch) {
    for (std::vector<key_ver_pair>::iterator it = key_ver_v.begin(); it != key_ver_v.end(); it++)
      _handle_prefetch_cb(ds_id, *it);
  }
    
  return 0;
}

int PBuffer::get_to_prefetch(int& num_app, std::vector<key_ver_pair>& key_ver_v)
{
  // Pick app
  std::vector<ACC_T> p_id_v, ep_id_v;
  palgo_to_pick_app_->get_to_prefetch(num_app, p_id_v, cache.get_cached_acc_v(), ep_id_v);
  // std::cout << ">>> palgo_to_pick_app_->get_to_prefetch returned: \n"
  //           << "p_id_v= " << patch_pre::vec_to_str<ACC_T>(p_id_v) << "\n"
  //           << "ep_id_v= " << patch_pre::vec_to_str<ACC_T>(ep_id_v) << "\n";
  
  // int i;
  std::vector<ACC_T>::iterator jt;
  // for (jt = ep_id_v.begin(), i = 0; i < buffer_size - num_app - cache.size(), jt != ep_id_v.end(); i++, jt++)
  for (jt = ep_id_v.begin(); jt != ep_id_v.end(); jt++)
    p_id_v.push_back(*jt);
  
  // std::cout << "----------------------------------------\n";
  // std::cout << "p_id_v= " << patch_pre::vec_to_str<ACC_T>(p_id_v) << "\n";
  // 
  for (std::vector<ACC_T>::iterator it = p_id_v.begin(); it != p_id_v.end(); it++) {
    if (!p_id__last_cached_step_map.contains(*it) )
      p_id__last_cached_step_map[*it] = -1;

    // std::cout << "for p_id= " << *it << ":\n"
    //           << "\t p_id__last_cached_step_map= " << p_id__last_cached_step_map[*it] << "\n"
    //           << "\t p_id__last_acced_step_map= " << p_id__last_acced_step_map[*it] << "\n";
    if (p_id__last_cached_step_map[*it] < p_id__last_acced_step_map[*it] ) {
      p_id__last_cached_step_map[*it] = p_id__last_acced_step_map[*it];
    }
    else if (p_id__last_cached_step_map[*it] > p_id__last_acced_step_map[*it] )
      continue;
    // 
    ACC_T step_to_prefetch = p_id__last_cached_step_map[*it] + 1;
    // std::cout << "step_to_prefetch= " << step_to_prefetch << "\n";
    std::deque<key_ver_pair>& key_ver_deq = p_id__reged_key_ver_deq_map[*it];
    // std::cout << "p_id = " << *it << ", key_ver_deq= \n";
    // for (std::deque<key_ver_pair>::iterator dit = key_ver_deq.begin(); dit != key_ver_deq.end(); dit++)
    //   std::cout << "\t <" << dit->first << "," << dit->second << "> \n";
    
    ACC_T front_step_in_deq = p_id__front_step_in_deq_map[*it];
    // std::cout << "front_step_in_deq= " << front_step_in_deq << "\n";
    while (!key_ver_deq.empty() && front_step_in_deq < step_to_prefetch) {
      key_ver_deq.pop_front();
      front_step_in_deq++;
    }
    
    if (key_ver_deq.empty() )
      continue;
    
    key_ver_pair kv = key_ver_deq.front();
    // LOG(INFO) << "will key_ver_v.push_back <" << kv.first << ", " << kv.second << ">";
    key_ver_v.push_back(key_ver_deq.front() );
    key_ver_deq.pop_front();
    front_step_in_deq++;
    
    p_id__last_cached_step_map[*it] = step_to_prefetch;
    p_id__front_step_in_deq_map[*it] = front_step_in_deq;
  }
  // std::cout << "***** \n";
}

bool PBuffer::contains(key_ver_pair kv)
{
  return cache.contains(kv);
}

std::vector<key_ver_pair> PBuffer::get_content_v()
{
  return cache.get_content_v();
}
