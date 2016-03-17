#include "palgo.h"
#include "patch_pre.h"

/**********************************************  PAlgo  *******************************************/
std::string PAlgo::to_str()
{
  std::stringstream ss;
  // ss << "acc_s= " << patch::set_to_str<>(acc_s) << "\n"
  //   << "acc_v= " << patch::vec_to_str<>(acc_v) << "\n";
  
  return ss.str();
}

std::vector<ACC_T> PAlgo::get_acc_v() { return acc_v; }

void PAlgo::reset()
{
  acc_s.clear();
  acc_v.clear();
}

int PAlgo::add_access(ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
  
  return 0;
}

int PAlgo::add_access(float acc_time, ACC_T acc)
{
  acc_s.insert(acc);
  acc_v.push_back(acc);
  
  return 0;
}

/**********************************************  TAlgo  *******************************************/
TAlgo::TAlgo() {}
TAlgo::~TAlgo() {}

std::string TAlgo::to_str()
{
  std::stringstream ss;
  ss << "acc___arr__inter_time_pair_v_map= \n";
  for (std::map<ACC_T, std::vector<time_time_pair> >::iterator map_it = acc___arr__inter_time_pair_v_map.begin();
       map_it != acc___arr__inter_time_pair_v_map.end(); map_it++) {
    ss << "\t acc= " << map_it->first << "\n";
    for (std::vector<time_time_pair>::iterator it = (map_it->second).begin(); it != (map_it->second).end(); it++)
      ss << "\t" << PAIR_TO_STR(*it) << "\n";
      
      // ss << "\t" << "\n";
  }
  
  ss << "acc__mean_var_pair_map= \n";
  for (std::map<ACC_T, mean_var_pair>::iterator it = acc__mean_var_pair_map.begin(); it != acc__mean_var_pair_map.end(); it++)
    ss << "\t acc= " << it->first << " : " << PAIR_TO_STR(it->second) << "\n";
  
  return ss.str();
}

void TAlgo::reset()
{
  PAlgo::reset();
  acc___arr__inter_time_pair_v_map.clear();
  acc__mean_var_pair_map.clear();
}

int TAlgo::train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v)
{
  int err;
  for (std::vector<arr_time__acc_pair>::iterator it = arr_time__acc_pair_v.begin(); it != arr_time__acc_pair_v.end(); it++) {
    return_if_err(add_access(it->first, it->second), err)
  }
  
  return 0;
}

int TAlgo::add_access(float arr_time, ACC_T acc)
{
  int err;
  return_if_err(PAlgo::add_access(acc), err)
  
  if (acc___arr__inter_time_pair_v_map.count(acc) == 0)
    acc___arr__inter_time_pair_v_map[acc] = std::vector<time_time_pair>();
  
  std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[acc];
  float inter_time = NO_INTER_TIME;
  if (!ai_v.empty() ) {
    time_time_pair last_ai = ai_v.back();
    inter_time = arr_time - last_ai.first;
    return_if_err(inter_time < 0, err)
  }
  ai_v.push_back(std::make_pair(arr_time, inter_time) );
  if (inter_time == NO_INTER_TIME)
    return 0;
  // Updating mean, var
  int num_sample = ai_v.size() - 1; // First inter_arr_time is always 0
  
  float sum = 0;
  for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++)
    sum += it->second;
  float mean = sum / num_sample;
  
  float var = MIN_VARIANCE;
  if (num_sample > 2) {
    sum = 0;
    for (std::vector<time_time_pair>::iterator it = ai_v.begin(); it != ai_v.end(); it++) {
      if (it->second != NO_INTER_TIME)
        sum += pow(it->second - mean, 2);
    }
    var = std::max(sum / (num_sample - 1), MIN_VARIANCE);
  }
  
  acc__mean_var_pair_map[acc] = std::make_pair(mean, var);
  
  return 0;
}

int TAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                           const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  std::map<float, ACC_T> prob_acc_map;
  std::vector<ACC_T> skipped_v;
  for (std::map<ACC_T, mean_var_pair>::iterator it = acc__mean_var_pair_map.begin(); it != acc__mean_var_pair_map.end(); it++) {
    std::vector<time_time_pair>& ai_v = acc___arr__inter_time_pair_v_map[it->first];
    mean_var_pair mv = it->second;
    
    float time_passed_since_last_arr = _time - ai_v.back().first;
    return_if_err(time_passed_since_last_arr < 0, err)
    
    if (time_passed_since_last_arr < MIN_INTER_TIME_FOR_PREDICTION) {
      // log_(WARNING, "skipping acc= " << it->first << "\n"
      //               << "\t time_passed_since_last_arr= " << time_passed_since_last_arr << " < MIN_INTER_TIME_FOR_PREDICTION= " << MIN_INTER_TIME_FOR_PREDICTION)
      skipped_v.push_back(it->first);
      continue;
    }
    else if (time_passed_since_last_arr > 2*mv.first) {
    // if (time_passed_since_last_arr > 4*pow(mv.second, (float)0.5) ) {
      // log_(WARNING, "skipping acc= " << it->first << "\n"
      //               << "\t time_passed_since_last_arr= " << time_passed_since_last_arr  << " >> 3*mean= " << 3*mv.first)
                    // << " >> 4*std= " << 4*pow(mv.second, (float)0.5) << "; acc= " << it->first)
      continue;
    }
    
    boost::math::normal_distribution<float> inter_arr_dist(mv.first, pow(mv.second, 0.5) );
    float prob = boost::math::cdf(boost::math::complement(inter_arr_dist, time_passed_since_last_arr) );
    while (prob_acc_map.count(prob) != 0)
      prob += 0.0001;
    // log_(INFO, ">>>>> " << "acc= " << it->first << "\n"
    //           << "\t time_passed_since_last_arr= " << time_passed_since_last_arr << "\n"
    //           << "\t prob= " << prob)
    prob_acc_map[prob] = it->first;
  }
  
  // for (std::map<float, ACC_T>::reverse_iterator rit = prob_acc_map.rbegin(); rit != prob_acc_map.rend(); rit++)
  for (std::map<float, ACC_T>::iterator it = prob_acc_map.begin(); it != prob_acc_map.end(); it++) {
    acc_v.push_back(it->second);
    if (acc_v.size() == num_acc)
      break;
  }
  while (acc_v.size() < num_acc && !skipped_v.empty() ) {
    acc_v.push_back(skipped_v.back() );
    skipped_v.pop_back();
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

/**********************************************  MAlgo  *******************************************/
MAlgo::MAlgo(PALGO_T malgo_t, int context_size)
: parse_tree(malgo_t, context_size) {}
  
MAlgo::~MAlgo() {}

void MAlgo::reset()
{
  PAlgo::reset();
  parse_tree.reset();
}

std::string MAlgo::parse_tree_to_pstr() { return parse_tree.to_pretty_str(); }

int MAlgo::train(std::vector<ACC_T> acc_v)
{
  int err;
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    return_if_err(add_access(*it), err)
  }
  
  return 0;
}

int MAlgo::add_access(ACC_T acc)
{
  int err;
  return_if_err(PAlgo::add_access(acc), err);
  return_if_err(parse_tree.add_access(acc), err)
  
  return 0;
}

int MAlgo::get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map)
{
  int err;
  return_if_err(parse_tree.get_key_prob_map_for_prefetch(acc_prob_map), err)
  return 0;
}

int MAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                           const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  return_if_err(parse_tree.get_to_prefetch(num_acc, acc_v), err)
  // 
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
  log_(INFO, "constructed.")
}

LZAlgo::~LZAlgo() { log_(INFO, "destructed.") }

/******************************************  ALZAlgo  **********************************************/
ALZAlgo::ALZAlgo()
: MAlgo(MALGO_W_ALZ, 0)
{
  // 
  log_(INFO, "constructed.")
}

ALZAlgo::~ALZAlgo() { log_(INFO, "destructed.") }

/******************************************  PPMAlgo  *********************************************/
PPMAlgo::PPMAlgo(int context_size)
: MAlgo(MALGO_W_PPM, context_size)
{
  // 
  log_(INFO, "constructed.")
}

PPMAlgo::~PPMAlgo() { log_(INFO, "destructed.") }

/******************************************  POAlgo  **********************************************/
POAlgo::POAlgo()
: MAlgo(MALGO_W_PO, 0)
{
  // 
  log_(INFO, "constructed.")
}

POAlgo::~POAlgo() { log_(INFO, "destructed.") }

/**********************************************  MPAlgo  ******************************************/
MPAlgo::MPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v)
: palgo_t__context_size_v(palgo_t__context_size_v)
{
  for (std::vector<palgo_t__context_size_pair>::iterator it = palgo_t__context_size_v.begin(); it != palgo_t__context_size_v.end(); it++) {
    boost::shared_ptr<PAlgo> palgo_;
    switch (it->first) {
      case MALGO_W_LZ:
        palgo_ = boost::make_shared<LZAlgo>();
        break;
      case MALGO_W_ALZ:
        palgo_ = boost::make_shared<ALZAlgo>();
        break;
      case MALGO_W_PPM:
        palgo_ = boost::make_shared<PPMAlgo>(it->second);
        break;
      case MALGO_W_PO:
        palgo_ = boost::make_shared<POAlgo>();
        break;
      case TALGO:
        palgo_ = boost::make_shared<TAlgo>();
        break;
      default:
        break;
    }
    palgo_v.push_back(palgo_);
  }
  // 
  log_(INFO, "constructed.")
}

MPAlgo::~MPAlgo() { log_(INFO, "destructed.") }

std::string MPAlgo::to_str()
{
  std::stringstream ss;
  ss << "PAlgo::to_str= \n" << PAlgo::to_str() << "\n"
     << "palgo_t__context_size_v= \n" << patch::pvec_to_str<>(palgo_t__context_size_v) << "\n"
     << "palgo_id_used_v= " << patch::vec_to_str<>(palgo_id_used_v) << "\n";
  
  // ss << "palgo_v= \n";
  // for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++)
  //   ss << "a palgo= \n" << (*it)->to_str() << "\n";
  
  return ss.str();
}

void MPAlgo::reset()
{
  for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++)
    (*it)->reset();
}

int MPAlgo::train(std::vector<ACC_T> acc_v)
{
  int err;
  for (std::vector<ACC_T>::iterator it = acc_v.begin(); it != acc_v.end(); it++) {
    return_if_err(add_access(*it), err)
  }
  
  return 0;
}

int MPAlgo::add_access(ACC_T acc)
{
  int err;
  return_if_err(PAlgo::add_access(acc), err)
  
  for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++) {
    return_if_err((*it)->add_access(acc), err)
  }
  
  return 0;
}

int MPAlgo::add_access(float acc_time, ACC_T acc)
{
  int err;
  return_if_err(PAlgo::add_access(acc_time, acc), err)
  
  for (std::vector<boost::shared_ptr<PAlgo> >::iterator it = palgo_v.begin(); it != palgo_v.end(); it++) {
    return_if_err((*it)->add_access(acc_time, acc), err)
  }
  
  return 0;
}

/***************************************  WMPAlgo : MPAlgo  ***************************************/
WMPAlgo::WMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v,
                 std::vector<float> palgo_id__weight_v)
: MPAlgo(palgo_t__context_size_v),
  palgo_id__weight_v(palgo_id__weight_v)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

WMPAlgo::~WMPAlgo() { log_(INFO, "destructed.") }

std::string WMPAlgo::to_str()
{
  std::stringstream ss;
  ss << "MPAlgo::to_str= \n" << MPAlgo::to_str() << "\n"
     << "palgo_id__weight_v= \n" << patch::vec_to_str<>(palgo_id__weight_v) << "\n";
  
  return ss.str();
}

int WMPAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int num_palgo_ = palgo_v.size();
  std::vector<std::map<ACC_T, float> > palgo_id__acc_prob_map_v(num_palgo_);
  
  for (int i = 0; i < num_palgo_; i++) {
    std::map<ACC_T, float>& acc_prob_map = palgo_id__acc_prob_map_v[i];
    palgo_v[i]->get_acc_prob_map_for_prefetch(_time, acc_prob_map);
    // log_(INFO, "parse_tree_" << i << ", acc_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc_prob_map) )
  }
  
  std::map<ACC_T, float> acc__weighted_prob_map;
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    float weighted_prob = 0;
    for (int i = 0; i < num_palgo_; i++)
      weighted_prob += palgo_id__acc_prob_map_v[i][*it] * palgo_id__weight_v[i];
    
    acc__weighted_prob_map[*it] = weighted_prob;
  }
  // log_(INFO, "acc__weighted_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__weighted_prob_map) )
  
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

/***************************************  MMPAlgo : MPAlgo  ***************************************/
MMPAlgo::MMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v)
: MPAlgo(palgo_t__context_size_v)
{
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

MMPAlgo::~MMPAlgo() { log_(INFO, "destructed.") }

std::string MMPAlgo::to_str()
{
  std::stringstream ss;
  ss << "MPAlgo::to_str= \n" << MPAlgo::to_str() << "\n";
  
  return ss.str();
}

int MMPAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int num_palgo_ = palgo_v.size();
  std::vector<std::map<ACC_T, float> > palgo_id__acc_prob_map_v(num_palgo_);
  
  for (int i = 0; i < num_palgo_; i++) {
    std::map<ACC_T, float>& acc_prob_map = palgo_id__acc_prob_map_v[i];
    palgo_v[i]->get_acc_prob_map_for_prefetch(_time, acc_prob_map);
    // log_(INFO, "palgo_id= " << i << ", acc_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc_prob_map) )
  }
  
  std::map<ACC_T, float> acc__max_prob_map;
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    float max_prob = 0;
    for (int i = 0; i < num_palgo_; i++) {
      if (max_prob < palgo_id__acc_prob_map_v[i][*it] )
        max_prob = palgo_id__acc_prob_map_v[i][*it];
    }
    acc__max_prob_map[*it] = max_prob;
  }
  // log_(INFO, "acc__max_prob_map= \n" << patch::map_to_str<ACC_T, float>(acc__max_prob_map) )
  
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

/***************************************  BMPAlgo : MPAlgo  ***************************************/
BMPAlgo::BMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v, int window_size)
: MPAlgo(palgo_t__context_size_v),
  window_size(window_size)
{
  for (int i = 0; i < palgo_t__context_size_v.size(); i++) {
    palgo_id__score_queue_v.push_back(boost::make_shared<patch::Queue<int> >(window_size) );
    palgo_id__last_predicted_acc_v_v.push_back(boost::make_shared<std::vector<ACC_T> >() );
  }
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

BMPAlgo::~BMPAlgo() { log_(INFO, "destructed.") }

std::string BMPAlgo::to_str()
{
  std::stringstream ss;
  ss << "MPAlgo::to_str= \n" << MPAlgo::to_str() << "\n"
     << "window_size= " << window_size << "\n"
     << "palgo_id__score_queue_v= \n";
  int id = 0;
  for (std::vector<boost::shared_ptr<patch::Queue<int> > >::iterator it = palgo_id__score_queue_v.begin(); it != palgo_id__score_queue_v.end(); it++, id++)
    ss << id << " : " << (*it)->to_str() << "\n";
  
  return ss.str();
}

int BMPAlgo::add_access(float acc_time, ACC_T acc)
{
  int err;
  return_if_err(MPAlgo::add_access(acc_time, acc), err)
  
  for (int i = 0; i < palgo_v.size(); i++) {
    std::vector<ACC_T>& t_acc_v = *palgo_id__last_predicted_acc_v_v[i];
    if (std::find(t_acc_v.begin(), t_acc_v.end(), acc) != t_acc_v.end() )
      palgo_id__score_queue_v[i]->push(1);
    else
      palgo_id__score_queue_v[i]->push(0);
  }
  
  return 0;
}

int BMPAlgo::get_malgo_score(int palgo_id)
{
  int score = 0;
  boost::shared_ptr<patch::Queue<int> > score_q_ = palgo_id__score_queue_v[palgo_id];
  // log_(INFO, "palgo_id= " << palgo_id << ", score_q= " << score_q_->to_str() )
  for (std::deque<int>::iterator it = score_q_->begin(); it != score_q_->end(); it++)
    score += *it;
  
  return score;
}

int BMPAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                             const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  for (int i = 0; i < palgo_v.size(); i++) {
    std::vector<ACC_T>& t_acc_v = *palgo_id__last_predicted_acc_v_v[i];
    t_acc_v.clear();
    std::vector<ACC_T> t_eacc_v;
    int t_num_acc = num_acc;
    return_if_err(palgo_v[i]->get_to_prefetch(_time, num_acc, t_acc_v, std::vector<ACC_T>(), t_eacc_v), err)
  }
  
  // log_(INFO, "palgo_id__last_predicted_acc_v_v=")
  // int id = 0;
  // for (std::vector<boost::shared_ptr<std::vector<ACC_T> > >::iterator it = palgo_id__last_predicted_acc_v_v.begin(); it != palgo_id__last_predicted_acc_v_v.end(); it++, id++)
  //   std::cout << id << " : " << patch::vec_to_str<>(**it) << "\n";
  
  // log_(INFO, "palgo_id__score_queue_v=")
  // id = 0;
  // for (std::vector<boost::shared_ptr<patch::Queue<int> > >::iterator it = palgo_id__score_queue_v.begin(); it != palgo_id__score_queue_v.end(); it++, id++)
  //   std::cout << id << " : " << (*it)->to_str() << "\n";
  
  int max_score = 0;
  int i_max = 0;
  for (int i = 0; i < palgo_v.size(); i++) {
    int cur_score = get_malgo_score(i);
    if (cur_score > max_score) {
      max_score = cur_score;
      i_max = i;
    }
  }
  
  palgo_id_used_v.push_back(i_max);
  acc_v = *palgo_id__last_predicted_acc_v_v[i_max];
  num_acc = acc_v.size();
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

/**************************************  MJMPAlgo : MPAlgo  ***************************************/
MJMPAlgo::MJMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v,
                   float beta)
: MPAlgo(palgo_t__context_size_v),
  beta(beta)
{
  if (beta < 0 || beta > 1) {
    log_(ERROR, "unexpected beta= " << beta)
    return;
  }
  
  for (int i = 0; i < palgo_v.size(); i++) {
    // palgo_id__weight_v.push_back(std::numeric_limits<float>::max() );
    palgo_id__weight_v.push_back(12345678);
    palgo_id__last_predicted_acc_v_v.push_back(boost::make_shared<std::vector<ACC_T> >() );
  }
  // 
  log_(INFO, "constructed; \n" << to_str() )
}

MJMPAlgo::~MJMPAlgo() { log_(INFO, "destructed.") }

std::string MJMPAlgo::to_str()
{
  std::stringstream ss;
  ss << "MPAlgo::to_str= \n" << MPAlgo::to_str() << "\n"
    << "palgo_id__weight_v= " << patch::vec_to_str<>(palgo_id__weight_v) << "\n";
  
  return ss.str();
}

int MJMPAlgo::add_access(ACC_T acc)
{
  int err;
  return_if_err(MPAlgo::add_access(acc), err)
  
  // Note: following works assuming add_access, get_to_prefetch, add_access, ...
  if (last_predicted_acc_v.empty() ||
      std::find(last_predicted_acc_v.begin(), last_predicted_acc_v.end(), acc) != last_predicted_acc_v.end() )
    return 0;
  
  int palgo_id = 0;
  for (std::vector<float>::iterator it = palgo_id__weight_v.begin(); it != palgo_id__weight_v.end(); it++, palgo_id++) {
    std::vector<ACC_T>& t_last_predicted_acc_v = *palgo_id__last_predicted_acc_v_v[palgo_id];
    if (std::find(t_last_predicted_acc_v.begin(), t_last_predicted_acc_v.end(), acc) == t_last_predicted_acc_v.end() )
      *it += (float)log2f(beta);
      // *it *= beta;
  }
  
  return 0;
}

int MJMPAlgo::add_access(float acc_time, ACC_T acc)
{
  int err;
  return_if_err(MPAlgo::add_access(acc_time, acc), err)
  
  // Note: following works assuming add_access, get_to_prefetch, add_access, ...
  if (last_predicted_acc_v.empty() ||
      std::find(last_predicted_acc_v.begin(), last_predicted_acc_v.end(), acc) != last_predicted_acc_v.end() )
    return 0;
  
  int palgo_id = 0;
  for (std::vector<float>::iterator it = palgo_id__weight_v.begin(); it != palgo_id__weight_v.end(); it++, palgo_id++) {
    std::vector<ACC_T>& t_last_predicted_acc_v = *palgo_id__last_predicted_acc_v_v[palgo_id];
    if (std::find(t_last_predicted_acc_v.begin(), t_last_predicted_acc_v.end(), acc) == t_last_predicted_acc_v.end() )
      *it += (float)log2f(beta);
      // *it *= beta;
  }
  
  return 0;
}

// Predicts next num_acc accesses and fills acc_v accordingly by the majority vote -- starts predicting
// from the most-voted acc, and continuing with the less-voted ones
int MJMPAlgo::get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                              const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  std::map<ACC_T, float> acc__sum_of_weights_map;
  for (int i = 0; i < palgo_v.size(); i++) {
    std::vector<ACC_T>& t_acc_v = *palgo_id__last_predicted_acc_v_v[i];
    t_acc_v.clear();
    std::vector<ACC_T> t_eacc_v;
    int t_num_acc = num_acc;
    return_if_err(palgo_v[i]->get_to_prefetch(num_acc, t_acc_v, std::vector<ACC_T>(), t_eacc_v), err)
    // for (std::vector<ACC_T>::iterator it = t_acc_v.begin(); it != t_acc_v.end(); it++) {
    //   if (acc__sum_of_weights_map.count(*it) == 0)
    //     acc__sum_of_weights_map[*it] = palgo_id__weight_v[i];
    //   else
    //     acc__sum_of_weights_map[*it] += palgo_id__weight_v[i];
    // }
  }
  // log_(INFO, "acc__sum_of_weights_map= \n" << patch::map_to_str<>(acc__sum_of_weights_map) )
  // std::map<float, ACC_T> sum_of_weights__acc_map;
  // for (std::map<ACC_T, float>::iterator it = acc__sum_of_weights_map.begin(); it != acc__sum_of_weights_map.end(); it++)
  //   sum_of_weights__acc_map[it->second] = it->first;
  
  // for (std::map<float, ACC_T>::reverse_iterator rit = sum_of_weights__acc_map.rbegin(); rit != sum_of_weights__acc_map.rend(); rit++) {
  //   acc_v.push_back(rit->second);
  //   if (acc_v.size() == num_acc)
  //     break;
  // }
  int palgo_id = std::distance(palgo_id__weight_v.begin(),
                               std::max_element(palgo_id__weight_v.begin(), palgo_id__weight_v.end() ) );
  palgo_id_used_v.push_back(palgo_id);
  acc_v = *palgo_id__last_predicted_acc_v_v[palgo_id];
  num_acc = acc_v.size();
  last_predicted_acc_v = acc_v;
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}

int MJMPAlgo::get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                              const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v)
{
  int err;
  std::map<ACC_T, float> acc__sum_of_weights_map;
  for (int i = 0; i < palgo_v.size(); i++) {
    std::vector<ACC_T>& t_acc_v = *palgo_id__last_predicted_acc_v_v[i];
    t_acc_v.clear();
    std::vector<ACC_T> t_eacc_v;
    int t_num_acc = num_acc;
    return_if_err(palgo_v[i]->get_to_prefetch(_time, num_acc, t_acc_v, std::vector<ACC_T>(), t_eacc_v), err)
  }
  int palgo_id = std::distance(palgo_id__weight_v.begin(),
                               std::max_element(palgo_id__weight_v.begin(), palgo_id__weight_v.end() ) );
  palgo_id_used_v.push_back(palgo_id);
  acc_v = *palgo_id__last_predicted_acc_v_v[palgo_id];
  num_acc = acc_v.size();
  last_predicted_acc_v = acc_v;
  // 
  for (std::set<ACC_T>::iterator it = acc_s.begin(); it != acc_s.end(); it++) {
    if (std::find(cached_acc_v.begin(), cached_acc_v.end(), *it) == cached_acc_v.end() && 
        std::find(acc_v.begin(), acc_v.end(), *it) == acc_v.end() )
      eacc_v.push_back(*it);
  }
  
  return 0;
}
