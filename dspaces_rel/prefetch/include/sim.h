#ifndef _SIM_H_
#define _SIM_H_

// #include <boost/math/distributions/exponential.hpp>
#include <math.h>
#include <stdlib.h>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <glog/logging.h>

#include "profiler.h"
#include "patch_pre.h"
#include "prefetch.h"

/************************************************  WASpace  ********************************************/
template <typename T>
class WASpace {
  private:
    int num_ds;
    size_t app_context_size;
    
    patch_pre::thread_safe_vector<T> wa_space_vec;
    patch_pre::thread_safe_map<char, std::vector<T> > ds_id__content_vec_map;
    std::map<char, boost::shared_ptr<PBuffer> >  ds_id__pbuffer_map;
    
    patch_pre::thread_safe_map<int, char> app_id__ds_id_map;
    
    patch_pre::thread_safe_map<key_ver_pair, int> key_ver__p_id_map;
    
    patch_pre::syncer<T> putget_syncer;
  public:
    WASpace(int num_ds, char* ds_id_, size_t pbuffer_size, size_t app_context_size)
    : num_ds(num_ds), 
      app_context_size(app_context_size)
    { 
      for (int i = 0; i < num_ds; i++) {
        char ds_id = ds_id_[i];
        
        std::vector<T> content_vec;
        ds_id__content_vec_map[ds_id] = content_vec;
        
        ds_id__pbuffer_map[ds_id] = boost::make_shared<PBuffer>(ds_id, pbuffer_size, app_context_size,
                                                                true, boost::bind(&WASpace::handle_prefetch, this, _1, _2) );
      
      }
      // 
      LOG(INFO) << "WASpace:: constructed."; 
    }
    ~WASpace() { LOG(INFO) << "WASpace:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "num_ds= " << boost::lexical_cast<std::string>(num_ds) << "\n";
      ss << "app_id__ds_id_map= \n";
      for (std::map<int, char>::iterator it = app_id__ds_id_map.begin(); it != app_id__ds_id_map.end(); it++) {
        ss << "\t" << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
      }
      
      ss << "ds_id__content_vec_map= \n";
      typename std::map<char, std::vector<T> >::iterator map_it;
      for (map_it = ds_id__content_vec_map.begin(); map_it != ds_id__content_vec_map.end(); map_it++) {
        ss << "\tds_id= " << boost::lexical_cast<std::string>(map_it->first) << "\n";
        for (typename std::vector<T>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
          ss << "\t\t<" << boost::lexical_cast<std::string>(vec_it->first) << ", "
             << boost::lexical_cast<std::string>(vec_it->second) << "> \n ";
        }
        ss << "\n";
      }
      
      // ss << "app_context_size= " << boost::lexical_cast<std::string>(app_context_size) << "\n";
      ss << "ds_id__pbuffer_map= \n";
      for (std::map<char, boost::shared_ptr<PBuffer> >::iterator map_it = ds_id__pbuffer_map.begin(); map_it != ds_id__pbuffer_map.end(); map_it++) {
        ss << "*** ds_id= " << boost::lexical_cast<std::string>(map_it->first) << "\n"
           << "pbuffer= \n" << (map_it->second)->to_str();
      }
      
      return ss.str();
    }
    
    int reg_app(int app_id, char ds_id)
    {
      if (app_id__ds_id_map.contains(app_id) ) {
        LOG(ERROR) << "reg_app:: already reged app_id= " << app_id;
        return 1;
      }
      app_id__ds_id_map[app_id] = ds_id;
      return 0;
    }
    
    // Note: syncer does not work properly when the add_sync_point and notify are called with 0 time interval
    int put(int p_id, T kv)
    {
      if (contains('*', kv) ) {
        LOG(ERROR) << "put:: already in wa_space; <key= " << kv.first << ", ver= " << kv.second << ">.";
        return 1;
      }
      char ds_id = app_id__ds_id_map[p_id];
      ds_id__content_vec_map[ds_id].push_back(kv);
      // Immediately broadcast it to every ds peer so pbuffers can be updated
      for (std::map<char, boost::shared_ptr<PBuffer> >::iterator it = ds_id__pbuffer_map.begin(); it != ds_id__pbuffer_map.end(); it++) {
        (it->second)->reg_key_ver(p_id, kv);
      }
      key_ver__p_id_map[kv] = p_id;
      
      wa_space_vec.push_back(kv);
      putget_syncer.notify(kv);
      // LOG(INFO) << "put:: notified <key= " << kv.first << ", ver= " << kv.second << ">.";
      
      return 0;
    }
    
    int get(bool blocking, int c_id, T kv, char& get_type)
    {
      if (!contains('*', kv) ) {
        if (blocking) {
          putget_syncer.add_sync_point(kv, 1);
          // LOG(INFO) << "get:: waiting for <key= " << kv.first << ", ver= " << kv.second << "> ...";
          putget_syncer.wait(kv);
          // LOG(INFO) << "get:: done waiting for <key= " << kv.first << ", ver= " << kv.second << ">.";
          putget_syncer.del_sync_point(kv);
        }
        else {
          LOG(WARNING) << "get:: wa_space does not contain <key= " << kv.first << ", ver= " << kv.second << ">.";
          return 1;
        }
      }
      
      char ds_id = app_id__ds_id_map[c_id];
      if (contains(ds_id, kv) ) {
        get_type = 'l';
      }
      else { // Remote fetch
        LOG(INFO) << "get:: remote fetching to ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
        ds_id__content_vec_map[ds_id].push_back(kv);
        get_type = 'r';
      }
      
      ds_id__pbuffer_map[ds_id]->add_access(key_ver__p_id_map[kv], kv);
      
      return 0;
    }
    
    void handle_prefetch(char ds_id, T kv)
    {
      LOG(INFO) << "handle_prefetch:: prefetched to ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    bool contains(char ds_id, T kv)
    {
      if (ds_id == '*')
        return (std::find(wa_space_vec.begin(), wa_space_vec.end(), kv) != wa_space_vec.end() );
      else {
        std::vector<key_ver_pair>& content_vec = ds_id__content_vec_map[ds_id];
        return (std::find(content_vec.begin(), content_vec.end(), kv) != content_vec.end() ) || ds_id__pbuffer_map[ds_id]->contains(kv);
      }
    }
};

/************************************************  PCSim  ********************************************/

typedef std::pair<std::string, unsigned int> key_ver_pair;

class PCSim { // Prefetching Simulator
  private:
    int num_p, num_c;
    std::vector<char> p_id__ds_id_vec, c_id__ds_id_vec;
    std::vector<int> p_id__num_put_vec, c_id__num_get_vec;
    std::vector<float> p_id__put_rate_vec, c_id__get_rate_vec;
    std::vector<std::vector<float> > p_id__inter_arr_time_vec_vec, c_id__inter_arr_time_vec_vec;
    
    boost::shared_ptr<WASpace<key_ver_pair> > wa_space_;
    
    std::map<int, std::vector<char> > c_id__get_type_vec_map;
    std::map<int, float> c_id__get_lperc_map;
    std::vector<boost::shared_ptr<boost::thread> > thread_vec;
    
    
    void sim_p(int p_id);
    void sim_c(int c_id);
  public:
    PCSim(int num_ds, char* ds_id_, size_t pbuffer_size, size_t app_context_size,
          int num_p, int num_c,
          std::vector<char> p_id__ds_id_vec, std::vector<char> c_id__ds_id_vec,
          std::vector<int> p_id__num_put_vec, std::vector<int> c_id__num_get_vec,
          std::vector<float> p_id__put_rate_vec, std::vector<float> c_id__get_rate_vec,
          std::vector<std::vector<float> > p_id__inter_arr_time_vec_vec, std::vector<std::vector<float> > c_id__inter_arr_time_vec_vec );
    ~PCSim();
    std::string to_str();
    std::string to_str_end();
    
    void sim_all();
    std::map<int, float> get_c_id__get_lperc_map();
    void wait_for_threads();
};

#endif // _SIM_H_
