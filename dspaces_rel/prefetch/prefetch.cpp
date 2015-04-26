#include "prefetch.h"

PBuffer::PBuffer(bool with_prefetch, size_t buffer_size, func_handle_prefetch_cb _handle_prefetch_cb, 
                 size_t app_context_size )
: with_prefetch(with_prefetch),
  buffer_size(buffer_size),
  _handle_prefetch_cb(_handle_prefetch_cb),
  app_context_size(app_context_size),
  cache(buffer_size),
  ppm_algo_to_pick_app(app_context_size)
{
  // 
  LOG(INFO) << "PBuffer:: constructed.";
}

PBuffer::~PBuffer() { LOG(INFO) << "PBuffer:: destructed."; }

std::string PBuffer::to_str()
{
  std::stringstream ss;
  ss << "with_prefetch= " << boost::lexical_cast<std::string>(with_prefetch) << "\n";
  ss << "buffer_size= " << boost::lexical_cast<std::string>(buffer_size) << "\n";
  
  ss << "reged_key_ver_vec= \n";
  for (std::vector<key_ver_pair>::iterator it = reged_key_ver_vec.begin();
       it != reged_key_ver_vec.end(); it++) {
    ss << "\t<" << it->first << "," << it->second << ">\n";
  }
  
  ss << "acced_key_ver_vec= \n";
  for (std::vector<key_ver_pair>::iterator it = acced_key_ver_vec.begin();
       it != acced_key_ver_vec.end(); it++) {
    ss << "\t<" << it->first << "," << it->second << ">\n";
  }
  
  ss << "app_id__acced_key_ver_vec_map= \n";
  for (std::map<int, std::vector<key_ver_pair> >::iterator map_it = app_id__acced_key_ver_vec_map.begin(); 
       map_it != app_id__acced_key_ver_vec_map.end(); map_it++) {
    ss << "\t app_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
    for (std::vector<key_ver_pair>::iterator kv_it = (map_it->second).begin();
         kv_it != (map_it->second).end(); kv_it++) {
      ss << "\t\t<" << boost::lexical_cast<std::string>(kv_it->first) << "," << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
    }
  }
  
  ss << "app_id__key_ver_deq_map= \n";
  for (std::map<int, std::deque<key_ver_pair> >::iterator map_it = app_id__key_ver_deq_map.begin(); 
       map_it != app_id__key_ver_deq_map.end(); map_it++) {
    ss << "\t app_id= " << boost::lexical_cast<std::string>(map_it->first) << ":\n";
    for (std::deque<key_ver_pair>::iterator kv_it = (map_it->second).begin();
         kv_it != (map_it->second).end(); kv_it++) {
      ss << "\t\t<" << boost::lexical_cast<std::string>(kv_it->first) << "," << boost::lexical_cast<std::string>(kv_it->second) << ">\n";
    }
  }
  
  ss << "cache= \n" << cache.to_str();
  ss << "\n";
  
  return ss.str();
}

// ****************************************  state rel  ***************************************** //
int PBuffer::reg_key_ver(std::string key, unsigned int ver, int app_id)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (reged_key_ver_vec.contains(kv) ) {
    LOG(ERROR) << "reg_key_ver:: already registered! <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  reged_key_ver_vec.push_back(kv);
  
  if (app_id__key_ver_deq_map.contains(app_id) ) {
    app_id__key_ver_deq_map[app_id].push_back(kv);
  }
  else {
    std::deque<key_ver_pair> key_ver_deq;
    key_ver_deq.push_back(kv);
    app_id__key_ver_deq_map[app_id] = key_ver_deq;
  }
}

// ****************************************  operational  *************************************** //
int PBuffer::add_access(std::string key, unsigned int ver, int app_id)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!reged_key_ver_vec.contains(kv) ) {
    LOG(WARNING) << "add_access:: non-registered <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  
  ppm_algo_to_pick_app.add_access(app_id);
  
  acced_key_ver_vec.push_back(kv);
  app_id__acced_key_ver_vec_map[app_id].push_back(kv);
  // Update app data FIFO queue before giving prefetching decision
  std::deque<key_ver_pair>& key_ver_deq = app_id__key_ver_deq_map[app_id];
  for (std::deque<key_ver_pair>::iterator it = key_ver_deq.begin(); it != key_ver_deq.end(); it++) {
    if (*it == kv) {
      while (key_ver_deq.front() != kv)
        key_ver_deq.pop_front();
    }
  }
  
  // Call for prefetching per access
  // boost::thread(&PBuffer::prefetch, this);
  prefetch();
  
  return 0;
}

int PBuffer::prefetch()
{
  size_t num_apps = 1;
  std::vector<key_ver_pair> key_ver_vec;
  get_to_prefetch(num_apps, key_ver_vec);
  
  for (std::vector<key_ver_pair>::iterator it = key_ver_vec.begin(); it != key_ver_vec.end(); it++) {
    std::map<std::string, std::string> handle_prefetch_map;
    handle_prefetch_map["key"] = it->first;
    handle_prefetch_map["ver"] = boost::lexical_cast<std::string>(it->second);
    
    if (with_prefetch)
      _handle_prefetch_cb(handle_prefetch_map);
    
    push(it->first, it->second);
  }
}

int PBuffer::get_to_prefetch(size_t& num_apps, std::vector<key_ver_pair>& key_ver_vec)
{
  // Pick app
  KEY_T* app_ids_;
  ppm_algo_to_pick_app.get_to_prefetch(num_apps, app_ids_);
  // Space acts as FIFO queue for data flow between p-c
  for (int i = 0; i < num_apps; i++) {
    std::deque<key_ver_pair>& key_ver_deq = app_id__key_ver_deq_map[app_ids_[i] ];
    key_ver_vec.push_back(key_ver_deq.front() );
    key_ver_deq.pop_front();
  }
  free(app_ids_);
}

int PBuffer::push(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!reged_key_ver_vec.contains(kv) ) {
    LOG(ERROR) << "push:: reged_key_ver_vec does not contain <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  if (cache.push(kv) ) {
    LOG(ERROR) << "push:: cache.push failed for <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  
  return 0;
}

bool PBuffer::contains(std::string key, unsigned int ver)
{
  return cache.contains(std::make_pair(key, ver) );
}

