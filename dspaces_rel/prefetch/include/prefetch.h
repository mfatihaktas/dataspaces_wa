#ifndef _PREFETCH_H_
#define _PREFETCH_H_

#include <deque>
#include <stdlib.h>
#include <algorithm>

#include "patch_pre.h"
#include "palgorithm.h"
#include <boost/make_shared.hpp>

typedef std::pair<std::string, unsigned int> key_ver_pair;
typedef boost::function<void(char, key_ver_pair)> func_handle_prefetch_cb;
typedef boost::function<void(key_ver_pair)> func_handle_del_cb;

#define NULL_ACC -1

// Note: Mapping between app_id and <key, ver> will be given to PBuffer
class PBuffer { //Prefetching Buffer
  private:
    char ds_id;
    size_t buffer_size; // # of <key, ver>
    size_t app_context_size;
    bool w_prefetch;
    func_handle_prefetch_cb _handle_prefetch_cb;
    func_handle_del_cb _handle_del_cb;
    
    patch_pre::thread_safe_map<int, std::deque<key_ver_pair> >  p_id__reged_key_ver_deq_map;
    patch_pre::thread_safe_map<int, int> p_id__front_step_in_deq_map;
    
    patch_pre::thread_safe_map<int, int> p_id__last_acced_step_map;
    patch_pre::thread_safe_map<int, int> p_id__last_cached_step_map;
    // Mostly for checking and logging
    patch_pre::thread_safe_vector<key_ver_pair> reged_key_ver_v;
    patch_pre::thread_safe_vector<key_ver_pair> acced_key_ver_v;
    patch_pre::thread_safe_map<int, std::vector<key_ver_pair> > p_id__acced_key_ver_v_map;
    
    Cache<ACC_T, key_ver_pair> cache;
    
    boost::shared_ptr<PrefetchAlgo> algo_to_pick_app_;
    boost::mutex add_acc_mutex;
    
    int get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector);
  public:
    PBuffer(char ds_id, size_t buffer_size, PREFETCH_T prefetch_t,
            bool w_prefetch, func_handle_prefetch_cb _handle_prefetch_cb,
            func_handle_del_cb _handle_del_cb);
    ~PBuffer();
    std::string to_str();
    
    int reg_key_ver(int p_id, key_ver_pair kv);
    int add_access(int p_id, key_ver_pair kv);
    bool contains(key_ver_pair kv);
    std::vector<key_ver_pair> get_content_vec();
    
    void sim_prefetch_accuracy(std::vector<int> p_id_v, std::vector<key_ver_pair> key_ver_v, 
                               float& hit_rate, std::vector<char>& accuracy_v);
};

#endif // _PREFETCH_H_