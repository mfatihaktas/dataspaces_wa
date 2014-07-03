#ifndef _DSPACES_DRIVE_H_
#define _DSPACES_DRIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <map>
#include <unistd.h>

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

extern "C" {
  #include "dataspaces.h"
}

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
    
  private:
    int num_peers, appid;
    bool finalized;
    int nprocs, rank;
    MPI_Comm mpi_comm;
};

class TestClient
{
  public:
    TestClient(int num_peers, int app_id);
    ~TestClient();
    void put_test();
    void get_test();
    
    void dummy_put();
    void dummy_get();
  private:
    int num_peers, app_id;
    boost::shared_ptr<DSpacesDriver> ds_driver_;
};

#endif //end of _DSPACES_DRIVE_H_