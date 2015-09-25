#ifndef _SIM_H_
#define _SIM_H_

// #include <boost/math/distributions/exponential.hpp>
// #include <math.h>
#include <cstdlib>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <glog/logging.h>

#include "prefetch.h"

/************************************************  PCSim  *****************************************/
class PCSim { // P-C Simulator
  protected:
    bool w_prefetch;
    int num_p, num_c;
    std::vector<char> p_id__ds_id_v, c_id__ds_id_v;
    std::vector<int> p_id__num_put_v, c_id__num_get_v;
    std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
    std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;

    COOR_T *lcoor_, *ucoor_;
    boost::shared_ptr<WASpace> wa_space_;
    
    std::map<int, std::vector<char> > c_id__get_type_v_map;
    std::vector<boost::shared_ptr<boost::thread> > thread_v;
  public:
    PCSim(std::vector<char> ds_id_v, bool w_prefetch,
          int num_p, int num_c,
          std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
          std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
          std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
          std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v);
    ~PCSim();
    virtual std::string to_str();
    std::string to_str_end();
    
    void sim_all();
    virtual void sim_p(int p_id) = 0;
    virtual void sim_c(int c_id, bool blocking_get) = 0;
    
    std::map<int, float> get_c_id__get_lperc_map();
    
    void handle_data_act(DATA_ACT_T data_act_t, char ds_id, key_ver_pair kv, lcoor_ucoor_pair lucoor);
};

/************************************************  MPCSim  ****************************************/
class MPCSim : public PCSim { // Markov
  private:
    patch_all::syncer<key_ver_pair> bget_syncer;
    
  public:
    MPCSim(std::vector<char> ds_id_v, bool w_prefetch,
           int num_p, int num_c,
           std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
           std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
           std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
           std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
           MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer);
    ~MPCSim();
    std::string to_str();
    
    void sim_p(int p_id);
    void sim_c(int c_id, bool blocking_get);
};

/*******************************************  SPCSim  **********************************************/
typedef std::pair<int, lcoor_ucoor_pair> app_id__lcoor_ucoor_pair;

class SPCSim : public PCSim { // Spatial
  private:
    spatial_syncer s_syncer;
  public:
    SPCSim(std::vector<char> ds_id_v, int num_p, int num_c,
           std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
           std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
           std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
           std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v,
           SALGO_T salgo_t, int sexpand_length, COOR_T* lcoor_, COOR_T* ucoor_, bool w_prefetch);
    ~SPCSim() {}
    std::string to_str();
    
    void sim_p(int p_id);
    void sim_c(int c_id, bool blocking_get);
    void get_lucoor_to_acc_v(std::vector<lcoor_ucoor_pair>& lucoor_v);
};

#endif // _SIM_H_
