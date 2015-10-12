#include "markov.h"

/**********************************************  MAlgo  *******************************************/
MAlgo::MAlgo(MALGO_T malgo_t, int context_size)
: parse_tree(malgo_t, context_size) {}
  
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
: MAlgo(MALGO_W_LZ, 0)
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

LZAlgo::~LZAlgo() { LOG(INFO) << "LZAlgo:: destructed."; }

/******************************************  ALZAlgo  **********************************************/
ALZAlgo::ALZAlgo()
: MAlgo(MALGO_W_ALZ, 0)
{
  // 
  LOG(INFO) << "ALZAlgo:: constructed.";
}

ALZAlgo::~ALZAlgo() { LOG(INFO) << "ALZAlgo:: destructed."; }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(int context_size)
: MAlgo(MALGO_W_PPM, context_size)
{
  // 
  LOG(INFO) << "PPMAlgo:: constructed.";
}

PPMAlgo::~PPMAlgo() { LOG(INFO) << "PPMAlgo:: destructed."; }

/******************************************  POAlgo  **********************************************/
POAlgo::POAlgo()
: MAlgo(MALGO_W_PO, 0)
{
  // 
  LOG(INFO) << "POAlgo:: constructed.";
}

POAlgo::~POAlgo() { LOG(INFO) << "POAlgo:: destructed."; }

/**********************************************  MMAlgo  ******************************************/
MMAlgo::MMAlgo(std::vector<malgo_t__context_size_pair> malgo_t__context_size_v)
: malgo_t__context_size_v(malgo_t__context_size_v)
{
  for (std::vector<malgo_t__context_size_pair>::iterator it = malgo_t__context_size_v.begin(); it != malgo_t__context_size_v.end(); it++)
    parse_tree_v.push_back(boost::make_shared<ParseTree>(it->first, it->second) );
  // 
  LOG(INFO) << "MMAlgo:: constructed.";
}

MMAlgo::~MMAlgo() { LOG(INFO) << "MMAlgo:: destructed."; }

std::string MMAlgo::to_str()
{
  std::stringstream ss;
  ss << "malgo_t__context_size_v= \n" << patch_all::pvec_to_str<>(malgo_t__context_size_v) << "\n"
     << "acc_s= " << patch_all::set_to_str<>(acc_s) << "\n"
     << "acc_v= " << patch_all::vec_to_str<>(acc_v) << "\n";
  
  // ss << "parse_tree_v= \n";
  // for (std::vector<boost::shared_ptr<ParseTree> >::iterator parse_tree__ = parse_tree_v.begin(); parse_tree__ != parse_tree_v.end(); parse_tree__++)
  //   ss << "a parse_tree= \n" << (*parse_tree__)->to_str() << "\n";
  
  return ss.str();
}

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
      LOG(INFO) << "add_access:: (*parse_tree__)->add_access for malgo_t= " << (*parse_tree__)->get_malgo_t();
      return 1;
    }
  }
  
  return 0;
}

/***************************************  WMMAlgo : MMAlgo  ***************************************/
WMMAlgo::WMMAlgo(std::vector<malgo_t__context_size_pair> malgo_t__context_size_v,
                 std::vector<float> malgo_id__weight_v)
: MMAlgo(malgo_t__context_size_v),
  malgo_id__weight_v(malgo_id__weight_v)
{
  // 
  LOG(INFO) << "WMMAlgo:: constructed; \n" << to_str();
}

WMMAlgo::~WMMAlgo() { LOG(INFO) << "WMMAlgo:: destructed."; }

std::string WMMAlgo::to_str()
{
  std::stringstream ss;
  ss << "MMAlgo::to_str= \n" << MMAlgo::to_str() << "\n"
     << "malgo_id__weight_v= \n" << patch_all::vec_to_str<>(malgo_id__weight_v) << "\n";
  
  return ss.str();
}

int WMMAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int num_pt_ = parse_tree_v.size();
  std::vector<std::map<ACC_T, float> > pt_id__acc_prob_map_v(num_pt_);
  
  for (int i = 0; i < num_pt_; i++) {
    std::map<ACC_T, float>& acc_prob_map = pt_id__acc_prob_map_v[i];
    parse_tree_v[i]->get_key_prob_map_for_prefetch(acc_prob_map);
    // std::cout << "get_to_prefetch:: parse_tree_" << i << ", acc_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc_prob_map) << "\n";
  }
  
  std::map<ACC_T, float> acc__weighted_prob_map;
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    float weighted_prob = 0;
    for (int i = 0; i < num_pt_; i++)
      weighted_prob += pt_id__acc_prob_map_v[i][*it] * malgo_id__weight_v[i];
    
    acc__weighted_prob_map[*it] = weighted_prob;
  }
  // std::cout << "get_to_prefetch:: acc__weighted_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__weighted_prob_map) << "\n";
  
  std::map<float, ACC_T> weighted_prob__acc_map;
  for (std::map<ACC_T, float>::iterator it = acc__weighted_prob_map.begin(); it != acc__weighted_prob_map.end(); it++)
    weighted_prob__acc_map[it->second] = it->first;
  
  for (std::map<float, ACC_T>::reverse_iterator rit = weighted_prob__acc_map.rbegin(); rit != weighted_prob__acc_map.rend(); rit++) {
    acc_v.push_back(rit->second);
    if (acc_v.size() == num_acc)
      break;
  }
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

/***************************************  MMMAlgo : MMAlgo  ***************************************/
MMMAlgo::MMMAlgo(std::vector<malgo_t__context_size_pair> malgo_t__context_size_v)
: MMAlgo(malgo_t__context_size_v)
{
  // 
  LOG(INFO) << "MMMAlgo:: constructed; \n" << to_str();
}

MMMAlgo::~MMMAlgo() { LOG(INFO) << "MMMAlgo:: destructed."; }

std::string MMMAlgo::to_str()
{
  std::stringstream ss;
  ss << "MMAlgo::to_str= \n" << MMAlgo::to_str() << "\n";
  
  return ss.str();
}

int MMMAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int num_pt_ = parse_tree_v.size();
  std::vector<std::map<ACC_T, float> > pt_id__acc_prob_map_v(num_pt_);
  
  for (int i = 0; i < num_pt_; i++) {
    std::map<ACC_T, float>& acc_prob_map = pt_id__acc_prob_map_v[i];
    parse_tree_v[i]->get_key_prob_map_for_prefetch(acc_prob_map);
    // std::cout << "get_to_prefetch:: parse_tree_" << i << ", acc_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc_prob_map) << "\n";
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
  // std::cout << "get_to_prefetch:: acc__max_prob_map= \n" << patch_all::map_to_str<ACC_T, float>(acc__max_prob_map) << "\n";
  
  std::map<float, ACC_T> max_prob__acc_map;
  for (std::map<ACC_T, float>::iterator it = acc__max_prob_map.begin(); it != acc__max_prob_map.end(); it++)
    max_prob__acc_map[it->second] = it->first;
  
  for (std::map<float, ACC_T>::reverse_iterator rit = max_prob__acc_map.rbegin(); rit != max_prob__acc_map.rend(); rit++) {
    acc_v.push_back(rit->second);
    if (acc_v.size() == num_acc)
      break;
  }
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

/***************************************  BMMAlgo : MMAlgo  ***************************************/
BMMAlgo::BMMAlgo(std::vector<malgo_t__context_size_pair> malgo_t__context_size_v, int window_size)
: MMAlgo(malgo_t__context_size_v),
  window_size(window_size)
{
  for (int i = 0; i < malgo_t__context_size_v.size(); i++) {
    malgo_id__score_queue_v.push_back(boost::make_shared<patch_all::Queue<int> >(window_size) );
    malgo_id__last_predicted_acc_v_v.push_back(boost::make_shared<std::vector<ACC_T> >() );
  }
  // 
  LOG(INFO) << "BMMAlgo:: constructed; \n" << to_str();
}

BMMAlgo::~BMMAlgo() { LOG(INFO) << "BMMAlgo:: destructed."; }

std::string BMMAlgo::to_str()
{
  std::stringstream ss;
  ss << "MMAlgo::to_str= \n" << MMAlgo::to_str() << "\n"
     << "window_size= " << window_size << "\n"
     << "malgo_id__score_queue_v= \n";
  int id = 0;
  for (std::vector<boost::shared_ptr<patch_all::Queue<int> > >::iterator it = malgo_id__score_queue_v.begin(); it != malgo_id__score_queue_v.end(); it++, id++)
    ss << id << " : " << (*it)->to_str() << "\n";
  
  return ss.str();
}

int BMMAlgo::add_access(ACC_T acc)
{
  // LOG(INFO) << "add_access:: acc= " << acc << "\n";
  int id = 0;
  for (std::vector<boost::shared_ptr<std::vector<ACC_T> > >::iterator it = malgo_id__last_predicted_acc_v_v.begin(); it != malgo_id__last_predicted_acc_v_v.end(); it++, id++) {
    if (std::find((*it)->begin(), (*it)->end(), acc) != (*it)->end() )
      malgo_id__score_queue_v[id]->push(1);
    else
      malgo_id__score_queue_v[id]->push(0);
  }
  
  return MMAlgo::add_access(acc);
}

int BMMAlgo::get_malgo_score(int malgo_id)
{
  int score = 0;
  for (std::deque<int>::iterator it = malgo_id__score_queue_v[malgo_id]->begin(); it != malgo_id__score_queue_v[malgo_id]->end(); it++)
    score += *it;
  
  return score;
}

int BMMAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  for (int i = 0; i < malgo_t__context_size_v.size(); i++) {
    malgo_id__last_predicted_acc_v_v[i]->clear();
    if (parse_tree_v[i]->get_to_prefetch(1, *malgo_id__last_predicted_acc_v_v[i] ) ) {
      LOG(ERROR) << "get_to_prefetch:: parse_tree_v[" << i << "]->get_to_prefetch failed.";
      return 1;
    }
  }
  
  // std::cout << "------------------------------------------- \n";
  // std::cout << "get_to_prefetch:: malgo_id__last_predicted_acc_v_v= \n";
  // int id = 0;
  // for (std::vector<boost::shared_ptr<std::vector<ACC_T> > >::iterator it = malgo_id__last_predicted_acc_v_v.begin(); it != malgo_id__last_predicted_acc_v_v.end(); it++, id++)
  //   std::cout << id << " : " << patch_all::vec_to_str<>(**it) << "\n";
  
  // std::cout << "get_to_prefetch:: malgo_id__score_queue_v= \n";
  // id = 0;
  // for (std::vector<boost::shared_ptr<patch_all::Queue<int> > >::iterator it = malgo_id__score_queue_v.begin(); it != malgo_id__score_queue_v.end(); it++, id++)
  //   std::cout << id << " : " << (*it)->to_str() << "\n";
  
  int max_score = 0;
  int i_max = 0;
  for (int i = 0; i < malgo_t__context_size_v.size(); i++) {
    int cur_score = get_malgo_score(i);
    if (cur_score > max_score) {
      max_score = cur_score;
      i_max = i;
    }
  }
  
  acc_v = *malgo_id__last_predicted_acc_v_v[i_max];
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}
