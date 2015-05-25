#include "palgorithm.h"

/***************************************  PrefetchAlgo  *******************************************/
PrefetchAlgo::PrefetchAlgo(PREFETCH_T prefetch_t, size_t context_size)
: parse_tree(prefetch_t, context_size) {}
  
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

int PrefetchAlgo::get_to_prefetch(size_t& num_acc, KEY_T*& acc_)
{
  return parse_tree.get_to_prefetch(num_acc, acc_);
}

int PrefetchAlgo::sim_prefetch_accuracy(float& hit_rate, size_t cache_size, 
                                        std::vector<acc_step_pair> acc_step_v, std::vector<char>& accuracy_v )
{
  Cache<acc_step_pair> cache(cache_size);
  int num_miss = 0;
  std::map<KEY_T, int> acc_step_map;
  
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    std::cout << "sim_prefetch_accuracy:: is <" << it->first << ", " << it->second << ">"
              << "in the cache= \n" << cache.to_str() << "\n";
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wA-dataspaces scenario data is used only once
    cache.del(*it);
    
    add_access(it->first); // Reg only the acc
    
    size_t num_acc = 1; //cache_size;
    KEY_T* acc_;
    get_to_prefetch(num_acc, acc_);
    // Update cache
    for (int i = 0; i < num_acc; i++) {
      KEY_T acc = acc_[i];
      // if (cache.contains(acc) )
      //   continue;
      if (acc_step_map.count(acc) == 0)
        acc_step_map[acc] = 0;
      
      cache.push(std::make_pair(acc, acc_step_map[acc]) );
      acc_step_map[acc] += 1;
    }
    free(acc_);
  }
  
  hit_rate = 1.0 - (float)num_miss / acc_step_v.size();
  
  return 0;
}

/******************************************  LZAlgo  **********************************************/
LZAlgo::LZAlgo()
: PrefetchAlgo(W_LZ, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: destructed."; }

/******************************************  ALZAlgo  **********************************************/
ALZAlgo::ALZAlgo()
: PrefetchAlgo(W_ALZ, 0)
{
  // 
  LOG(INFO) << "ALZAlgo:: constructed.";
}

ALZAlgo::~ALZAlgo() { LOG(INFO) << "ALZAlgo:: destructed."; }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(size_t context_size)
: PrefetchAlgo(W_PPM, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: destructed."; }

/******************************************  POAlgo  **********************************************/
POAlgo::POAlgo()
: PrefetchAlgo(W_POISSON, 0)
{
  // 
  LOG(INFO) << "POAlgo:: constructed.";
}

POAlgo::~POAlgo() { LOG(INFO) << "POAlgo:: destructed."; }
