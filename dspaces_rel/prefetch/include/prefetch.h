#ifndef _PREFETCH_H_
#define _PREFETCH_H_

#include <deque>

#include "patch_pre.h"
#include "palgorithm.h"

typedef std::pair<std::string, unsigned int> key_ver_pair;
typedef boost::function<void(char, key_ver_pair)> func_handle_prefetch_cb;

class PBuffer { //Prefetching Buffer
  private:
    char ds_id;
    size_t buffer_size;
    bool with_prefetch;
    func_handle_prefetch_cb _handle_prefetch_cb;
    size_t app_context_size;
    
    patch_pre::thread_safe_map<int, std::deque<key_ver_pair> >  app_id__key_ver_deq_map;
    // Mostly for checking and logging
    patch_pre::thread_safe_vector<key_ver_pair> reged_key_ver_vec;
    patch_pre::thread_safe_vector<key_ver_pair> acced_key_ver_vec;
    patch_pre::thread_safe_map<int, std::vector<key_ver_pair> > app_id__acced_key_ver_vec_map;
    
    Cache<key_ver_pair> cache;
    
    // LZAlgo palgo;
    PPMAlgo ppm_algo_to_pick_app;
    
    int push(key_ver_pair kv);
    bool contains(key_ver_pair kv);
    int get_to_prefetch(size_t& num_keys, std::vector<key_ver_pair>& key_ver_vector);
    int prefetch();
  public:
    PBuffer(char ds_id, size_t buffer_size, 
            bool with_prefetch, func_handle_prefetch_cb _handle_prefetch_cb, 
            size_t app_context_size);
    ~PBuffer();
    std::string to_str();
    
    int reg_key_ver(int app_id, key_ver_pair kv);
    int add_access(int app_id, key_ver_pair kv);
};

#endif // _PREFETCH_H_