#include "lzprefetch.h"

/******************************************  LZAlgo  **********************************************/
LZAlgo::LZAlgo(int alphabet_size, char* alphabet_)
: alphabet_size(alphabet_size),
  alphabet_(alphabet_),
  parse_tree(false, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: constructed."; }

void LZAlgo::print_access_seq()
{
  LOG(INFO) << "print_access_seq:: ";
  for (std::vector<char>::iterator it = access_seq_vector.begin(); it != access_seq_vector.end(); it++) {
    std::cout << *it << ", ";
  }
  std::cout << "\n";
}

void LZAlgo::print_parse_tree()
{
  LOG(INFO) << "print_parse_tree::\n";
  std::cout << parse_tree.to_str();
}

void LZAlgo::pprint_parse_tree()
{
  LOG(INFO) << "pprint_parse_tree::\n";
  std::cout << parse_tree.to_pretty_str();
}

int LZAlgo::add_access(char key)
{
  access_seq_vector.push_back(key);
  parse_tree.add_access(key);
  
  return 0;
}

int LZAlgo::get_key_prob_map_for_prefetch(std::map<char, float>& key_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch(key_prob_map);
}

int LZAlgo::get_to_prefetch(int& num_keys, char*& keys_)
{
  return parse_tree.get_to_prefetch(num_keys, keys_);
}

int LZAlgo::sim_prefetch_accuracy(float& hit_rate, int cache_size, std::vector<char> access_seq_v)
{
  std::deque<char> cache;
  int num_access = access_seq_v.size();
  int num_miss = 0;
  
  for (std::vector<char>::iterator it = access_seq_v.begin(); it != access_seq_v.end(); it++) {
    if (std::find(cache.begin(), cache.end(), *it) == cache.end() ) // Not in cache
      num_miss++;
    
    add_access(*it);
    
    int num_keys = 1; //cache_size;
    char* keys_;
    get_to_prefetch(num_keys, keys_);
    // Update cache
    for (int i = 0; i < num_keys; i++) {
      char key = keys_[i];
      if (std::find(cache.begin(), cache.end(), key) != cache.end() ) // In cache
        continue;
      
      if (cache.size() == cache_size)
        cache.pop_front();
      cache.push_back(key);
    }
    free(keys_);
  }
  
  hit_rate = 1.0 - (float)num_miss/num_access;
  
  return 0;
}

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(int alphabet_size, char* alphabet_, int context_size)
: alphabet_size(alphabet_size),
  alphabet_(alphabet_),
  parse_tree(true, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: constructed."; }

void PPMAlgo::print_access_seq()
{
  LOG(INFO) << "print_access_seq:: ";
  for (std::vector<char>::iterator it = access_seq_vector.begin(); it != access_seq_vector.end(); it++) {
    std::cout << *it << ",";
  }
  std::cout << "\n";
}

void PPMAlgo::print_parse_tree()
{
  LOG(INFO) << "print_parse_tree::\n";
  std::cout << parse_tree.to_str();
}

void PPMAlgo::pprint_parse_tree()
{
  LOG(INFO) << "pprint_parse_tree::\n";
  std::cout << parse_tree.to_pretty_str();
}

int PPMAlgo::add_access(char key)
{
  access_seq_vector.push_back(key);
  parse_tree.add_access_with_context(key);
  
  return 0;
}

int PPMAlgo::get_key_prob_map_for_prefetch(std::map<char, float>& key_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch_with_context(key_prob_map);
}

int PPMAlgo::get_to_prefetch(int& num_keys, char*& keys_)
{
  return parse_tree.get_to_prefetch(num_keys, keys_);
}

int PPMAlgo::sim_prefetch_accuracy(float& hit_rate, int cache_size, std::vector<char> access_seq_v, std::vector<char>& accuracy_seq_v)
{
  std::deque<char> cache;
  int num_access = access_seq_v.size();
  int num_miss = 0;
  char predicted_key;
  
  for (std::vector<char>::iterator it = access_seq_v.begin(); it != access_seq_v.end(); it++) {
    if (std::find(cache.begin(), cache.end(), *it) == cache.end() ) // Not in cache
      num_miss++;
    
    if (it =! access_seq_v.begin() {
      if (*it == predicted_key)
        accuracy_seq_v.push_back('-');
      else
        accuracy_seq_v.push_back('f');
    }
    
    add_access(*it);
    
    int num_keys = 1; //cache_size;
    char* keys_;
    get_to_prefetch(num_keys, keys_);
    if (num_keys > 0)
      predicted_key = keys[0];
    // Update cache
    for (int i = 0; i < num_keys; i++) {
      char key = keys_[i];
      if (std::find(cache.begin(), cache.end(), key) != cache.end() ) // In cache
        continue;
      
      if (cache.size() == cache_size)
        cache.pop_front();
      cache.push_back(key);
    }
    free(keys_);
  }
  
  hit_rate = 1.0 - (float)num_miss/num_access;
  
  return 0;
}