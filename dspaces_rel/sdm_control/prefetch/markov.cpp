#include "markov.h"

/**********************************************  MMAlgo  ******************************************/
MMAlgo::MMAlgo(MMALGO_T mmalgo_t,
               std::map<MALGO_T, float> malgo_t__weight_map)
: mmalgo_t(mmalgo_t),
  malgo_t__weight_map(malgo_t__weight_map)
{
  for (std::map<PREFETCH_T, float>::iterator it = prefetch_t__weight_map.begin(); it != prefetch_t__weight_map.end(); it++) {
    parse_tree_v.push_back(boost::make_shared<ParseTree>(it->first) );
    if (mmalgo_t == MMALGO_W_WEIGHT)
      pt_id__weight_map[parse_tree_v.size() - 1] = it->second;
  }
  // 
  LOG(INFO) << "MMAlgo:: constructed.";
}

MMAlgo::~MMAlgo() { LOG(INFO) << "MMAlgo:: destructed."; }

void MMAlgo::reset()
{
  for (std::vector<boost::shared_ptr<ParseTree> >::iterator parse_tree__ = parse_tree_v.begin(); parse_tree__ != parse_tree_v.end(); parse_tree__++)
    (*parse_tree__)->reset();
    
  acc_s.clear();
  acc_v.clear();
}

std::vector<ACC_T> MMAlgo::get_acc_v() { return acc_v; }

int MMAlgo::train(std::vector<ACC_T> acc_v)
{
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (add_access(*it) )
      return 1;
  }
  
  return 0;
}

int MMAlgo::add_access(ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
  
  for (std::vector<boost::shared_ptr<ParseTree> >::iterator parse_tree__ = parse_tree_v.begin(); parse_tree__ != parse_tree_v.end(); parse_tree__++) {
    if ( (*parse_tree__)->add_access(acc) ) {
      LOG(INFO) << "add_access:: (*parse_tree__)->add_access for prefetch_t= " << (*parse_tree__)->get_prefetch_t();
      return 1;
    }
  }
  
  return 0;
}

int MMAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                            const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  if (m_prefetch_t == MMALGO_W_WEIGHT) {
    if (get_to_prefetch_w_weight(num_acc, acc_v) )
      return 1;
  }
  else if (m_prefetch_t == MMALGO_W_MAX) {
    if (get_to_prefetch_w_max(num_acc, acc_v) )
      return 1;
  }
    
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

int MMAlgo::get_to_prefetch_w_max(int& num_acc, std::vector<ACC_T>& acc_v)
{
  int num_pt_ = parse_tree_v.size();
  std::vector<std::map<ACC_T, float> > pt_id__acc_prob_map_v(num_pt_);
  
  std::set<ACC_T> acc_s;
  for (int i = 0; i < num_pt_; i++) {
    std::map<ACC_T, float>& acc_prob_map = pt_id__acc_prob_map_v[i];
    parse_tree_v[i]->get_key_prob_map_for_prefetch(acc_prob_map);
    // std::cout << "get_to_prefetch_w_max:: parse_tree_" << i << ", acc_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc_prob_map) << "\n";
    
    for (std::map<ACC_T, float>::iterator it = acc_prob_map.begin(); it != acc_prob_map.end(); it++)
      acc_s.insert(it->first);
  }
  
  std::map<ACC_T, float> acc__max_prob_map;
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    float max_prob = 0;
    for (int i = 0; i < num_pt_; i++) {
      if (max_prob < pt_id__acc_prob_map_v[i][*it] )
        max_prob = pt_id__acc_prob_map_v[i][*it];
    }
    acc__max_prob_map[*it] = max_prob;
  }
  // std::cout << "get_to_prefetch_w_max:: acc__max_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__max_prob_map) << "\n";
  
  std::map<float, ACC_T> max_prob__acc_map;
  for (std::map<ACC_T, float>::iterator it = acc__max_prob_map.begin(); it != acc__max_prob_map.end(); it++)
    max_prob__acc_map[it->second] = it->first;
  
  for (std::map<float, ACC_T>::reverse_iterator rit = max_prob__acc_map.rbegin(); rit != max_prob__acc_map.rend(); rit++) {
    acc_v.push_back(rit->second);
    if (acc_v.size() == num_acc)
      break;
  }
  num_acc = acc_v.size();
  
  return 0;
}

int MMAlgo::get_to_prefetch_w_weight(int& num_acc, std::vector<ACC_T>& acc_v)
{
  int num_pt_ = parse_tree_v.size();
  std::vector<std::map<ACC_T, float> > pt_id__acc_prob_map_v(num_pt_);
  
  std::set<ACC_T> acc_s;
  for (int i = 0; i < num_pt_; i++) {
    std::map<ACC_T, float>& acc_prob_map = pt_id__acc_prob_map_v[i];
    parse_tree_v[i]->get_key_prob_map_for_prefetch(acc_prob_map);
    // std::cout << "get_to_prefetch_w_weight:: parse_tree_" << i << ", acc_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc_prob_map) << "\n";
    
    for (std::map<ACC_T, float>::iterator it = acc_prob_map.begin(); it != acc_prob_map.end(); it++)
      acc_s.insert(it->first);
  }
  
  std::map<ACC_T, float> acc__weighted_prob_map;
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    float weighted_prob = 0;
    for (int i = 0; i < num_pt_; i++)
      weighted_prob += pt_id__acc_prob_map_v[i][*it] * pt_id__weight_map[i];
    
    acc__weighted_prob_map[*it] = weighted_prob;
  }
  // std::cout << "get_to_prefetch_w_weight:: acc__weighted_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__weighted_prob_map) << "\n";
  
  
  std::map<float, ACC_T> weighted_prob__acc_map;
  for (std::map<ACC_T, float>::iterator it = acc__weighted_prob_map.begin(); it != acc__weighted_prob_map.end(); it++)
    weighted_prob__acc_map[it->second] = it->first;
  
  for (std::map<float, ACC_T>::reverse_iterator rit = weighted_prob__acc_map.rbegin(); rit != weighted_prob__acc_map.rend(); rit++) {
    acc_v.push_back(rit->second);
    if (acc_v.size() == num_acc)
      break;
  }
  num_acc = acc_v.size();
  
  return 0;
}

/**********************************************  MAlgo  *******************************************/
MAlgo::MAlgo(PREFETCH_T prefetch_t, int context_size)
: parse_tree(prefetch_t, context_size) {}
  
MAlgo::~MAlgo() {}

void MAlgo::reset()
{
  parse_tree.reset();
  
  acc_s.clear();
  acc_v.clear();
}

std::string MAlgo::parse_tree_to_pstr() { return parse_tree.to_pretty_str(); }

std::vector<ACC_T> MAlgo::get_acc_v() { return acc_v; }

int MAlgo::train(std::vector<ACC_T> acc_v)
{
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    if (add_access(*it) )
      return 1;
  }
  
  return 0;
}

int MAlgo::add_access(ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
  return parse_tree.add_access(acc);
}

int MAlgo::get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map)
{
  return parse_tree.get_key_prob_map_for_prefetch(acc_prob_map);
}

int MAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
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

/******************************************  LZAlgo  **********************************************/
LZAlgo::LZAlgo()
: MAlgo(W_LZ, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: destructed."; }

/******************************************  ALZAlgo  **********************************************/
ALZAlgo::ALZAlgo()
: MAlgo(W_ALZ, 0)
{
  // 
  LOG(INFO) << "ALZAlgo:: constructed.";
}

ALZAlgo::~ALZAlgo() { LOG(INFO) << "ALZAlgo:: destructed."; }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(int context_size)
: MAlgo(W_PPM, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: destructed."; }

/******************************************  POAlgo  **********************************************/
POAlgo::POAlgo()
: MAlgo(W_PO, 0)
{
  // 
  LOG(INFO) << "POAlgo:: constructed.";
}

POAlgo::~POAlgo() { LOG(INFO) << "POAlgo:: destructed."; }
