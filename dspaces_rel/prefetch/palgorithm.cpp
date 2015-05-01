#include "palgorithm.h"

/***************************************  PrefetchAlgo  *******************************************/
PrefetchAlgo::PrefetchAlgo(bool with_context, size_t context_size)
: parse_tree(with_context, context_size) {}
  
PrefetchAlgo::~PrefetchAlgo() {}

std::string PrefetchAlgo::parse_tree_to_str() { return parse_tree.to_str(); }

std::string PrefetchAlgo::parse_tree_to_pstr() { return parse_tree.to_pretty_str(); }

std::string PrefetchAlgo::access_seq_to_str()
{
  std::stringstream ss;
  for (std::vector<KEY_T>::iterator it = access_vec.begin(); it != access_vec.end(); it++) {
    ss << boost::lexical_cast<std::string>(*it) << ",";
  }
  ss << "\n";
  
  return ss.str();
}

std::vector<KEY_T> PrefetchAlgo::get_access_vec()
{
  return access_vec;
}

int PrefetchAlgo::add_access(KEY_T key)
{
  access_vec.push_back(key);
  parse_tree.add_access(key);
}

int PrefetchAlgo::get_key_prob_map_for_prefetch(std::map<KEY_T, float>& key_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch(key_prob_map);
}

int PrefetchAlgo::get_to_prefetch(size_t& num_keys, KEY_T*& keys_)
{
  return parse_tree.get_to_prefetch(num_keys, keys_);
}

int PrefetchAlgo::sim_prefetch_accuracy(float& hit_rate, size_t cache_size, std::vector<KEY_T> access_seq_v, 
                                        std::vector<char>& accuracy_seq_v )
{
  Cache<KEY_T> cache(cache_size);
  int num_miss = 0;
  
  for (std::vector<KEY_T>::iterator it = access_seq_v.begin(); it != access_seq_v.end(); it++) {
    if (!cache.contains(*it) ) {
      accuracy_seq_v.push_back('f');
      num_miss++;
    }
    else {
      accuracy_seq_v.push_back('-');
    }
    
    add_access(*it);
    
    size_t num_keys = 1; //cache_size;
    KEY_T* keys_;
    get_to_prefetch(num_keys, keys_);
    // Update cache
    for (int i = 0; i < num_keys; i++) {
      KEY_T key = keys_[i];
      if (cache.contains(key) )
        continue;
      
      cache.push(key);
    }
    free(keys_);
  }
  
  hit_rate = 1.0 - (float)num_miss/access_seq_v.size();
  
  return 0;
}

/******************************************  LZAlgo  **********************************************/
LZAlgo::LZAlgo()
: PrefetchAlgo(false, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: destructed."; }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(size_t context_size)
: PrefetchAlgo(true, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: destructed."; }

