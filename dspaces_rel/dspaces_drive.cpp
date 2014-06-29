#include "dspaces_drive.h"

DSpacesDriver::DSpacesDriver()
{
  mpi_comm = MPI_COMM_WORLD;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  //
  
}

DSpacesDriver::~DSpacesDriver()
{
  dspaces_finalize();
  MPI_Barrier(mpi_comm);
  MPI_Finalize();
}

int DSpacesDriver::dspaces_init_(int num_peers, int appid)
{
  return dspaces_init(num_peers, appid, &mpi_comm, NULL);
}

int DSpacesDriver::sync_put(const char* var_name, unsigned int ver, int size,
                            int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data)
{
  
  dspaces_define_gdim (var_name, ndim, gdim);
  
  dspaces_lock_on_write(var_name, &mpi_comm);
  int dspaces_put(var_name, ver, size,
                  ndim, lb, ub, data);
  dspaces_unlock_on_write(var_name, &mpi_comm);
  
  return dspaces_put_sync();
}