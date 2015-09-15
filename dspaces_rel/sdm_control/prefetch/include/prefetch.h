#ifndef _PREFETCH_H_
#define _PREFETCH_H_

#include "markov.h"
#include "sfc.h"

/********************************************  Cache  **********************************************/
template <typename ACC_T, typename T>
class Cache {
  typedef boost::function<void(T) > func_handle_cache_data_del_cb;
  private:
    int cache_size;
    func_handle_cache_data_del_cb handle_data_del_cb;
    
    std::deque<T> cache;
    
    patch_all::thread_safe_map<T, ACC_T> e_acc_map;
    patch_all::thread_safe_map<ACC_T, int> acc_num_map;

  public:
    Cache(int cache_size, func_handle_cache_data_del_cb handle_data_del_cb = 0)
    : cache_size(cache_size), handle_data_del_cb(handle_data_del_cb)
    {
      LOG(INFO) << "Cache:: constructed.";
    }
    
    ~Cache() { LOG(INFO) << "Cache:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "cache_size= " << cache_size << "\n";
      // ss << "acc_num_map= \n" << patch_all::map_to_str<ACC_T, int>(acc_num_map) << "\n";
      ss << "acc_num_map= \n" << acc_num_map.to_str() << "\n";
      ss << "cache_content= \n";
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << "> \n";
      ss << "\n";
      
      return ss.str();
    }
    
    int push(ACC_T acc, T e)
    {
      if (contains(e) )
        return 1;
      else if (cache.size() == cache_size)
        pop();
      
      if (!acc_num_map.contains(acc) )
        acc_num_map[acc] = 0;
      acc_num_map[acc] += 1;
      
      e_acc_map[e] = acc;
      
      cache.push_back(e);
      
      return 0;
    }
    
    int pop()
    {
      T e_to_del = cache.front();
      return del(e_acc_map[e_to_del], e_to_del);
    }
    
    int del(ACC_T acc, T e)
    {
      if (!contains(e) )
        return 1;
      
      if (handle_data_del_cb != 0)
        handle_data_del_cb(e);
      
      acc_num_map[acc] -= 1;
      // e_acc_map.erase(e_acc_map.find(e) );
      e_acc_map.del(e);
      cache.erase(std::find(cache.begin(), cache.end(), e) );
      
      return 0;
    }
    
    bool contains(T e)
    {
      return (std::find(cache.begin(), cache.end(), e) != cache.end() );
    }
    
    int size() { return cache.size(); }
    
    std::vector<T> get_content_v()
    {
      std::vector<T> v;
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        v.push_back(*it);
      
      return v;
    }
    
    std::vector<ACC_T> get_cached_acc_v()
    {
      std::vector<ACC_T> acc_v;
      for (typename std::map<ACC_T, int>::iterator it = acc_num_map.begin(); it != acc_num_map.end(); it++) {
        if (it->second > 0)
          acc_v.push_back(it->first);
      }
      
      return acc_v;
    }
};

/******************************************  MPBuffer  ********************************************/
const int NULL_P_ID = -1;

typedef char DATA_ACT_T;
const DATA_ACT_T DATA_ACT_DEL = 'd';
const DATA_ACT_T DATA_ACT_PREFETCH = 'p';
typedef boost::function<void(DATA_ACT_T, char, key_ver_pair) > func_handle_mpbuffer_data_act_cb;

// Note: Mapping between app_id and <key, ver> will be given to MPBuffer
class MPBuffer { // Markov Prefetching
  private:
    char ds_id;
    int max_num_key_ver; // # of <key, ver>
    int app_context_size;
    bool w_prefetch;
    func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb;
    
    patch_all::thread_safe_map<int, std::deque<key_ver_pair> >  p_id__reged_kv_deq_map;
    patch_all::thread_safe_map<int, int> p_id__front_step_in_deq_map;
    patch_all::thread_safe_map<key_ver_pair, int> kv__p_id_map;
    
    patch_all::thread_safe_map<int, int> p_id__last_acced_step_map;
    patch_all::thread_safe_map<int, int> p_id__last_cached_step_map;
    // Mostly for checking and logging
    patch_all::thread_safe_vector<key_ver_pair> reged_kv_v;
    patch_all::thread_safe_vector<key_ver_pair> acced_kv_v;
    patch_all::thread_safe_map<int, std::vector<key_ver_pair> > app_id__acced_kv_v_map;
    
    Cache<ACC_T, key_ver_pair> cache;
    boost::shared_ptr<MAlgo> malgo_to_pick_app_;
    boost::mutex add_acc_mutex;
  public:
    MPBuffer(char ds_id, int max_num_key_ver, MALGO_T malgo_t,
             bool w_prefetch, func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act = 0);
    ~MPBuffer();
    std::string to_str();
    
    int reg_key_ver(int p_id, key_ver_pair kv);
    
    int handle_data_del(key_ver_pair kv);
    int add_access(key_ver_pair kv);
    int get_to_prefetch(int& num_keys, std::vector<key_ver_pair>& kv_v);
    bool contains(key_ver_pair kv);
    std::vector<key_ver_pair> get_kv_v();
    
    void sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> kv_v, 
                               float& hit_rate, std::vector<char>& accuracy_v);
};

/******************************************  MWASpace  ********************************************/
typedef boost::function<void(DATA_ACT_T, char, key_ver_pair, lcoor_ucoor_pair) > func_handle_data_act_cb;

class WASpace {
  protected:
    std::vector<char> ds_id_v;
    func_handle_data_act_cb handle_data_act_cb;
    
    patch_all::thread_safe_map<int, char> app_id__ds_id_map;
  public:
    WASpace(std::vector<char> ds_id_v,
            MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~WASpace();
    virtual std::string to_str();
    
    int reg_ds(char ds_id);
    int reg_app(int app_id, char ds_id);
    
    virtual int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '') = 0;
    virtual int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v) = 0;
    virtual int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_) = 0;
    // virtual int get(bool blocking, int c_id, char& get_type,
    //                 std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_) = 0;
};

/******************************************  MWASpace  ********************************************/
class MWASpace : WASpace {
  private:
    patch_all::thread_safe_vector<key_ver_pair> kv_v;
    patch_all::thread_safe_map<char, boost::shared_ptr<patch_all::thread_safe_vector<key_ver_pair> > > ds_id__kv_vp_map;
    std::map<char, boost::shared_ptr<MPBuffer> >  ds_id__mpbuffer_map;
    
    patch_all::thread_safe_map<key_ver_pair, int> kv__p_id_map;
    
    patch_all::thread_safe_map<key_ver_pair, lcoor_ucoor_pair> kv__lucoor_map;
    // boost::mutex get_mutex;
  public:
    MWASpace(std::vector<char> ds_id_v,
             MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~MWASpace();
    std::string to_str();
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '');
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    bool contains(char ds_id, key_ver_pair kv);
    void handle_mpbuffer_data_act(DATA_ACT_T mpbuffer_data_act_t, char ds_id, key_ver_pair kv);
};

/******************************************  SWASpace  ********************************************/
typedef char SPREDICTOR_T;
const SPREDICTOR_T HILBERT_SPREDICTOR = 'h';

class SWASpace : WASpace {
  private:
    func_handle_data_act_cb handle_data_act_cb;
  
    boost::shared_ptr<QTable<char> > qtable_;
    
    // std::vector<box_t> acced_box_v;
    boost::shared_ptr<Predictor> predictor_;
  public:
    SWASpace(std::vector<char> ds_id_v,
             SPREDICTOR_T spredictor_t, int sexpand_length, COOR_T* lcoor_, COOR_T* ucoor_,
             bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~SWASpace();
    std::string to_str();
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '');
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
};

#endif // _PREFETCH_H_