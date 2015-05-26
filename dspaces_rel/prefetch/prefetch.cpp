#include "prefetch.h"

PBuffer::PBuffer(char ds_id, size_t buffer_size, size_t app_context_size,
                 bool with_prefetch, func_handle_prefetch_cb _handle_prefetch_cb,
                 func_handle_del_cb _handle_del_cb )
: ds_id(ds_id), buffer_size(buffer_size),
  app_context_size(app_context_size),
  with_prefetch(with_prefetch),
  _handle_prefetch_cb(_handle_prefetch_cb),
  _handle_del_cb(_handle_del_cb),
  cache(buffer_size)
{
  if (app_context_size) 
    algo_to_pick_app_ = boost::make_shared<PPMAlgo>(app_context_size);
  else // LZAlgo
    algo_to_pick_app_ = boost::make_shared<LZAlgo>();
  
  // 
  LOG(INFO) << "PBuffer:: constructed.";
}

PBuffer::~PBuffer() { LOG(INFO) << "PBuffer:: destructed."; }

std::string PBuffer::to_str()
{
  std::stringstream ss;
  ss << "with_prefetch= " << boost::lexical_cast<std::string>(with_prefetch) << "\n";
  ss << "buffer_size= " << boost::lexical_cast<std::string>(buffer_size) << "\n";
  ss << "app_context_size= " << boost::lexical_cast<std::string>(app_context_size) << "\n";
  // ss << "reged_key_ver_vec= \n";
  // for (std::vector<key_ver_pair>::iterator it = reged_key_ver_vec.begin();
  //     it != reged_key_ver_vec.end(); it++) {
  //   ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << ">\n";
  // }
  
  // ss << "acced_key_ver_vec= \n";
  // for (std::vector<key_ver_pair>::iterator it = acced_key_ver_vec.begin();
  //     it != acced_key_ver_vec.end(); it++) {
  //   ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << ">\n";
  // }
  
  // ss << "p_id__acced_key_ver_vec_map= \n";
  // for (std::map<int, std::vector<key_ver_pair> >::iterator map_it = p_id__acced_key_ver_vec_map.begin(); 
  //     map_it != p_id__acced_key_ver_vec_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::vector<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  // ss << "p_id__key_ver_deq_map= \n";
  // for (std::map<int, std::deque<key_ver_pair> >::iterator map_it = p_id__key_ver_deq_map.begin(); 
  //     map_it != p_id__key_ver_deq_map.end(); map_it++) {
  //   ss << "\t p_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
  //   for (std::deque<key_ver_pair>::iterator kv_it = (map_it->second).begin();
  //       kv_it != (map_it->second).end(); kv_it++) {
  //     ss << "\t\t <" << boost::lexical_cast<std::string>(kv_it->first) << ", " << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
  //   }
  // }
  
  ss << "cache= \n" << cache.to_str() << "\n";
  
  ss << "algo_to_pick_app_= \n"
     << "\t parse_tree_to_pstr= \n" << algo_to_pick_app_->parse_tree_to_pstr() << "\n";
  
  return ss.str();
}

// ****************************************  state rel  ***************************************** //
int PBuffer::reg_key_ver(int p_id, key_ver_pair kv)
{
  if (reged_key_ver_vec.contains(kv) ) {
    LOG(ERROR) << "reg_key_ver:: already registered! <key= " << kv.first << ", ver= " << kv.second << ">.";
    return 1;
  }
  reged_key_ver_vec.push_back(kv);
  // 
  if (p_id__key_ver_deq_map.contains(p_id) ) {
    p_id__key_ver_deq_map[p_id].push_back(kv);
  }
  else {
    std::deque<key_ver_pair> key_ver_deq;
    key_ver_deq.push_back(kv);
    p_id__key_ver_deq_map[p_id] = key_ver_deq;
  }
}

// ****************************************  operational  *************************************** //
int PBuffer::add_access(int p_id, key_ver_pair kv)
{
  if (!reged_key_ver_vec.contains(kv) ) {
    LOG(WARNING) << "add_access:: non-registered <key= " << kv.first << ", ver= " << kv.second << ">.";
    return 1;
  }
  
  if (!cache.del(kv) )
    _handle_del_cb(kv);
  
  
  size_t num_app = 1;
  std::vector<key_ver_pair> key_ver_vec;
  {// Causes problems while building the parse tree for multi-threaded scenario
    boost::lock_guard<boost::mutex> guard(add_acc_mutex);
    
    if (algo_to_pick_app_->add_access(p_id) ) {
      LOG(INFO) << "add_access:: algo_to_pick_app_->add_access failed for p_id= " << p_id;
    }
  
    acced_key_ver_vec.push_back(kv);
    p_id__acced_key_ver_vec_map[p_id].push_back(kv);
    // Update app data FIFO queue before giving prefetching decision
    std::deque<key_ver_pair>& key_ver_deq = p_id__key_ver_deq_map[p_id];
    for (std::deque<key_ver_pair>::iterator it = key_ver_deq.begin(); it != key_ver_deq.end(); it++) {
      if (*it == kv) {
        while (key_ver_deq.front() != kv)
          key_ver_deq.pop_front();
        key_ver_deq.pop_front();
      }
    }
    
    get_to_prefetch(num_app, key_ver_vec);
    // To avoid remote fetching while a kv is being prefetched -- assuming prefetching will never fail
    for (std::vector<key_ver_pair>::iterator it = key_ver_vec.begin(); it != key_ver_vec.end(); it++)
      cache.push(*it);
  }
  
  // Call for prefetching per access
  if (with_prefetch) {
    for (std::vector<key_ver_pair>::iterator it = key_ver_vec.begin(); it != key_ver_vec.end(); it++)
      _handle_prefetch_cb(ds_id, *it);
  }
    
  return 0;
}

int PBuffer::get_to_prefetch(size_t& num_app, std::vector<key_ver_pair>& key_ver_vec)
{
  // Pick app
  KEY_T* p_id_;
  std::vector<KEY_T> ep_id_v;
  algo_to_pick_app_->get_to_prefetch(num_app, p_id_, std::vector<KEY_T>(), ep_id_v);
  // Space acts as FIFO queue for data flow between p-c
  // Note: Here we chose not to fill remaning apps (_num_app - num_app) to let the palgo to decide about
  // 'best' number of keys to prefetch
  if (num_app == 0) { // Pick the app that made the last access
    num_app = 1;
    p_id_ = (KEY_T*)malloc(num_app*sizeof(KEY_T) );
    p_id_[0] = algo_to_pick_app_->get_acc_v().back();
    
    // LOG(INFO) << "get_to_prefetch:: num_app == 0; p_id_= " << p_id_[0];
  }
  
  for (int i = 0; i < num_app; i++) {
    std::deque<key_ver_pair>& key_ver_deq = p_id__key_ver_deq_map[p_id_[i] ];
    if (key_ver_deq.empty() )
      break;
    
    key_ver_vec.push_back(key_ver_deq.front() );
    key_ver_deq.pop_front();
  }
  free(p_id_);
}

bool PBuffer::contains(key_ver_pair kv)
{
  return cache.contains(kv);
}

std::vector<key_ver_pair> PBuffer::get_content_vec()
{
  return cache.get_content_v();
}
