#ifndef _DS_DRIVE_H_
#define _DS_DRIVE_H_

#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <unistd.h>
#include <string>
#include <sys/time.h>

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

extern "C" {
  #include "dataspaces.h"
}

typedef boost::function<void(char*)> function_cb_on_get;

class DSDriver
{
  private:
    int app_id, num_peers;
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
    boost::mutex dspaces_sync_put_mtx;
    
    int get_flag;
    int get__flag;
    int sync_put_flag;
    //
    std::map<std::string, function_cb_on_get> varname_cbonget_map;
    std::vector<boost::shared_ptr<boost::thread> > riget_thread_v;
  public:
    DSDriver(int app_id, int num_peers);
    DSDriver(int app_id, int num_peers, MPI_Comm mpi_comm);
    ~DSDriver();
    int close();
    int init(int num_peers, int app_id);
    int sync_put(const char* var_name, unsigned int ver, int size,
                 int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get_(const char* var_name, unsigned int ver, int size,
             int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(const char* var_name, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int sync_put_without_lock(const char* var_name, unsigned int ver, int size,
                              int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
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