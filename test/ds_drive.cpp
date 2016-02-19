#include "ds_drive.h"

uint64_t DSDriver::get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for (int i = 0; i < ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i] ) {
      log(ERROR, "lb= " << lb << " is not feasible!")
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb) {
      log(ERROR, "ub= " << ub << " is not feasible!")
      return 0;
    }
    dim_length[i] = ub - lb + 1;
  }
  
  uint64_t volume = 1;
  for (int i = 0; i < ndim; i++)
    volume *= dim_length[i];
  
  return volume;
}

DSDriver::DSDriver(int num_peer, int app_id)
: num_peer(num_peer), app_id(app_id),
  closed(false)
{
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
  int err;
  return_err_if_ret_cond_flag(init(num_peer, app_id), err, !=, 0, )
  // 
  log(INFO, "constructed.")
}

DSDriver::DSDriver(MPI_Comm mpi_comm, int num_peer, int app_id)
: mpi_comm(mpi_comm), num_peer(num_peer), app_id(app_id),
  closed(false)
{
  int err;
  return_err_if_ret_cond_flag(init(num_peer, app_id), err, !=, 0, )
  // 
  log(INFO, "constructed.")
}

DSDriver::~DSDriver()
{
  if (!closed)
    close();
  // 
  log(INFO, "destructed.")
}

int DSDriver::close()
{
  if (closed)
    return 2;
  try {
    dspaces_finalize();
    MPI_Barrier(mpi_comm);
    MPI_Finalize();
  }
  catch (std::exception& ex) {
    log(ERROR, "Exception=" << ex.what() )
    return 1;
  }
  // 
  closed = true;
  log(INFO, "closed.")
  return 0;
}

int DSDriver::init(int num_peer, int app_id)
{
  return dspaces_init(num_peer, app_id, &mpi_comm, NULL);
}

int DSDriver::sync_put(const char* var_name, unsigned int ver, int size,
                       int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_, void* data_)
{
  lock_on_write(var_name);
  int r = dspaces_put(var_name, ver, size, ndim, lb_, ub_, data_);
  // dspaces_put_sync();
  unlock_on_write(var_name);
  
  return r;
}

int DSDriver::get(const char* var_name, unsigned int ver, int size,
                      int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_, void *data_)
{
  lock_on_read(var_name);
  // dspaces_define_gdim(var_name, ndim, gdim_);
  int r = dspaces_get(var_name, ver, size, ndim, lb_, ub_, data_);
  unlock_on_read(var_name);
  
  return r;
}

int DSDriver::del(const char* var_name, unsigned int ver)
{
  lock_on_read(var_name);
  // dspaces_define_gdim(var_name, ndim, gdim_);
  int r = dspaces_remove(var_name, ver);
  unlock_on_read(var_name);
  
  return r;
}

void DSDriver::lock_on_write(const char* var_name)
{
  log(INFO, "started; var_name= " << var_name)
  dspaces_lock_on_write(var_name, &mpi_comm);
  log(INFO, "done; var_name= " << var_name)
}


void DSDriver::unlock_on_write(const char* var_name)
{
  log(INFO, "started; var_name= " << var_name)
  dspaces_unlock_on_write(var_name, &mpi_comm);
  log(INFO, "done; var_name= " << var_name)
}

void DSDriver::lock_on_read(const char* var_name)
{
  log(INFO, "started; var_name= " << var_name)
  dspaces_lock_on_read(var_name, &mpi_comm);
  log(INFO, "done; var_name= " << var_name)
}

void DSDriver::unlock_on_read(const char* var_name)
{
  log(INFO, "started; var_name= " << var_name)
  dspaces_unlock_on_read(var_name, &mpi_comm);
  log(INFO, "done; var_name= " << var_name)
}
