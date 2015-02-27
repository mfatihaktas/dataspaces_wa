#include "prefetch.h"

PBuffer::PBuffer(size_t buffer_size, func_handle_prefetch_cb _handle_prefetch_cb, 
                 char* alphabet_, size_t aphabet_size, size_t context_size)
: _handle_prefetch_cb(_handle_prefetch_cb),
  alphabet_(alphabet_),
  alphabet_size(alphabet_size),
  buffer_size(buffer_size),
  cache(buffer_size),
  palgo(aphabet_size, alphabet_, context_size)
{
  LOG(INFO) << "PBuffer:: constructed.";
}

PBuffer::~PBuffer() { LOG(INFO) << "PBuffer:: destructed."; }

// ****************************************  state rel  ***************************************** //
int PBuffer::reg_key_ver__pkey_pver_pair(std::string key, unsigned int ver)
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
  size_t num_keys = buffer_size - cache.size();
  std::vector<key_ver_pair> key_ver_vector;
  get_to_prefetch(num_keys, key_ver_vector);
  
  boost::thread(&PBuffer::prefetch, this, key_ver_vector);
  return 0;
}

int PBuffer::push(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  if (!key_ver__pkey_pver_map.contains(kv) ) {
    LOG(ERROR) << "push:: key_ver__pkey_pver_map does not contain <key= " << key << ", ver= " << ver << ">.";
    return 1;
  }
  return cache.push(key_ver__pkey_pver_map[kv] );
}

bool PBuffer::contains(std::string key, unsigned int ver)
{
  key_ver_pair kv = std::make_pair(key, ver);
  return cache.contains(key_ver__pkey_pver_map[kv] );
}

int PBuffer::get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector)
{
  unsigned int pver = key_ver__pkey_pver_map.size() / alphabet_size;
  char* pkeys_;
  palgo.get_to_prefetch(num_keys, pkeys_);
  for (int i = 0; i < num_keys; i++) {
    pkey_pver_pair pkpv = std::make_pair(pkeys_[i], pver);
    if (!pkey_pver__key_ver_map.contains(pkpv) ) {
      LOG(WARNING) << "get_to_prefetch:: palgo.get_to_prefetch returned a pkvp which is not registered!";
    }
    else {
      key_ver_vector.push_back(pkey_pver__key_ver_map[pkpv] );
    }
  }
  free(pkeys_);
}

int PBuffer::prefetch(std::vector<key_ver_pair>& key_ver_vector)
{
  for (std::vector<key_ver_pair>::iterator it = key_ver_vector.begin(); it != key_ver_vector.end(); it++) {
    std::map<std::string, std::string> handle_prefetch_map;
    handle_prefetch_map["key"] = it->first;
    handle_prefetch_map["ver"] = boost::lexical_cast<std::string>(it->second);
    
    _handle_prefetch_cb(handle_prefetch_map);
    push(it->first, it->second);
  }
}
