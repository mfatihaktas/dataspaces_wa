#include "ds_drive.h"

/*****************************************  DspacesDriver  ****************************************/
#define INTER_LOCK_TIME 1000*1000 //usec
#define INTER_RI_GET_TIME 2*1000*1000 //usec

uint64_t DSDriver::get_data_length(int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_)
{
  uint64_t dim_length[ndim];
  
  for (int i = 0; i < ndim; i++) {
    uint64_t lb = lb_[i];
    if (lb < 0 || lb > gdim_[i] ) {
      log_(ERROR, "lb= " << lb << " is not feasible!")
      return 0;
    }
    uint64_t ub = ub_[i];
    if (ub < 0 || ub > gdim_[i] || ub < lb) {
      log_(ERROR, "ub= " << ub << " is not feasible!")
      return 0;
    }
    dim_length[i] = ub - lb + 1;
  }
  
  uint64_t volume = 1;
  for (int i = 0; i < ndim; i++)
    volume *= dim_length[i];
  
  return volume;
}

DSDriver::DSDriver(int app_id, int num_peers)
: app_id(app_id), num_peers(num_peers),
  closed(false), get_flag(false), get__flag(false), sync_put_flag(false)
{
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
  init(num_peers, app_id);
  // 
  log_(INFO, "constructed.")
}

DSDriver::DSDriver(int app_id, int num_peers, MPI_Comm mpi_comm)
: app_id(app_id), num_peers(num_peers), mpi_comm(mpi_comm),
  closed(false), get_flag(false), get__flag(false), sync_put_flag(false)
{
  init(num_peers, app_id);
  
  this->get__flag = false;
  
  refresh_last_lock_time();
  // 
  log_(INFO, "constructed.")
}

DSDriver::~DSDriver()
{
  // riget_threads cause hanging.
  // for (int i = 0; i < riget_thread_v.size(); i++)
  //   riget_thread_v[i]->join();
  // log_(INFO, "~all riget_threads joined.")
  
  if (!closed)
    close();
  // 
  log_(INFO, "destructed.")
}

int DSDriver::close()
{
  if (closed) {
    log_(WARNING, "already closed!")
    return 2;
  }
  try {
    dspaces_finalize();
    MPI_Barrier(mpi_comm);
    MPI_Finalize();
  }
  
  catch(std::exception& ex) {
    log_(ERROR, "Exception=" << ex.what() )
    return 1;
  }
  // 
  closed = true;
  log_(INFO, "closed.")
  return 0;
}

int DSDriver::init(int num_peers, int app_id)
{
  return dspaces_init(num_peers, app_id, &mpi_comm, NULL);
}

int DSDriver::sync_put(const char* var_name, unsigned int ver, int size,
                       int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  boost::lock_guard<boost::mutex> guard(dspaces_sync_put_mtx);
  // dspaces_define_gdim(var_name, ndim, gdim_);
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->sync_put_flag = true;
  }
  
  // log_(INFO, "will wait for get__flag= " << this->get__flag)
  
  while (this->get__flag) {
    // {
    //   boost::lock_guard<boost::mutex> guard(property_mtx);
    //   flag = this->get__flag;
    // }
  }
  // usleep(10*1000);
  // log_(INFO, "done waiting for get__flag.")
  
  // do_timing("sync_put");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    lock_on_write(var_name);
  }
  // refresh_last_lock_time();
  
  int r;
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    r = dspaces_put(var_name, ver, size,
                    ndim, lb_, ub_, data_);
  }
  dspaces_put_sync();
  
  // do_timing("sync_put");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    unlock_on_write(var_name);
  }
  // refresh_last_lock_time();
  
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->sync_put_flag = false;
  }
  
  return r;
}

int DSDriver::get_(const char* var_name, unsigned int ver, int size,
                   int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  boost::lock_guard<boost::mutex> guard(dspaces_get__mtx);
  // dspaces_define_gdim(var_name, ndim, gdim_);
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->get__flag = true;
  }
  
  // log_(INFO, "will wait for get_flag || sync_put_flag...")
  bool flag = true;
  while (this->get_flag || this->sync_put_flag) {
    {
      // boost::lock_guard<boost::mutex> guard(property_mtx);
      // flag = this->get_flag || this->sync_put_flag;
    }
  }
  // log_(INFO, "done waiting for get_flag || sync_put_flag.")
  
  // do_timing("get_");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    lock_on_read(var_name);
  }
  
  int r;
  {
    // log_(INFO, "locking for var_name= " << var_name)
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    // boost::lock_guard<boost::mutex> guard2(dspaces_read_mtx);
    // log_(INFO, "will get var_name= " << var_name)
    r = dspaces_get(var_name, ver, size,
                    ndim, lb_, ub_, data_);
    // log_(INFO, "got var_name= " << var_name)
    // log_(INFO, "dspaces_get done for var_name= " << var_name)
  }
  // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
  // do_timing("get_");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
    unlock_on_read(var_name);
  }
  
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->get__flag = false;
  }
  
  return r;
}

int DSDriver::get(const char* var_name, unsigned int ver, int size,
                  int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  boost::lock_guard<boost::mutex> guard(dspaces_get_mtx);
  
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->get_flag = true;
  }
  
  // log_(INFO, "will wait for get__flag...")
  struct timeval wait_start_time_val, current_time_val, diff_time_val;
  if (gettimeofday(&wait_start_time_val, NULL) ) {
    log_(ERROR, "gettimeofday returned non-zero.")
    return 1;
  }
  
  bool flag = true;
  while (flag) {
    {
      if (gettimeofday(&current_time_val, NULL) ) {
        log_(ERROR, "gettimeofday returned non-zero.")
        return 1;
      }
      timeval_subtract(&diff_time_val, &current_time_val, &wait_start_time_val);
      if (diff_time_val.tv_sec > 1) {
        log_(ERROR, "too much waiting for get__flag, breaking.")
        // TODO
        // this->get__flag = false;
        break;
      }
      
      boost::lock_guard<boost::mutex> guard(property_mtx);
      flag = this->get__flag;
    }
  }
  // log_(INFO, "done waiting for get__flag.")
  
  // dspaces_define_gdim(var_name, ndim, gdim_);
  // do_timing("get");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
    // do_timing("get");
    lock_on_read(var_name);
  }
  
  int r;
  {
    
    // log_(INFO, "will get var_name= " << var_name)
    // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
    r = dspaces_get(var_name, ver, size,
                    ndim, lb_, ub_, data_);
    // log_(INFO, "got var_name= " << var_name)
    // log_(INFO, "dspaces_get done for var_name= " << var_name)
    
  }
  
  // do_timing("get");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
    unlock_on_read(var_name);
  }
  
  {
    boost::lock_guard<boost::mutex> guard(property_mtx);
    this->get_flag = false;
  }
  
  return r;
}

int DSDriver::sync_put_without_lock(const char* var_name, unsigned int ver, int size,
                                    int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  // dspaces_define_gdim(var_name, ndim, gdim_);
  
  int r = dspaces_put(var_name, ver, size,
                      ndim, lb_, ub_, data_);
  r = dspaces_put_sync();
  
  unlock_on_write(var_name);
  
  return r;
}

int DSDriver::del(const char* var_name, unsigned int ver)
{
  lock_on_write(var_name);
  int r = dspaces_remove(var_name, ver);
  unlock_on_write(var_name);
  
  return r;
}

void DSDriver::lock_on_write(const char* var_name)
{
  // log_(INFO, "locking var_name= " << var_name)
  dspaces_lock_on_write(var_name, &mpi_comm);
  // log_(INFO, "locked var_name= " << var_name)
  refresh_last_lock_time();
}


void DSDriver::unlock_on_write(const char* var_name)
{
  // log_(INFO, "unlocking var_name= " << var_name)
  dspaces_unlock_on_write(var_name, &mpi_comm);
  // log_(INFO, "unlocked var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::lock_on_read(const char* var_name)
{
  // log_(INFO, "locking var_name= " << var_name)
  dspaces_lock_on_read(var_name, &mpi_comm);
  // log_(INFO, "locked var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::unlock_on_read(const char* var_name)
{
  // log_(INFO, "unlocking var_name= " << var_name)
  dspaces_unlock_on_read(var_name, &mpi_comm);
  // log_(INFO, "unlocked var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::reg_cb_on_get(std::string var_name, function_cb_on_get cb)
{
  varname_cbonget_map[var_name] = cb;
}

void DSDriver::init_get_thread(std::string var_name, int size)
{
  boost::shared_ptr<boost::thread> t_(
    new boost::thread(&DSDriver::t_get, this, var_name, size)
  );
  riget_thread_v.push_back(t_);
  //
  log_(INFO, "inited for var_name= " << var_name)
}

void DSDriver::t_get(std::string var_name, int size)
{
  //1 dimensional char array is expected
  // uint64_t gdim = 0; //size;
  // uint64_t lb = 0;
  // uint64_t ub = 0; //size-1;
  uint64_t* gdim_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* lb_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  uint64_t* ub_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
  for (int i = 0; i < 3; i++) {
    gdim_[i] = 0;
    lb_[i] = 0;
    ub_[i] = 0;
  }
  
  char *data_ = (char*)malloc(size*sizeof(char) );
  
  // get_(const char* var_name, unsigned int ver, int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
  
  // while ( get_(var_name.c_str(), 0, size*sizeof(char), 3, gdim_, lb_, ub_, data_) ) {
  if ( get_(var_name.c_str(), 0, size*sizeof(char), 3, gdim_, lb_, ub_, data_) ) {
    log_(ERROR, "ri_get failed!")
    // usleep(INTER_RI_GET_TIME);
  }
  else {
    // log_(INFO, "ri_data=\n" << data)
    varname_cbonget_map[var_name](data_);
  }
  
  free(data_);
  free(gdim_);
  free(lb_);
  free(ub_);
}

// Exclusive access on locks
void DSDriver::do_timing(const char* called_from)
{
  // boost::lock_guard<boost::mutex> guard(property_mtx);
  timeval inter_lock_time_val;
  get_inter_lock_time(&inter_lock_time_val);
  
  // log_(INFO, called_from << "." << "inter_lock_time_val=" << inter_lock_time_val.tv_sec << "..." << inter_lock_time_val.tv_usec)
  
  if (inter_lock_time_val.tv_sec > 0) {
    return;
  }
  
  time_t diff = INTER_LOCK_TIME - inter_lock_time_val.tv_usec;
  // log_(INFO, called_from << "." << "diff= " << diff << " usec.")
  if (diff > 0) {
    // log_(INFO, called_from << "." << "sleeping for " << diff << " usec.")
    usleep(diff);
    // log_(INFO, called_from << "." << "DONE sleeping for " << diff << " usec.")
  }
}

time_t DSDriver::get_inter_lock_time(struct timeval *r)
{
  struct timeval time_val;
  if (gettimeofday(&time_val, NULL) ) {
    log_(ERROR, "gettimeofday returned non-zero.")
    return 1;
  }
  timeval_subtract(r, &time_val, &last_lock_time);
  
  return 0;
}

void DSDriver::timeval_subtract(struct timeval *r, struct timeval *x, struct timeval *y)
{
  // Perform the carry for the later subtraction by updating y.
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  //Compute the time remaining to wait. tv_usec is certainly positive.
  r->tv_sec = x->tv_sec - y->tv_sec;
  r->tv_usec = x->tv_usec - y->tv_usec;
}

int DSDriver::refresh_last_lock_time()
{
  boost::lock_guard<boost::mutex> guard(property_mtx);
  if (gettimeofday(&last_lock_time, NULL) ) {
    log_(ERROR, "gettimeofday returned non-zero.")
    return 1;
  }
  
  return 0;
}