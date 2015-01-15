#ifndef _DS_DRIVE_H_
#define _DS_DRIVE_H_

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/time.h>
#include <vector>

extern "C" {
  #include "dataspaces.h"
}

class DSDriver
{
  public:
    DSDriver(int num_dscnodes, int app_id);
    DSDriver(MPI_Comm mpi_comm, int num_dscnodes, int app_id);
    ~DSDriver();
    int finalize();
    int init(int num_dscnodes, int app_id);
    int sync_put(const char* var_name, unsigned int ver, int size,
                 int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    int get(const char* var_name, unsigned int ver, int size,
            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_);
    void lock_on_write(const char* var_name);
    void unlock_on_write(const char* var_name);
    void lock_on_read(const char* var_name);
    void unlock_on_read(const char* var_name);
    
  private:
    bool finalized;
    int num_dscnodes, app_id;
    int nprocs, rank;
    MPI_Comm mpi_comm;
};

#endif //end of _DS_DRIVE_H_