#include "palgorithm.h"

/***************************************  PrefetchAlgo  *******************************************/
PrefetchAlgo::PrefetchAlgo(size_t alphabet_size, char* alphabet_, bool with_context, size_t context_size)
: alphabet_size(alphabet_size),
  alphabet_(alphabet_),
  parse_tree(with_context, context_size) {}
  
PrefetchAlgo::~PrefetchAlgo() {}

std::string PrefetchAlgo::parse_tree_to_str() { return parse_tree.to_str(); }

std::string PrefetchAlgo::parse_tree_to_pstr() { return parse_tree.to_pretty_str(); }

std::string PrefetchAlgo::access_seq_to_str()
{
  std::stringstream ss;
  for (std::vector<char>::iterator it = access_vector.begin(); it != access_vector.end(); it++) {
    ss << *it << ",";
  }
  ss << "\n";
  
  return ss.str();
}

int PrefetchAlgo::add_access(char key)
{
  access_vector.push_back(key);
  parse_tree.add_access(key);
}

int PrefetchAlgo::get_key_prob_map_for_prefetch(std::map<char, float>& key_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch(key_prob_map);
}

int PrefetchAlgo::get_to_prefetch(size_t& num_keys, char*& keys_)
{
  return parse_tree.get_to_prefetch(num_keys, keys_);
}

int PrefetchAlgo::sim_prefetch_accuracy(float& hit_rate, size_t cache_size, std::vector<char> access_seq_v, 
                                        std::vector<char>& accuracy_seq_v )
{
  Cache<char> cache(cache_size);
  int num_miss = 0;
  
  for (std::vector<char>::iterator it = access_seq_v.begin(); it != access_seq_v.end(); it++) {
    if (!cache.contains(*it) ) {
      accuracy_seq_v.push_back('f');
      num_miss++;
    }
    else {
      accuracy_seq_v.push_back('-');
    }
    
    add_access(*it);
    
    size_t num_keys = 1; //cache_size;
    char* keys_;
    get_to_prefetch(num_keys, keys_);
    // Update cache
    for (int i = 0; i < num_keys; i++) {
      char key = keys_[i];
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
LZAlgo::LZAlgo(size_t alphabet_size, char* alphabet_)
: PrefetchAlgo(alphabet_size, alphabet_, false, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: constructed."; }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(size_t alphabet_size, char* alphabet_, size_t context_size)
: PrefetchAlgo(alphabet_size, alphabet_, true, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: constructed."; }

