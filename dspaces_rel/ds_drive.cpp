#include "ds_drive.h"

//************************************  DspacesDriver  *******************************//
DSpacesDriver::DSpacesDriver(int num_peers, int appid)
: finalized(false),
  num_peers(num_peers),
  appid(appid)
{
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
  init(num_peers, appid);
  //
  LOG(INFO) << "DSpacesDriver:: constructed.";
}

DSpacesDriver::~DSpacesDriver()
{
  for (int i = 0; i < riget_thread_v.size(); i++){
    riget_thread_v[i]->join();
  }
  //LOG(INFO) << "~DSpacesDriver:: all riget_threads joined.";
  
  if (!finalized){
    finalize();
  }
  //
  LOG(INFO) << "~DSpacesDriver:: destructed.";
}

int DSpacesDriver::finalize()
{
  if (finalized){
    LOG(INFO) << "finalize:: already finalized!";
    return 2;
  }
  try{
    dspaces_finalize();
    MPI_Barrier(mpi_comm);
    MPI_Finalize();
  }
  
  catch(std::exception & ex)
  {
    LOG(ERROR) << "finalize:: Exception=" << ex.what();
    return 1;
  }
  //
  finalized = true;
  LOG(INFO) << "finalize:: finalized.";
  return 0;
}

int DSpacesDriver::init(int num_peers, int appid)
{
  return dspaces_init(num_peers, appid, &mpi_comm, NULL);
}

int DSpacesDriver::sync_put(const char* var_name, unsigned int ver, int size,
                            int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  dspaces_define_gdim(var_name, ndim, gdim_);
  
  dspaces_lock_on_write(var_name, &mpi_comm);
  int result = dspaces_put(var_name, ver, size,
                           ndim, lb_, ub_, data_);
  dspaces_put_sync();
  dspaces_unlock_on_write(var_name, &mpi_comm);
  
  return result;
}

int DSpacesDriver::get(const char* var_name, unsigned int ver, int size,
                       int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  dspaces_define_gdim(var_name, ndim, gdim_);
  
  dspaces_lock_on_read(var_name, &mpi_comm);
  
  usleep(1000*1000);
  int result= dspaces_get(var_name, ver, size,
                          ndim, lb_, ub_, data_);
  dspaces_unlock_on_read(var_name, &mpi_comm);
  
  return result;
}

int DSpacesDriver::sync_put_without_lock(const char* var_name, unsigned int ver, int size,
                                         int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  dspaces_define_gdim(var_name, ndim, gdim_);
  
  int result = dspaces_put(var_name, ver, size,
                           ndim, lb_, ub_, data_);
  dspaces_put_sync();
  dspaces_unlock_on_write(var_name, &mpi_comm);
  
  return result;
}

void DSpacesDriver::lock_on_write(const char* var_name)
{
  LOG(INFO) << "lock_before_write:: locking var_name= " << var_name;
  dspaces_lock_on_write(var_name, &mpi_comm);
}

void DSpacesDriver::unlock_on_write(const char* var_name)
{
  LOG(INFO) << "unlock_before_write:: unlocking var_name= " << var_name;
  dspaces_unlock_on_write(var_name, &mpi_comm);
}

void DSpacesDriver::lock_on_read(const char* var_name)
{
  LOG(INFO) << "lock_before_read:: locking var_name= " << var_name;
  dspaces_lock_on_read(var_name, &mpi_comm);
}

void DSpacesDriver::unlock_on_read(const char* var_name)
{
  LOG(INFO) << "unlock_before_read:: unlocking var_name= " << var_name;
  dspaces_unlock_on_read(var_name, &mpi_comm);
}

void DSpacesDriver::reg_cb_on_get(std::string var_name, function_cb_on_get cb)
{
  varname_cbonget_map[var_name] = cb;
}

void DSpacesDriver::ri_get(std::string var_name, int size)
{
  //1 dimensional char array is expected
  uint64_t gdim = size;
  char *data = (char*)malloc(size*sizeof(char));
  uint64_t lb = 0;
  uint64_t ub = size-1;
  
  if( get(var_name.c_str(), 1, sizeof(char), 1, &gdim, &lb, &ub, data) ){
    LOG(ERROR) << "ri_get:: get failed!";
    return;
  }
  LOG(INFO) << "ri_get:: data=\n" << data;
  varname_cbonget_map[var_name](data);
  
  free(data);
}

void DSpacesDriver::init_riget_thread(std::string var_name, int size)
{
  boost::shared_ptr<boost::thread> t_(
    new boost::thread(&DSpacesDriver::ri_get, this, var_name, size)
  );
  riget_thread_v.push_back(t_);
  //
  LOG(INFO) << "init_riget_thread:: inited for var_name= " << var_name;
}
