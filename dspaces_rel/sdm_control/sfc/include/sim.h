#ifndef _SIM_H_
#define _SIM_H_

#include "sfc.h"

/*******************************************  PCSim  **********************************************/
typedef std::pair<int, lcoor_ucoor_pair> app_id__lcoor_ucoor_pair;

class PCSim { // Prefetching Simulator
  private:
    std::vector<char> p_id__ds_id_v, c_id__ds_id_v;
    
    WASpace wa_space;
    
    std::map<int, std::vector<char> > c_id__get_type_v_map;
    std::map<int, float> c_id__get_lperc_map;

  public:
    PCSim(char predictor_t, std::vector<char> ds_id_v,
          int pbuffer_size, int pexpand_length,
          COOR_T* lcoor_, COOR_T* ucoor_,
          std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v);
    ~PCSim();
    std::string to_str();
    std::map<int, float> get_c_id__get_lperc_map();
    
    void sim(std::vector<app_id__lcoor_ucoor_pair>& p_id__lcoor_ucoor_pair_v,
             std::vector<app_id__lcoor_ucoor_pair>& c_id__lcoor_ucoor_pair_v);
};

#endif // _SIM_H_
