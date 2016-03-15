#ifndef _DS_DRIVE_H_
#define _DS_DRIVE_H_

#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <unistd.h>
#include <string>
#include <sys/time.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <pthread.h>

#include "patch.h"
#include "patch_ds.h"

extern "C" {
  #include "dataspaces.h"
}

std::string get_data_id(const char* var_name, unsigned int ver, int ndim, uint64_t *lb_, uint64_t *ub_);

typedef boost::function<void(char*)> function_cb_on_get;

class DSDriver
{
  struct get_info {
    public:
      const char* var_name;
      unsigned int ver;
      int size, ndim;
      uint64_t *gdim_, *lb_, *ub_;
      void** data__;
      get_info(const char* var_name, unsigned int ver, int size,
               int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void** data__)
      : var_name(var_name), ver(ver), size(size),
        ndim(ndim), gdim_(gdim_), lb_(lb_), ub_(ub_), data__(data__)
      {}
      ~get_info() {}
  };
  
  private:
    int app_id, num_peer;
    MPI_Comm mpi_comm;
    
    bool closed;
    int nprocs, rank;
    
    struct timeval construct_time;
    struct timeval last_lock_time;
    
    boost::mutex property_mtx;
    boost::mutex dspaces_read_mtx;
    boost::mutex dspaces_write_mtx;
    
    boost::mutex dspaces_get__mtx;
    boost::mutex dspaces_get_mtx;
    // boost::recursive_mutex dspaces_get_mtx;
    boost::mutex dspaces_sync_put_mtx;
    // boost::mutex dspaces_put_get_mtx;
    // boost::timed_mutex dspaces_put_get_mtx;
    // pthread_mutex_t dspaces_put_get_mtx;
    // boost::mutex lock_mtx, unlock_mtx;
    // int putget_lock_unlock_count;
    bool get_done, get_success;
    patch::BQueue<boost::shared_ptr<get_info> > get_info_bq;
    patch::syncer<unsigned int> syncer;
    patch::thread_safe_map<unsigned int, int> hashed_data_id__get_return_map;
    boost::shared_ptr<boost::thread> get_poll_t_;
    
    int get_flag;
    int get__flag;
    int sync_put_flag;
    //
    std::map<std::string, function_cb_on_get> varname_cbonget_map;
    // std::vector<boost::shared_ptr<boost::thread> > riget_thread_v;
    
  public:
    static uint64_t get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_);
    
    DSDriver(int app_id, int num_peer);
    DSDriver(int app_id, int num_peer, MPI_Comm mpi_comm);
    ~DSDriver();
    int close();
    int init(int num_peer, int app_id);
    int sync_put(const char* var_name, unsigned int ver, int size,
                 int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    // int get_(const char* var_name, unsigned int ver, int size,
    //         int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int reg_get_wait_for_completion(const char* var_name, unsigned int ver, int size,
                                    int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get_poll();
    int timed_get(const char* var_name, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int plain_get(const char* var_name, unsigned int ver, int size,
                  int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(const char* var_name, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int sync_put_without_lock(const char* var_name, unsigned int ver, int size,
                              int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int del(const char* var_name, unsigned int ver);
    
    void lock_on_write(const char* var_name);
    void unlock_on_write(const char* var_name);
    void lock_on_read(const char* var_name);
    void unlock_on_read(const char* var_name);
    void reg_cb_on_get(std::string var_name, function_cb_on_get cb);
    void t_get(std::string var_name, int size);
    void init_get_thread(std::string var_name, int size);
    
    void do_timing(const char*);
    time_t get_inter_lock_time(struct timeval *result);
    void timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
    int refresh_last_lock_time();
};

#endif //end of _DS_DRIVE_H_