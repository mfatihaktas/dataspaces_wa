#include "prefetch.h"

PBuffer::PBuffer(bool with_prefetch, size_t buffer_size, func_handle_prefetch_cb _handle_prefetch_cb, 
                 char* alphabet_, size_t alphabet_size, size_t context_size)
: with_prefetch(with_prefetch),
  buffer_size(buffer_size),
  _handle_prefetch_cb(_handle_prefetch_cb),
  alphabet_(alphabet_),
  alphabet_size(alphabet_size),
  cache(buffer_size),
  palgo(alphabet_size, alphabet_, context_size)
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
  ss << "alphabet_size= " << boost::lexical_cast<std::string>(alphabet_size) << "\n";
  ss << "alphabet_= ";
  for (int i = 0; i < alphabet_size; i++) {
    // LOG(INFO) << "to_str:: alphabet_[" << i << "]= " << alphabet_[i] << "\n";
    ss << boost::lexical_cast<std::string>(alphabet_[i]) << ", ";
  }
  ss << "\n";
  
  ss << "key_ver__pkey_pver_map= \n";
  for (std::map<key_ver_pair, pkey_pver_pair>::iterator it = key_ver__pkey_pver_map.begin(); 
       it != key_ver__pkey_pver_map.end(); it++) {
    ss << "\t<" << (it->first).first << "," << (it->first).second << "> : "
       << "<" << (it->second).first << "," << (it->second).second << ">\n";
  }
  
  ss << "accessed_key_ver_vector= \n";
  for (std::vector<key_ver_pair>::iterator it = accessed_key_ver_vector.begin();
       it != accessed_key_ver_vector.end(); it++) {
    ss << "\t<" << it->first << "," << it->second << ">\n";
  }
  
  ss << "buffered_key_ver_vector= \n";
  for (std::vector<key_ver_pair>::iterator it = buffered_key_ver_vector.begin();
       it != buffered_key_ver_vector.end(); it++) {
    ss << "\t<" << it->first << "," << it->second << ">\n";
  }
  
  ss << "cache= \n" << cache.to_str();
  ss << "\n";
  
  return ss.str();
}

// ****************************************  state rel  ***************************************** //
int PBuffer::reg_key_ver__pkey_pver_pair(int app_id, std::string key, unsigned int ver)
{
  // TODO: for every <key, ver> advertised one of the peer dataspaces is registered with pkey by 
  // walking on the alphabet_ with incremental pver
  key_ver_pair kv = std::make_pair(key, ver);
  if (key_ver__pkey_pver_map.contains(kv) ) {
    LOG(ERROR) << "reg_key_ver__pkey_pver_pair:: already registered <key= " << key << ", ver= " << ver << ">!";
    return 1;
  }
  
  int size = key_ver__pkey_pver_map.size();
  pkey_pver_pair pkpv = std::make_pair(alphabet_[size % alphabet_size], size/alphabet_size);
  key_ver__pkey_pver_map[kv] = pkpv;
  pkey_pver__key_ver_map[pkpv] = kv;
}

// ****************************************  operational  *************************************** //
int PBuffer::add_access(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!key_ver__pkey_pver_map.contains(kv) ) {
    LOG(WARNING) << "add_access:: non-registered <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  palgo.add_access(key_ver__pkey_pver_map[kv].first );
  accessed_key_ver_vector.push_back(kv);
  // Call for prefetching per access
  boost::thread(&PBuffer::prefetch, this);
  // prefetch();
  
  return 0;
}

int PBuffer::prefetch()
{
  size_t num_keys = 1;
  std::vector<key_ver_pair> key_ver_vector;
  get_to_prefetch(num_keys, key_ver_vector);
  
  for (std::vector<key_ver_pair>::iterator it = key_ver_vector.begin(); it != key_ver_vector.end(); it++) {
    std::map<std::string, std::string> handle_prefetch_map;
    handle_prefetch_map["key"] = it->first;
    handle_prefetch_map["ver"] = boost::lexical_cast<std::string>(it->second);
    
    if (with_prefetch)
      _handle_prefetch_cb(handle_prefetch_map);
      
    push(it->first, it->second);
  }
}

int PBuffer::get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector)
{
  unsigned int pver = accessed_key_ver_vector.size() / alphabet_size;
  char* pkeys_;
  palgo.get_to_prefetch(num_keys, pkeys_);
  for (int i = 0; i < num_keys; i++) {
    pkey_pver_pair pkpv = std::make_pair(pkeys_[i], pver);
    if (!pkey_pver__key_ver_map.contains(pkpv) ) {
      LOG(WARNING) << "get_to_prefetch:: palgo.get_to_prefetch returned not registered!;" 
                   << "<pkey= " << pkeys_[i] << ", " << "pver= " << pver << ">.";
    }
    else {
      key_ver_vector.push_back(pkey_pver__key_ver_map[pkpv] );
    }
  }
  free(pkeys_);
}

int PBuffer::push(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!key_ver__pkey_pver_map.contains(kv) ) {
    LOG(ERROR) << "push:: key_ver__pkey_pver_map does not contain <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  if (cache.push(key_ver__pkey_pver_map[kv] ) ) {
    LOG(ERROR) << "push:: cache.push failed for <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  buffered_key_ver_vector.push_back(kv);
  
  return 0;
}

bool PBuffer::contains(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  return cache.contains(key_ver__pkey_pver_map[kv] );
}

