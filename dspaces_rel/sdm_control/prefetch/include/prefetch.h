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
    
    boost::mutex mutex;
    
    std::deque<T> cache;
    // std::deque<T> fixed_cache;
    // std::deque<T> what_deled_cache;
    // patch::thread_safe_map<ACC_T, std::vector<T> > acc__e_v_map;
    
    patch::thread_safe_map<T, ACC_T> e_acc_map;
    patch::thread_safe_map<ACC_T, int> acc_num_map;
    // patch::thread_safe_map<ACC_T, int> fixed_acc_num_map;
    // patch::thread_safe_map<ACC_T, int> fixed_acc_del_num_map;
    
    // patch::not_thread_safe_map<T, ACC_T> e_acc_map;
    // patch::not_thread_safe_map<ACC_T, int> acc_num_map;
    // patch::not_thread_safe_map<ACC_T, int> fixed_acc_num_map;
    // patch::not_thread_safe_map<ACC_T, int> fixed_acc_del_num_map;
    
  public:
    Cache(int cache_size, func_handle_cache_data_del_cb handle_data_del_cb = 0)
    : cache_size(cache_size), handle_data_del_cb(handle_data_del_cb)
    {
      // log_(INFO, "constructed.")
    }
    
    ~Cache() { /* log_(INFO, "destructed.") */ }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "cache_size= " << cache_size << "\n"
         << "acc_num_map= \n" << acc_num_map.to_str() << "\n"
        // << "fixed_acc_num_map= \n" << fixed_acc_num_map.to_str() << "\n"
        // << "fixed_acc_del_num_map= \n" << fixed_acc_del_num_map.to_str() << "\n"
         << "cache_content= " << patch::pdeque_to_str<>(cache) << "\n";
        // << "fixed_cache_content= " << patch::pdeque_to_str<>(fixed_cache) << "\n"
        // << "what_deled_cache_content= " << patch::pdeque_to_str<>(what_deled_cache) << "\n";
      // std::vector<std::string> fixed_cache_key_v;
      // for (typename std::deque<T>::iterator it = fixed_cache.begin(); it != fixed_cache.end(); it++)
      //   fixed_cache_key_v.push_back(it->first);
      // log_(INFO, "to_str:: will sort fixed_cache_key_v...")
      // std::sort(fixed_cache_key_v.begin(), fixed_cache_key_v.end() );
      // ss << "fixed_cache_key_v= \n";
      // for (typename std::vector<std::string>::iterator it = fixed_cache_key_v.begin(); it != fixed_cache_key_v.end(); it++)
      //   ss << *it << "\n";
      
      // std::vector<std::string> what_deled_cache_key_v;
      // for (typename std::deque<T>::iterator it = what_deled_cache.begin(); it != what_deled_cache.end(); it++)
      //   what_deled_cache_key_v.push_back(it->first);
      // log_(INFO, "to_str:: will sort what_deled_cache_key_v...")
      // std::sort(what_deled_cache_key_v.begin(), what_deled_cache_key_v.end() );
      // ss << "what_deled_cache_key_v= \n";
      // for (typename std::vector<std::string>::iterator it = what_deled_cache_key_v.begin(); it != what_deled_cache_key_v.end(); it++)
      //   ss << *it << "\n";
      
      // ss << "acc__e_v_map= \n";
      // for (typename std::map<ACC_T, std::vector<T> >::iterator map_it = acc__e_v_map.begin(); map_it != acc__e_v_map.end(); map_it++)
      //   ss << map_it->first << " : " << patch::pvec_to_str<>(map_it->second) << "\n";
      
      return ss.str();
    }
    
    int push(ACC_T acc, T e) {
      if (contains(e) )
        return 1;
      
      // boost::lock_guard<boost::mutex> guard(mutex);
      if (cache.size() == cache_size) {
        // log_(INFO, "cache.size() == cache_size; cache_content= " << patch::pdeque_to_str<>(cache) )
        pop();
      }
      
      acc_num_map[acc] += 1;
      // fixed_acc_num_map[acc] += 1;
      
      e_acc_map[e] = acc;
      
      cache.push_back(e);
      // fixed_cache.push_back(e);
      // if (!acc__e_v_map.contains(acc) ) {
      //   std::vector<T> e_v;
      //   acc__e_v_map[acc] = e_v;
      // }
      // acc__e_v_map[acc].push_back(e);
      
      return 0;
    }
    
    int pop() {
      // boost::lock_guard<boost::mutex> guard(mutex);
      T e;
      // {
      e = cache.front();
      // }
      int err;
      return_if_err(del(e_acc_map[e], e), err)
      // int err;
      // return_err_if_ret_cond_flag(contains(e), err, ==, 0, 1)
      
      // if (handle_data_del_cb != 0)
      //   handle_data_del_cb(e);
      
      // ACC_T acc = e_acc_map[e];
      // acc_num_map[acc] -= 1;
      
      // // fixed_acc_del_num_map[acc] += 1;
      
      // e_acc_map.del(e);
      // cache.erase(std::find(cache.begin(), cache.end(), e) );
      // // what_deled_cache.push_back(e);
      return 0;
    }
    
    int del(ACC_T acc, T e) {
      int err;
      // return_err_if_ret_cond_flag(contains(e), err, ==, 0, 1)
      if (!contains(e))
        return 1;
      
      if (handle_data_del_cb != 0)
        handle_data_del_cb(e);
      
      // boost::lock_guard<boost::mutex> guard(mutex);
      
      acc_num_map[acc] -= 1;
      // if (!fixed_acc_del_num_map.contains(acc) )
      //   fixed_acc_del_num_map[acc] = 0;
      // fixed_acc_del_num_map[acc] += 1;
      
      e_acc_map.del(e);
      cache.erase(std::find(cache.begin(), cache.end(), e) );
      // what_deled_cache.push_back(e);
      
      return 0;
    }
    
    bool contains(T e) {
      // boost::lock_guard<boost::mutex> guard(mutex);
      return (std::find(cache.begin(), cache.end(), e) != cache.end() );
    }
    
    int get_available_size() {
      boost::lock_guard<boost::mutex> guard(mutex);
      return (cache_size - cache.size() );
    }
    
    int size() { 
      boost::lock_guard<boost::mutex> guard(mutex);
      return cache.size();
    }
    
    std::vector<T> get_content_v() {
      // boost::lock_guard<boost::mutex> guard(mutex);
      std::vector<T> v;
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        v.push_back(*it);
      
      return v;
    }
    
    std::vector<ACC_T> get_cached_acc_v() {
      // boost::lock_guard<boost::mutex> guard(mutex);
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

typedef char PREFETCH_DATA_ACT_T;
const PREFETCH_DATA_ACT_T PREFETCH_DATA_ACT_DEL = 'd';
const PREFETCH_DATA_ACT_T PREFETCH_DATA_ACT_PREFETCH = 'p';
typedef boost::function<void(PREFETCH_DATA_ACT_T, int, key_ver_pair) > func_handle_mpbuffer_data_act_cb;

class MPBuffer { // Markov Prefetching
  private:
    int ds_id;
    int max_num_key_ver;
    int app_context_size;
    bool w_prefetch;
    func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb;
    
    patch::thread_safe_map<int, std::deque<key_ver_pair> >  p_id__reged_kv_deq_map;
    patch::thread_safe_map<int, int> p_id__front_step_in_deq_map;
    patch::thread_safe_map<key_ver_pair, int> kv__p_id_map;
    
    patch::thread_safe_map<int, int> p_id__last_acced_step_map;
    patch::thread_safe_map<int, int> p_id__last_cached_step_map;
    // Mostly for checking and logging
    patch::thread_safe_vector<key_ver_pair> reged_kv_v;
    patch::thread_safe_vector<key_ver_pair> acced_kv_v;
    patch::thread_safe_map<int, std::vector<key_ver_pair> > app_id__acced_kv_v_map;
    
    Cache<ACC_T, key_ver_pair> cache;
    boost::shared_ptr<MAlgo> malgo_to_pick_app_;
    boost::mutex add_acc_mutex;
  public:
    MPBuffer(int ds_id, int max_num_key_ver, MALGO_T malgo_t,
             bool w_prefetch, func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb = 0);
    ~MPBuffer();
    std::string to_str();
    
    // Note: Mapping between p_id and <key, ver> will be given to MPBuffer
    int reg_key_ver(int p_id, key_ver_pair kv);
    
    int del(key_ver_pair kv);
    int handle_data_del(key_ver_pair kv);
    int add_access(key_ver_pair kv);
    int get_to_prefetch(int& num_keys, std::vector<key_ver_pair>& kv_v);
    bool contains(key_ver_pair kv);
    std::vector<key_ver_pair> get_kv_v();
    
    void sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> kv_v, 
                               float& hit_rate, std::vector<char>& accuracy_v);
};

/******************************************  MWASpace  ********************************************/
typedef boost::function<void(PREFETCH_DATA_ACT_T, int, key_ver_pair, lcoor_ucoor_pair) > func_handle_data_act_cb;

class WASpace {
  protected:
    std::vector<int> ds_id_v;
    func_handle_data_act_cb handle_data_act_cb;
    
    patch::thread_safe_map<int, int> app_id__ds_id_map;
  public:
    WASpace(std::vector<int> ds_id_v, func_handle_data_act_cb handle_data_act_cb = 0);
    ~WASpace() {}
    virtual std::string to_str();
    
    virtual int reg_ds(int ds_id);
    int reg_app(int app_id, int ds_id);
    
    virtual int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id = -1) = 0;
    virtual int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id) = 0;
    virtual int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<int>& ds_id_v) = 0;
    virtual int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_) = 0;
};

/******************************************  MWASpace  ********************************************/
class MWASpace : public WASpace {
  private:
    int max_num_key_ver_in_mpbuffer;
    MALGO_T malgo_t;
    bool w_prefetch;
    
    patch::thread_safe_vector<key_ver_pair> kv_v;
    patch::thread_safe_map<int, boost::shared_ptr<patch::thread_safe_vector<key_ver_pair> > > ds_id__kv_map;
    std::map<int, boost::shared_ptr<MPBuffer> >  ds_id__mpbuffer_map;
    
    patch::thread_safe_map<key_ver_pair, int> kv__p_id_map;
    patch::thread_safe_map<key_ver_pair, lcoor_ucoor_pair> kv__lucoor_map;
    
    // patch::thread_safe_map<int, std::vector<std::string> > p_id__key_map;
  public:
    MWASpace(std::vector<int> ds_id_v,
             MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~MWASpace();
    std::string to_str();
    std::string to_str_end();
    
    int reg_ds(int ds_id);
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id = -1);
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<int>& ds_id_v);
    bool contains(int ds_id, key_ver_pair kv);
    
    void handle_mpbuffer_data_act(PREFETCH_DATA_ACT_T mpbuffer_PREFETCH_data_act_t, int ds_id, key_ver_pair kv);
};

/******************************************  SWASpace  ********************************************/
class SWASpace : public WASpace {
  private:
    bool w_prefetch;
    
    boost::shared_ptr<QTable<int> > qtable_;
    
    // std::vector<box_t> acced_box_v;
    boost::shared_ptr<SAlgo> salgo_;
  public:
    SWASpace(std::vector<int> ds_id_v,
             SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~SWASpace() {}
    std::string to_str();
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id = '\0');
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, int ds_id);
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<int>& ds_id_v);
    int get_to_fetch(COOR_T* lcoor_, COOR_T* ucoor_, std::vector<lcoor_ucoor_pair>& lucoor_to_fetch_v);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
};

#endif // _PREFETCH_H_