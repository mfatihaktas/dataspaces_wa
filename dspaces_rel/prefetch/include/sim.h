#ifndef _SIM_H_
#define _SIM_H_

// #include <boost/math/distributions/exponential.hpp>
// #include <math.h>
#include <cstdlib>

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
    std::vector<char> ds_id_v;
    PREFETCH_T prefetch_t;
    
    patch_pre::thread_safe_vector<T> wa_content_v;
    patch_pre::thread_safe_map<char, boost::shared_ptr<patch_pre::thread_safe_vector<T> > > ds_id__content_vp_map;
    std::map<char, boost::shared_ptr<PBuffer> >  ds_id__pbuffer_map;
    
    patch_pre::thread_safe_map<int, char> app_id__ds_id_map;
    
    patch_pre::thread_safe_map<key_ver_pair, int> key_ver__p_id_map;
    
    patch_pre::syncer<T> putget_syncer;
    
    boost::mutex get_mutex;
  public:
    WASpace(std::vector<char> ds_id_v, int pbuffer_size, PREFETCH_T prefetch_t)
    : ds_id_v(ds_id_v),
      prefetch_t(prefetch_t)
    { 
      for (std::vector<char>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
        // std::vector<T> content_v;
        // ds_id__content_vp_map[*it] = content_v;
        ds_id__content_vp_map[*it] = boost::make_shared<patch_pre::thread_safe_vector<T> >();
        
        ds_id__pbuffer_map[*it] = boost::make_shared<PBuffer>(*it, pbuffer_size, prefetch_t,
                                                              true, boost::bind(&WASpace::handle_prefetch, this, _1, _2),
                                                              boost::bind(&WASpace::handle_del, this, _1) );
      }
      // 
      LOG(INFO) << "WASpace:: constructed."; 
    }
    ~WASpace() { LOG(INFO) << "WASpace:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      // ss << "num_ds= " << boost::lexical_cast<std::string>(num_ds) << "\n";
      // ss << "app_id__ds_id_map= \n";
      // for (std::map<int, char>::iterator it = app_id__ds_id_map.begin(); it != app_id__ds_id_map.end(); it++) {
      //   ss << "\t" << boost::lexical_cast<std::string>(it->first) << " : " << boost::lexical_cast<std::string>(it->second) << "\n";
      // }
      
      // ss << "ds_id__content_vp_map= \n";
      // typename std::map<char, std::vector<T> >::iterator map_it;
      // for (map_it = ds_id__content_vp_map.begin(); map_it != ds_id__content_vp_map.end(); map_it++) {
      //   ss << "\tds_id= " << boost::lexical_cast<std::string>(map_it->first) << "\n";
      //   for (typename std::vector<T>::iterator vec_it = (map_it->second).begin(); vec_it != (map_it->second).end(); vec_it++) {
      //     ss << "\t\t<" << boost::lexical_cast<std::string>(vec_it->first) << ", "
      //       << boost::lexical_cast<std::string>(vec_it->second) << "> \n ";
      //   }
      //   ss << "\n";
      // }
      
      ss << "ds_id__pbuffer_map= \n";
      for (std::map<char, boost::shared_ptr<PBuffer> >::iterator map_it = ds_id__pbuffer_map.begin(); map_it != ds_id__pbuffer_map.end(); map_it++) {
        ss << "*** ds_id= " << boost::lexical_cast<std::string>(map_it->first) << "\n"
           << "pbuffer= \n" << (map_it->second)->to_str();
      }
      
      std::vector<T> intersect_v;
      for (std::vector<char>::iterator it = ds_id_v.begin(); it != ds_id_v.end(); it++) {
        patch_pre::thread_safe_vector<T>& ds_content_v = *(ds_id__content_vp_map[*it] );
        std::sort(ds_content_v.begin(), ds_content_v.end() );
        
        std::vector<T> pbuffer_content_v = ds_id__pbuffer_map[*it]->get_content_v();
        std::sort(pbuffer_content_v.begin(), pbuffer_content_v.end() );
        
        std::vector<T> intersect_vec;
        std::set_intersection(ds_content_v.begin(), ds_content_v.end(),
                              pbuffer_content_v.begin(), pbuffer_content_v.end(), 
                              back_inserter(intersect_vec) );
        
        ss << "ds_id= " << boost::lexical_cast<std::string>(*it) << "; ";
        if (intersect_vec.empty() )
          ss << "No intersection between ds and pbuffer.\n";
        else
          ss << "INTERSECTION between ds and pbuffer.\n";
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
      ds_id__content_vp_map[ds_id]->push_back(kv);
      // Immediately broadcast it to every ds peer so pbuffers can be updated
      for (std::map<char, boost::shared_ptr<PBuffer> >::iterator it = ds_id__pbuffer_map.begin(); it != ds_id__pbuffer_map.end(); it++)
        (it->second)->reg_key_ver(p_id, kv);
      
      key_ver__p_id_map[kv] = p_id;
      
      wa_content_v.push_back(kv);
      putget_syncer.notify(kv);
      // LOG(INFO) << "put:: notified <key= " << kv.first << ", ver= " << kv.second << ">.";
      
      return 0;
    }
    
    int get(bool blocking, int c_id, T kv, char& get_type)
    {
      bool waited = false;
      if (!contains('*', kv) ) {
        if (blocking) {
          putget_syncer.add_sync_point(kv, 1);
          // LOG(INFO) << "get:: waiting for <key= " << kv.first << ", ver= " << kv.second << "> ...";
          putget_syncer.wait(kv);
          // LOG(INFO) << "get:: done waiting for <key= " << kv.first << ", ver= " << kv.second << ">.";
          putget_syncer.del_sync_point(kv);
          waited = true;
        }
        else {
          LOG(WARNING) << "get:: wa_space does not contain <key= " << kv.first << ", ver= " << kv.second << ">.";
          return 1;
        }
      }
      
      {
        boost::lock_guard<boost::mutex> guard(get_mutex);
        
        char ds_id = app_id__ds_id_map[c_id];
        if (contains(ds_id, kv) )
          get_type = 'l';
        else { // Remote fetch
          // LOG(INFO) << "get:: remote fetching to ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
          ds_id__content_vp_map[ds_id]->push_back(kv);
          if (waited)
            get_type = 'w';
          else
            get_type = 'r';
        }
        
        ds_id__pbuffer_map[ds_id]->add_access(key_ver__p_id_map[kv], kv);
      }
      
      return 0;
    }
    
    void handle_prefetch(char ds_id, T kv)
    {
      LOG(INFO) << "handle_prefetch:: prefetched to ds_id= " << ds_id << ", <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    void handle_del(T kv)
    {
      LOG(INFO) << "handle_del:: deleted <key= " << kv.first << ", ver= " << kv.second << ">.";
    }
    
    bool contains(char ds_id, T kv)
    {
      if (ds_id == '*')
        return wa_content_v.contains(kv);
        // return (std::find(wa_content_v.begin(), wa_content_v.end(), kv) != wa_content_v.end() );
      else {
        // patch_pre::thread_safe_vector<key_ver_pair>& content_v = *(ds_id__content_vp_map[ds_id] );
        return ds_id__content_vp_map[ds_id]->contains(kv) || ds_id__pbuffer_map[ds_id]->contains(kv);
        // return (std::find(content_v.begin(), content_v.end(), kv) != content_v.end() ) || ds_id__pbuffer_map[ds_id]->contains(kv);
      }
    }
};

/************************************************  PCSim  ********************************************/

typedef std::pair<std::string, unsigned int> key_ver_pair;

class PCSim { // Prefetching Simulator
  private:
    int num_p, num_c;
    std::vector<char> p_id__ds_id_v, c_id__ds_id_v;
    std::vector<int> p_id__num_put_v, c_id__num_get_v;
    std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
    std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;
    
    boost::shared_ptr<WASpace<key_ver_pair> > wa_space_;
    
    std::map<int, std::vector<char> > c_id__get_type_v_map;
    std::map<int, float> c_id__get_lperc_map;
    std::vector<boost::shared_ptr<boost::thread> > thread_v;
    
    
    void sim_p(int p_id);
    void sim_c(int c_id);
  public:
    PCSim(std::vector<char> ds_id_v, int pbuffer_size, PREFETCH_T prefetch_t,
          int num_p, int num_c,
          std::vector<char> p_id__ds_id_v, std::vector<char> c_id__ds_id_v,
          std::vector<int> p_id__num_put_v, std::vector<int> c_id__num_get_v,
          std::vector<float> p_id__put_rate_v, std::vector<float> c_id__get_rate_v,
          std::vector<std::vector<float> > p_id__inter_arr_time_v_v, std::vector<std::vector<float> > c_id__inter_arr_time_v_v );
    ~PCSim();
    std::string to_str();
    std::string to_str_end();
    
    void sim_all();
    std::map<int, float> get_c_id__get_lperc_map();
    void wait_for_threads();
};

#endif // _SIM_H_
