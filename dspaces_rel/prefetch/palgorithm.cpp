#include "palgorithm.h"

/***************************************  PrefetchAlgo  *******************************************/
PrefetchAlgo::PrefetchAlgo(PREFETCH_T prefetch_t, size_t context_size)
: parse_tree(prefetch_t, context_size) {}
  
PrefetchAlgo::~PrefetchAlgo() {}

std::string PrefetchAlgo::parse_tree_to_str() { return parse_tree.to_str(); }

std::string PrefetchAlgo::parse_tree_to_pstr() { return parse_tree.to_pretty_str(); }

std::vector<ACC_T> PrefetchAlgo::get_acc_v()
{
  return acc_v;
}

int PrefetchAlgo::add_access(ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
  parse_tree.add_access(acc);
}

int PrefetchAlgo::get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch(acc_prob_map);
}

int PrefetchAlgo::get_to_prefetch(size_t& num_acc, std::vector<ACC_T>& acc_v,
                                  const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  if (parse_tree.get_to_prefetch(num_acc, acc_v) )
    return 1;
  
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  return 0;
}

void PrefetchAlgo::sim_prefetch_accuracy(float& hit_rate, size_t cache_size, 
                                         std::vector<acc_step_pair> acc_step_v, std::vector<char>& accuracy_v )
{
  Cache<ACC_T, acc_step_pair> cache(cache_size, boost::function<void(acc_step_pair)>() );
  int num_miss = 0;
  std::map<ACC_T, int> acc__last_acced_step_map;
  std::map<ACC_T, int> acc__last_cached_step_map;
  
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    std::cout << "sim_prefetch_accuracy:: is <" << it->first << ", " << it->second << ">"
              << " in the cache= \n" << cache.to_str() << "\n";
    acc__last_acced_step_map[it->first] = it->second;
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wA-dataspaces scenario data is used only once
    cache.del(it->first, *it);
    
    add_access(it->first); // Reg only the acc
    
    size_t num_acc = 1; //cache_size;
    std::vector<ACC_T> acc_v;
    std::vector<ACC_T> eacc_v;
    get_to_prefetch(num_acc, acc_v, cache.get_cached_acc_v(), eacc_v);
    
    int i;
    std::vector<ACC_T>::iterator jt;
    for (jt = eacc_v.begin(), i = 0; i < cache_size - num_acc - cache.size(), jt != eacc_v.end(); i++, jt++)
      acc_v.push_back(*jt);
    
    // Update cache
    for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
      ACC_T acc = *it;
      if (acc__last_cached_step_map.count(acc) == 0)
        acc__last_cached_step_map[acc] = 0;
      
      if (acc__last_cached_step_map[acc] < acc__last_acced_step_map[acc] )
        acc__last_cached_step_map[acc] = acc__last_acced_step_map[acc];
      else if (acc__last_cached_step_map[acc] > acc__last_acced_step_map[acc])
        continue;
      // 
      cache.push(acc, std::make_pair(acc, acc__last_cached_step_map[acc] + 1) );
      acc__last_cached_step_map[acc] += 1;
    }
  }
  
  hit_rate = 1.0 - (float)num_miss / acc_step_v.size();
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
: PrefetchAlgo(W_PO, 0)
{
  // 
  LOG(INFO) << "POAlgo:: constructed.";
}

POAlgo::~POAlgo() { LOG(INFO) << "POAlgo:: destructed."; }
