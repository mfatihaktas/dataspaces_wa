#ifndef _PALGO_H_
#define _PALGO_H_

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <utility> // std::pair
#include <limits> // std::numeric_limits
#include <cmath> // log2

#include <boost/lexical_cast.hpp>
#include <boost/math/distributions.hpp>

#include "markov.h"
#include "sfc.h"

/**********************************************  PAlgo  *******************************************/
typedef char PALGO_T;
const PALGO_T TALGO = 't';

typedef int ACC_T; // Note: ACC_T is a "more sounding" substitute for KEY_T in Markov
const ACC_T NONE_ACC = -1;
typedef std::pair<ACC_T, int> acc_step_pair;
typedef std::pair<float, ACC_T> arr_time__acc_pair;
class PAlgo { // Prefetching
  protected:
    std::set<ACC_T> acc_s;
    std::vector<ACC_T> acc_v;
  public:
    PAlgo() {};
    ~PAlgo() {};
    virtual std::string to_str();
    
    std::vector<ACC_T> get_acc_v();
    
    virtual void reset();
    virtual int train(std::vector<ACC_T> acc_v) = 0;
    virtual int train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v) = 0;
    virtual int add_access(ACC_T acc);
    // virtual int add_access(float acc_time, ACC_T acc) { return add_access(acc); }
    virtual int add_access(float acc_time, ACC_T acc);
    virtual int get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map) = 0;
    virtual int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                                const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) = 0;
    virtual int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                                const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) = 0;
};

/**********************************************  TAlgo  *******************************************/
typedef std::pair<float, float> time_time_pair;
typedef std::pair<float, float> mean_var_pair;
const int NO_INTER_TIME = 0;
const float MIN_INTER_TIME_FOR_PREDICTION = 1;
const float MIN_VARIANCE = 20;

class TAlgo : public PAlgo { // Time
  private:
    std::map<ACC_T, std::vector<time_time_pair> > acc___arr__inter_time_pair_v_map;
    
    std::map<ACC_T, mean_var_pair> acc__mean_var_pair_map;
  public:
    TAlgo();
    ~TAlgo();
    std::string to_str();
    
    void reset();
    int train(std::vector<ACC_T> acc_v) { return 1; }
    int train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v);
    int add_access(ACC_T acc) { return 1; }
    int add_access(float acc_time, ACC_T acc);
    int get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map) { return 1; }
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) { return 1; }
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
};

/**********************************************  MAlgo  *******************************************/
class MAlgo : public PAlgo { // Markov
  protected:
    ParseTree parse_tree;
  public:
    MAlgo(PALGO_T malgo_t, int context_size);
    ~MAlgo();
    void reset();
    
    std::string parse_tree_to_pstr();
    int train(std::vector<ACC_T> acc_v);
    int train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v) { return 1; }
    int add_access(ACC_T acc);
    int add_access(float acc_time, ACC_T acc) { return add_access(acc); }
    int get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map);
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) {
      return get_to_prefetch(num_acc, acc_v, cached_acc_v, eacc_v);
    };
};

class LZAlgo : public MAlgo {
  public:
    LZAlgo();
    ~LZAlgo();
};

class ALZAlgo : public MAlgo {
  public:
    ALZAlgo();
    ~ALZAlgo();
};

class PPMAlgo : public MAlgo {
  public:
    PPMAlgo(int context_size);
    ~PPMAlgo();
};

class POAlgo : public MAlgo {
  public:
    POAlgo();
    ~POAlgo();
};

/*************************************  MPAlgo : PAlgo  *******************************************/
typedef std::pair<PALGO_T, int> palgo_t__context_size_pair;
class MPAlgo : public PAlgo { // Mixed
  protected:
    std::vector<palgo_t__context_size_pair> palgo_t__context_size_v;
    
    std::vector<boost::shared_ptr<PAlgo> > palgo_v;
  public:
    MPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v);
    ~MPAlgo();
    virtual std::string to_str();
    void reset();
    int get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map) { return 1; }
    
    int train(std::vector<ACC_T> acc_v);
    int train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v) { return 1; }
    virtual int add_access(ACC_T acc);
    virtual int add_access(float acc_time, ACC_T acc);
    virtual int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                                const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) = 0;
    virtual int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                                const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) = 0;
};

/***************************************  WMPAlgo : MPAlgo  ***************************************/
class WMPAlgo : public MPAlgo { // Weight
  private:
    std::vector<float> palgo_id__weight_v;
  public:
    WMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v,
            std::vector<float> palgo_id__weight_v);
    ~WMPAlgo();
    std::string to_str();
    
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) { return 1; }
};

/***************************************  MMPAlgo : MPAlgo  ***************************************/
class MMPAlgo : public MPAlgo { // Max
  private:
    
  public:
    MMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v);
    ~MMPAlgo();
    std::string to_str();
    
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) { return 1; }
};

/***************************************  BMPAlgo : MPAlgo  ***************************************/
class BMPAlgo : public MPAlgo { // Best
  private:
    int window_size;
    
    std::vector<boost::shared_ptr<patch::Queue<int> > > palgo_id__score_queue_v;
    std::vector<boost::shared_ptr<std::vector<ACC_T> > > palgo_id__last_predicted_acc_v_v;
  public:
    BMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v, int window_size);
    ~BMPAlgo();
    std::string to_str();
    
    int add_access(ACC_T acc);
    int get_malgo_score(int malgo_id);
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) { return 1; }
};

/**************************************  MJMPAlgo : MPAlgo  ***************************************/
// template<class T>
// bool compare_pair_w_second(const T &x, const T &y) {
//   return x.second < y.second; 
// }
// std::map<ACC_T, float>::iterator it = std::max_element(
//   acc__sum_of_weights_map.begin(), acc__sum_of_weights_map.end(),
//   compare_pair_w_second<std::pair<ACC_T, int> >);

const int NONE_PALGO = -1;
class MJMPAlgo : public MPAlgo { // Majority
  private:
    float beta;
    
    std::vector<ACC_T> last_predicted_acc_v;
    std::vector<boost::shared_ptr<std::vector<ACC_T> > > palgo_id__last_predicted_acc_v_v;
    std::vector<float> palgo_id__weight_v;
    
    std::vector<int> palgo_id_used_v;
  public:
    MJMPAlgo(std::vector<palgo_t__context_size_pair> palgo_t__context_size_v,
             float beta);
    ~MJMPAlgo();
    std::string to_str();
    int add_access(ACC_T acc);
    int add_access(float acc_time, ACC_T acc);
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
};

#endif // _PALGO_H_