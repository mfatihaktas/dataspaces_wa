#ifndef _SIM_H_
#define _SIM_H_

#include <boost/math/distributions/exponential.hpp>
#include <math.h>
#include <stdlib.h>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "profiler.h"
#include "patch_pre.h"


template <typename T>
class Space {
    boost::mutex mutex;
  private:
    patch_pre::thread_safe_vector<T> space;
    patch_pre::syncer<T> putget_syncer;
  public:
    Space() { LOG(INFO) << "Space:: constructed."; }
    ~Space() { LOG(INFO) << "Space:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "space_content= ";
      for (typename std::vector<T>::iterator it = space.begin(); it != space.end(); it++) {
        ss << "<" << it->first << "," << it->second << ">, ";
      }
      ss << "\n";
      
      return ss.str();
    }
    
    // Note: syncer does not work properly when the add_sync_point and notify are called with 0 time interval
    int put(T kv)
    {
      if (contains(kv) ) {
        return 1;
      }
      space.push_back(kv);
      putget_syncer.notify(kv);
      // LOG(INFO) << "put:: notified <key= " << kv.first << ", ver= " << kv.second << ">.";
      
      return 0;
    }
    
    int get(bool blocking, T kv)
    {
      if (!contains(kv) ) {
        if (blocking) {
          putget_syncer.add_sync_point(kv, 1);
          LOG(INFO) << "get:: will wait for <key= " << kv.first << ", ver= " << kv.second << "> ...";
          putget_syncer.wait(kv);
          // LOG(INFO) << "get:: done waiting for <key= " << kv.first << ", ver= " << kv.second << ">.";
          putget_syncer.del_sync_point(kv);
        }
        else {
          return 1;
        }
      }
      return 0;
    }
    
    bool contains(T kv)
    {
      return (std::find(space.begin(), space.end(), kv) != space.end() );
    
    }
    size_t size() { return space.size(); }
};

typedef std::pair<std::string, unsigned int> key_ver_pair;

class PCSim { // Prefetching Simulator
  private:
    int num_p, num_c;
    std::vector<int> num_put_vec, num_get_vec;
    std::vector<float> put_rate_vec, get_rate_vec;
    
    Space<key_ver_pair> space;
    
  public:
    PCSim(int num_p, std::vector<int> num_put_vec, std::vector<float> put_rate_vec, 
          int num_c, std::vector<int> num_get_vec, std::vector<float> get_rate_vec );
    ~PCSim();
    std::string to_str();
    
    int sim_all();
    int sim_p(int p_id);
    int sim_c(int c_id);
};

#endif // _SIM_H_
