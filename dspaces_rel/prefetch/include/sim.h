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

template <typename T>
class WASpace {
    boost::mutex mutex;
  private:
    int num_ds;
    
    patch_pre::thread_safe_vector<T> wa_space_vec;
    std::map<char, std::vector<T> > ds_id__content_vec_map;
    std::map<char, boost::shared_ptr<PBuffer> >  ds_id__pbuffer_map;
    
    patch_pre::thread_safe_map<int, char> app_id__ds_id_map;
    
    patch_pre::syncer<T> putget_syncer;
  public:
    WASpace(int num_ds, char* ds_id_)
    : num_ds(num_ds)
    { 
      for (int i = 0; i < num_ds; i++) {
        char ds_id = ds_id_[i];
        
        // patch_pre::thread_safe_vector<T> content_vec;
        std::vector<T> content_vec;
        ds_id__content_vec_map[ds_id] = content_vec;
        
        ds_id__pbuffer_map[ds_id] = boost::make_shared<PBuffer>(ds_id, 2,
                                                                true, boost::bind(&WASpace::handle_prefetch, this, _1, _2), 2);
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
        ss << "\tds_id= " << boost::lexical_cast<std::string>(map_it->first);
        for (typename std::vector<T>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
          ss << "<" << boost::lexical_cast<std::string>(vec_it->first) << "," 
             << boost::lexical_cast<std::string>(vec_it->second) << ">, ";
        }
        ss << "\n";
      }
      
      return ss.str();
    }
    
    bool contains(T kv)
    {
      return (std::find(wa_space_vec.begin(), wa_space_vec.end(), kv) != wa_space_vec.end() );
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
    int put(int app_id, T kv)
    {
      if (contains(kv) ) {
        LOG(ERROR) << "put:: already in wa_space; <key= " << kv.first << ", ver= " << kv.second << ">.";
        return 1;
      }
      char ds_id = app_id__ds_id_map[app_id];
      ds_id__pbuffer_map[ds_id]->reg_key_ver(app_id, kv);
      
      ds_id__content_vec_map[ds_id].push_back(kv);
      wa_space_vec.push_back(kv);
      
      putget_syncer.notify(kv);
      // LOG(INFO) << "put:: notified <key= " << kv.first << ", ver= " << kv.second << ">.";
      
      return 0;
    }
    
    int get(bool blocking, int app_id, T kv, char& get_type)
    {
      if (!contains(kv) ) {
        if (blocking) {
          putget_syncer.add_sync_point(kv, 1);
          // LOG(INFO) << "get:: will wait for <key= " << kv.first << ", ver= " << kv.second << "> ...";
          putget_syncer.wait(kv);
          // LOG(INFO) << "get:: done waiting for <key= " << kv.first << ", ver= " << kv.second << ">.";
          putget_syncer.del_sync_point(kv);
        }
        else {
          return 1;
        }
      }
      char ds_id = app_id__ds_id_map[app_id];
      std::vector<key_ver_pair>& content_vec = ds_id__content_vec_map[ds_id];
      if (std::find(content_vec.begin(), content_vec.end(), kv) != content_vec.end() ) {
        get_type = 'l';
      }
      else { // remote fetch
        content_vec.push_back(kv);
        get_type = 'r';
      }
      
      ds_id__pbuffer_map[ds_id]->add_access(app_id, kv);
      
      return 0;
    }
    
    void handle_prefetch(char ds_id, T kv)
    {
      ds_id__content_vec_map[ds_id].push_back(kv);
      LOG(INFO) << "handle_prefetch:: prefetched to ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
};

typedef std::pair<std::string, unsigned int> key_ver_pair;

class PCSim { // Prefetching Simulator
  private:
    int num_p, num_c;
    std::vector<char> p_id__ds_id_vec, c_id__ds_id_vec;
    std::vector<int> p_id__num_put_vec, c_id__num_get_vec;
    std::vector<float> p_id__put_rate_vec, c_id__get_rate_vec;
    
    boost::shared_ptr<WASpace<key_ver_pair> > wa_space_;
    
    std::map<int, std::vector<char> > c_id__get_type_vec_map;
    std::vector<boost::shared_ptr<boost::thread> > thread_vec;
  public:
    PCSim(int num_ds, char* ds_id_,
          int num_p, std::vector<char> p_id__ds_id_vec, std::vector<int> p_id__num_put_vec, std::vector<float> p_id__put_rate_vec, 
          int num_c, std::vector<char> c_id__ds_id_vec, std::vector<int> c_id__num_get_vec, std::vector<float> c_id__get_rate_vec );
    ~PCSim();
    std::string to_str();
    
    int sim_all();
    int sim_p(int p_id);
    int sim_c(int c_id);
};

#endif // _SIM_H_
