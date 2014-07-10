#ifndef _DS_DRIVE_H_
#define _DS_DRIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <map>
#include <unistd.h>
#include <string>

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

extern "C" {
  #include "dataspaces.h"
}

typedef boost::function<void(char*)> function_cb_on_get;

class DSpacesDriver
{
  public:
    DSpacesDriver(int num_peers, int appid);
    ~DSpacesDriver();
    int finalize();
    int init(int num_peers, int appid);
    int sync_put(const char* var_name, unsigned int ver, int size,
                 int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data);
    int get(const char* var_name, unsigned int ver, int size,
            int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data);
    
    void lock_on_write(const char* var_name);
    void unlock_on_write(const char* var_name);
    void lock_on_read(const char* var_name);
    void unlock_on_read(const char* var_name);
    void reg_cb_on_get(std::string var_name, function_cb_on_get cb);
    void ri_get(std::string var_name, int size);
    void init_riget_thread(std::string var_name, int size);
  private:
    bool finalized;
    int num_peers, appid;
    int nprocs, rank;
    MPI_Comm mpi_comm;
    //
    std::map<std::string, function_cb_on_get> varname_cbonget_map;
    std::vector<boost::shared_ptr<boost::thread> > riget_thread_v;
};

#endif //end of _DS_DRIVE_H_