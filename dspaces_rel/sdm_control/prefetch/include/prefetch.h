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
    // patch_all::thread_safe_map<ACC_T, std::vector<T> > acc__e_v_map;
    
    patch_all::thread_safe_map<T, ACC_T> e_acc_map;
    patch_all::thread_safe_map<ACC_T, int> acc_num_map;
    // patch_all::thread_safe_map<ACC_T, int> fixed_acc_num_map;
    // patch_all::thread_safe_map<ACC_T, int> fixed_acc_del_num_map;
    
    // patch_all::not_thread_safe_map<T, ACC_T> e_acc_map;
    // patch_all::not_thread_safe_map<ACC_T, int> acc_num_map;
    // patch_all::not_thread_safe_map<ACC_T, int> fixed_acc_num_map;
    // patch_all::not_thread_safe_map<ACC_T, int> fixed_acc_del_num_map;
    
  public:
    Cache(int cache_size, func_handle_cache_data_del_cb handle_data_del_cb = 0)
    : cache_size(cache_size), handle_data_del_cb(handle_data_del_cb)
    {
      // LOG(INFO) << "Cache:: constructed.";
    }
    
    ~Cache()
    {
      // LOG(INFO) << "Cache:: destructed."; 
    }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "cache_size= " << cache_size << "\n"
         << "acc_num_map= \n" << acc_num_map.to_str() << "\n"
        // << "fixed_acc_num_map= \n" << fixed_acc_num_map.to_str() << "\n"
        // << "fixed_acc_del_num_map= \n" << fixed_acc_del_num_map.to_str() << "\n"
         << "cache_content= " << patch_all::pdeque_to_str<>(cache) << "\n";
        // << "fixed_cache_content= " << patch_all::pdeque_to_str<>(fixed_cache) << "\n"
        // << "what_deled_cache_content= " << patch_all::pdeque_to_str<>(what_deled_cache) << "\n";
      // std::vector<std::string> fixed_cache_key_v;
      // for (typename std::deque<T>::iterator it = fixed_cache.begin(); it != fixed_cache.end(); it++)
      //   fixed_cache_key_v.push_back(it->first);
      // LOG(INFO) << "to_str:: will sort fixed_cache_key_v...";
      // std::sort(fixed_cache_key_v.begin(), fixed_cache_key_v.end() );
      // ss << "fixed_cache_key_v= \n";
      // for (typename std::vector<std::string>::iterator it = fixed_cache_key_v.begin(); it != fixed_cache_key_v.end(); it++)
      //   ss << *it << "\n";
      
      // std::vector<std::string> what_deled_cache_key_v;
      // for (typename std::deque<T>::iterator it = what_deled_cache.begin(); it != what_deled_cache.end(); it++)
      //   what_deled_cache_key_v.push_back(it->first);
      // LOG(INFO) << "to_str:: will sort what_deled_cache_key_v...";
      // std::sort(what_deled_cache_key_v.begin(), what_deled_cache_key_v.end() );
      // ss << "what_deled_cache_key_v= \n";
      // for (typename std::vector<std::string>::iterator it = what_deled_cache_key_v.begin(); it != what_deled_cache_key_v.end(); it++)
      //   ss << *it << "\n";
      
      // ss << "acc__e_v_map= \n";
      // for (typename std::map<ACC_T, std::vector<T> >::iterator map_it = acc__e_v_map.begin(); map_it != acc__e_v_map.end(); map_it++)
      //   ss << map_it->first << " : " << patch_all::pvec_to_str<>(map_it->second) << "\n";
      
      return ss.str();
    }
    
    int push(ACC_T acc, T e)
    {
      if (contains(e) )
        return 1;
      
      // boost::lock_guard<boost::mutex> guard(mutex);
      
      if (cache.size() == cache_size) {
        // LOG(INFO) << "push:: cache.size() == cache_size; cache_content= " << patch_all::pdeque_to_str<>(cache) << "\n";
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
    
    int pop()
    {
      // boost::lock_guard<boost::mutex> guard(mutex);
      
      T e;
      // {
      e = cache.front();
      // }
      // return del(e_acc_map[e], e);
      ACC_T acc = e_acc_map[e];
      if (!contains(e) )
        return 1;
      
      if (handle_data_del_cb != 0)
        handle_data_del_cb(e);
      
      acc_num_map[acc] -= 1;
      // fixed_acc_del_num_map[acc] += 1;
      
      e_acc_map.del(e);
      cache.erase(std::find(cache.begin(), cache.end(), e) );
      // what_deled_cache.push_back(e);
      
      return 0;
    }
    
    int del(ACC_T acc, T e)
    {
      if (!contains(e) )
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
    
    bool contains(T e)
    {
      // boost::lock_guard<boost::mutex> guard(mutex);
      return (std::find(cache.begin(), cache.end(), e) != cache.end() );
    }
    
    int size() 
    { 
      boost::lock_guard<boost::mutex> guard(mutex);
      return cache.size();
    }
    
    std::vector<T> get_content_v()
    {
      // boost::lock_guard<boost::mutex> guard(mutex);
      
      std::vector<T> v;
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        v.push_back(*it);
      
      return v;
    }
    
    std::vector<ACC_T> get_cached_acc_v()
    {
      // boost::lock_guard<boost::mutex> guard(mutex);
      
      std::vector<ACC_T> acc_v;
      for (typename std::map<ACC_T, int>::iterator it = acc_num_map.begin(); it != acc_num_map.end(); it++) {
        if (it->second > 0)
          acc_v.push_back(it->first);
      }
      
      return acc_v;
    }
};

template <typename MALGO>
void sim_prefetch_accuracy(MALGO& malgo,
                           int cache_size, std::vector<acc_step_pair> acc_step_v, 
                           float& hit_rate, std::vector<char>& accuracy_v )
{
  Cache<ACC_T, acc_step_pair> cache(cache_size, boost::function<void(acc_step_pair)>() );
  int num_miss = 0;
  
  std::map<ACC_T, int> acc__last_acced_step_map;
  
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    // std::cout << "sim_prefetch_accuracy:: is <" << it->first << ", " << it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    acc__last_acced_step_map[it->first] = it->second;
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wa-dataspaces scenario data is used only once
    cache.del(it->first, *it);
    
    malgo.add_access(it->first); // Reg only the acc
    
    int num_acc = 1; //cache_size;
    std::vector<ACC_T> acc_v, eacc_v;
    malgo.get_to_prefetch(num_acc, acc_v, std::vector<ACC_T>(), eacc_v);
    
    // Update cache
    for (std::vector<ACC_T>::iterator iit = acc_v.begin(); iit != acc_v.end(); iit++)
      cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
  }
  
  hit_rate = 1.0 - (float)num_miss / acc_step_v.size();
}

/******************************************  MPBuffer  ********************************************/
const int NULL_P_ID = -1;

typedef char PREFETCH_DATA_ACT_T;
const PREFETCH_DATA_ACT_T PREFETCH_DATA_ACT_DEL = 'd';
const PREFETCH_DATA_ACT_T PREFETCH_DATA_ACT_PREFETCH = 'p';
typedef boost::function<void(PREFETCH_DATA_ACT_T, char, key_ver_pair) > func_handle_mpbuffer_data_act_cb;

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
             bool w_prefetch, func_handle_mpbuffer_data_act_cb handle_mpbuffer_data_act_cb = 0);
    ~MPBuffer();
    std::string to_str();
    
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
typedef boost::function<void(PREFETCH_DATA_ACT_T, char, key_ver_pair, lcoor_ucoor_pair) > func_handle_data_act_cb;

class WASpace {
  protected:
    std::vector<char> ds_id_v;
    func_handle_data_act_cb handle_data_act_cb;
    
    patch_all::thread_safe_map<int, char> app_id__ds_id_map;
  public:
    WASpace(std::vector<char> ds_id_v, func_handle_data_act_cb handle_data_act_cb = 0);
    ~WASpace() {}
    virtual std::string to_str();
    
    virtual int reg_ds(char ds_id);
    int reg_app(int app_id, char ds_id);
    
    virtual int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '\0') = 0;
    virtual int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id) = 0;
    virtual int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v) = 0;
    virtual int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_) = 0;
};

/******************************************  MWASpace  ********************************************/
class MWASpace : public WASpace {
  private:
    int max_num_key_ver_in_mpbuffer;
    MALGO_T malgo_t;
    bool w_prefetch;
    
    patch_all::thread_safe_vector<key_ver_pair> kv_v;
    patch_all::thread_safe_map<char, boost::shared_ptr<patch_all::thread_safe_vector<key_ver_pair> > > ds_id__kv_vp_map;
    std::map<char, boost::shared_ptr<MPBuffer> >  ds_id__mpbuffer_map;
    
    patch_all::thread_safe_map<key_ver_pair, int> kv__p_id_map;
    patch_all::thread_safe_map<key_ver_pair, lcoor_ucoor_pair> kv__lucoor_map;
    
    // patch_all::thread_safe_map<int, std::vector<std::string> > p_id__key_map;
  public:
    MWASpace(std::vector<char> ds_id_v,
             MALGO_T malgo_t, int max_num_key_ver_in_mpbuffer, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~MWASpace();
    std::string to_str();
    std::string to_str_end();
    
    int reg_ds(char ds_id);
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '\0');
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id);
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
    
    bool contains(char ds_id, key_ver_pair kv);
    void handle_mpbuffer_data_act(PREFETCH_DATA_ACT_T mpbuffer_PREFETCH_data_act_t, char ds_id, key_ver_pair kv);
};

/******************************************  SWASpace  ********************************************/
class SWASpace : public WASpace {
  private:
    bool w_prefetch;
    
    boost::shared_ptr<QTable<char> > qtable_;
    
    // std::vector<box_t> acced_box_v;
    boost::shared_ptr<SAlgo> salgo_;
  public:
    SWASpace(std::vector<char> ds_id_v,
             SALGO_T salgo_t, COOR_T* lcoor_, COOR_T* ucoor_, int sexpand_length, bool w_prefetch, func_handle_data_act_cb handle_data_act_cb = 0);
    ~SWASpace() {}
    std::string to_str();
    
    int put(int p_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id = '\0');
    int del(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, char ds_id);
    int query(std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_, std::vector<char>& ds_id_v);
    int add_access(int c_id, std::string key, unsigned int ver, COOR_T* lcoor_, COOR_T* ucoor_);
};

#endif // _PREFETCH_H_