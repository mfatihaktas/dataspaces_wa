#ifndef _PALGO_H_
#define _PALGO_H_

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <utility> // std::pair

#include <boost/lexical_cast.hpp>
#include <boost/math/distributions.hpp>

/**********************************************  PAlgo  *******************************************/
typedef int ACC_T; // Note: ACC_T is a "more sounding" substitute for KEY_T in Markov
typedef std::pair<ACC_T, int> acc_step_pair;
typedef std::pair<float, ACC_T> arr_time__acc_pair;
class PAlgo { // Prefetching
  protected:
    std::set<ACC_T> acc_s;
    std::vector<ACC_T> acc_v;
  public:
    PAlgo();
    ~PAlgo();
    virtual std::string to_str();
    
    std::vector<ACC_T> get_acc_v();
    
    virtual void reset();
    virtual int train(std::vector<ACC_T> acc_v) = 0;
    virtual int train(std::vector<arr_time__acc_pair> arr_time__acc_pair_v) = 0;
    virtual int add_access(ACC_T acc);
    virtual int add_access(float acc_time, ACC_T acc) = 0;
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
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v) { return 1; }
    int get_to_prefetch(float _time, int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
};

#endif // _PALGO_H_