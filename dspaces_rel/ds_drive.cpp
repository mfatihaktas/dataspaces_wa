#include "ds_drive.h"

std::string get_data_id(const char* var_name, unsigned int ver, int ndim, uint64_t *lb_, uint64_t *ub_)
{
  return boost::lexical_cast<std::string>(var_name) + "_" + boost::lexical_cast<std::string>(ver);
}

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

DSDriver::DSDriver(int app_id, int num_peer)
: app_id(app_id), num_peer(num_peer),
  closed(false), get_flag(false), get__flag(false), sync_put_flag(false)
{
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
  init(num_peer, app_id);
  // 
  log_(INFO, "constructed.")
}

DSDriver::DSDriver(int app_id, int num_peer, MPI_Comm mpi_comm)
: app_id(app_id), num_peer(num_peer), mpi_comm(mpi_comm),
  closed(false), get_flag(false), get__flag(false), sync_put_flag(false)
{
  init(num_peer, app_id);
  
  this->get__flag = false;
  
  refresh_last_lock_time();
  
  // TEST_NZ(pthread_mutex_init(&dspaces_put_get_mtx, NULL) )
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
  
  get_poll_t_->interrupt();
  // TEST_NZ(pthread_mutex_destroy(&dspaces_put_get_mtx) )
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

int DSDriver::init(int num_peer, int app_id)
{
  int err;
  return_if_err(dspaces_init(num_peer, app_id, &mpi_comm, NULL), err)
  
  // boost::thread t(&DSDriver::get_poll, this);
  get_poll_t_ = boost::make_shared<boost::thread>(&DSDriver::get_poll, this);
  return 0;
}

int DSDriver::sync_put(const char* var_name, unsigned int ver, int size,
                      int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   ++putget_lock_unlock_count;
  //   log_(INFO, "before lock putget_lock_unlock_count= " << putget_lock_unlock_count)
  // }
  // TEST_NZ(pthread_mutex_lock(&dspaces_put_get_mtx) )
  // boost::unique_lock<boost::timed_mutex> guard(dspaces_put_get_mtx, 1);
  // boost::timed_mutex::scoped_lock scoped_lock(dspaces_put_get_mtx, boost::get_system_time() + boost::posix_time::seconds(10) );
  // boost::lock_guard<boost::mutex> guard(dspaces_put_get_mtx);
  boost::lock_guard<boost::mutex> guard(dspaces_sync_put_mtx);
  // dspaces_define_gdim(var_name, ndim, gdim_);
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   this->sync_put_flag = true;
  // }
  
  // log_(INFO, "will wait for get__flag= " << this->get__flag)
  
  // while (this->get__flag) {
  //   // {
  //   //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   //   flag = this->get__flag;
  //   // }
  // }
  // usleep(10*1000);
  // log_(INFO, "done waiting for get__flag.")
  
  // do_timing("sync_put");
  {
    // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
    // lock_on_write(var_name);
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
    // unlock_on_write(var_name);
  }
  // refresh_last_lock_time();
  
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   this->sync_put_flag = false;
  // }
  // TEST_NZ(pthread_mutex_unlock(&dspaces_put_get_mtx) )
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   --putget_lock_unlock_count;
  //   log_(INFO, "after unlock putget_lock_unlock_count= " << putget_lock_unlock_count)
  // }
  
  return r;
}
// int DSDriver::sync_put(const char* var_name, unsigned int ver, int size,
//                       int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_, void* data_)
// {
//   lock_on_write(var_name);
//   int r = dspaces_put(var_name, ver, size, ndim, lb_, ub_, data_);
//   // dspaces_put_sync();
//   unlock_on_write(var_name);
  
//   return r;
// }

// int DSDriver::get_(const char* var_name, unsigned int ver, int size,
//                   int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
// {
//   boost::lock_guard<boost::mutex> guard(dspaces_get__mtx);
//   // dspaces_define_gdim(var_name, ndim, gdim_);
//   {
//     boost::lock_guard<boost::mutex> guard(property_mtx);
//     this->get__flag = true;
//   }
  
//   // log_(INFO, "will wait for get_flag || sync_put_flag...")
//   bool flag = true;
//   while (this->get_flag || this->sync_put_flag) {
//     {
//       // boost::lock_guard<boost::mutex> guard(property_mtx);
//       // flag = this->get_flag || this->sync_put_flag;
//     }
//   }
//   // log_(INFO, "done waiting for get_flag || sync_put_flag.")
  
//   // do_timing("get_");
//   {
//     // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
//     // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
//     lock_on_read(var_name);
//   }
  
//   int r;
//   {
//     // log_(INFO, "locking for var_name= " << var_name)
//     // boost::lock_guard<boost::mutex> guard(dspaces_write_mtx);
//     // boost::lock_guard<boost::mutex> guard2(dspaces_read_mtx);
//     // log_(INFO, "will get var_name= " << var_name)
//     r = dspaces_get(var_name, ver, size,
//                     ndim, lb_, ub_, data_);
//     // log_(INFO, "got var_name= " << var_name)
//     // log_(INFO, "dspaces_get done for var_name= " << var_name)
//   }
//   // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
//   // do_timing("get_");
//   {
//     // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
//     unlock_on_read(var_name);
//   }
  
//   {
//     boost::lock_guard<boost::mutex> guard(property_mtx);
//     this->get__flag = false;
//   }
  
//   return r;
// }

int DSDriver::reg_get_wait_for_completion(const char* var_name, unsigned int ver, int size,
                                          int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void* data_)
{
  get_info_bq.push(
    boost::make_shared<get_info>(var_name, ver, size, ndim, gdim_, lb_, ub_, &data_) );
  
  log_(INFO, "waiting for get <var_name= " << var_name << ", ver= " << ver << ">...")
  unsigned int sync_point = patch::hash_str(get_data_id(var_name, ver, ndim, lb_, ub_) );
  syncer.add_sync_point(sync_point, 1);
  syncer.wait(sync_point);
  syncer.del_sync_point(sync_point);
  log_(INFO, "done waiting for get <var_name= " << var_name << ", ver= " << ver << ">.")
  
  return hashed_data_id__get_return_map[sync_point];
}

int DSDriver::get_poll()
{
  log_(INFO, "started...")
  while (1) {
    boost::shared_ptr<get_info> get_info_ = get_info_bq.pop();
    
    // lock_on_read(get_info_->var_name);
    get_done = false;
    get_success = false;
    boost::thread t(
      &DSDriver::plain_get, this, get_info_->var_name, get_info_->ver, get_info_->size,
                                  get_info_->ndim, get_info_->gdim_, get_info_->lb_, get_info_->ub_, *(get_info_->data__) );
    int counter = 0;
    do {
      if (counter)
        sleep(1);
      ++counter;
    } while (!get_done && counter < 10);
    
    int r = 1;
    if (get_done) {
      if (get_success)
        r = 0;
    }
    else {
      log_(INFO, "get_done= " << get_done << " but plain_get returned failure since counter= " << counter)
      t.interrupt();
    }
    // unlock_on_read(get_info_->var_name);
    
    unsigned int sync_point = patch::hash_str(
      get_data_id(get_info_->var_name, get_info_->ver, get_info_->ndim, get_info_->lb_, get_info_->ub_) );
    hashed_data_id__get_return_map[sync_point] = r;
    syncer.notify(sync_point);
  }
}

// boost::mutex dspaces_get_mtx;
int DSDriver::timed_get(const char* var_name, unsigned int ver, int size,
                        int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  int err;
  // boost::recursive_mutex::scoped_lock scoped_lock(dspaces_get_mtx);
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   ++putget_lock_unlock_count;
  //   log_(INFO, "before lock putget_lock_unlock_count= " << putget_lock_unlock_count)
  // }
  // TEST_NZ(pthread_mutex_lock(&dspaces_put_get_mtx) )
  // boost::unique_lock<boost::timed_mutex> guard(dspaces_put_get_mtx, 1);
  // boost::timed_mutex::scoped_lock scoped_lock(dspaces_put_get_mtx, boost::get_system_time() + boost::posix_time::milliseconds(1000) );
  // boost::lock_guard<boost::mutex> guard(dspaces_put_get_mtx);
  boost::lock_guard<boost::mutex> guard(dspaces_get_mtx);
  // boost::mutex::scoped_lock scoped_lock(dspaces_get_mtx);
  
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   this->get_flag = true;
  // }
  
  // // log_(INFO, "will wait for get__flag...")
  // struct timeval wait_start_time_val, current_time_val, diff_time_val;
  // return_if_err(gettimeofday(&wait_start_time_val, NULL), err)
  // bool flag = true;
  // while (flag) {
  //   {
  //     return_if_err(gettimeofday(&current_time_val, NULL), err)
  //     timeval_subtract(&diff_time_val, &current_time_val, &wait_start_time_val);
  //     if (diff_time_val.tv_sec > 1) {
  //       log_(ERROR, "too much waiting for get__flag, breaking.")
  //       // TODO
  //       // this->get__flag = false;
  //       break;
  //     }
      
  //     boost::lock_guard<boost::mutex> guard(property_mtx);
  //     flag = this->get__flag;
  //   }
  // }
  // log_(INFO, "done waiting for get__flag.")
  
  // dspaces_define_gdim(var_name, ndim, gdim_);
  // do_timing("get");
  // {
  //   // boost::lock_guard<boost::mutex> guard(lock_mtx);
  //   lock_on_read(var_name);
  // }
  
  // int r;
  // {
  //   // log_(INFO, "will get var_name= " << var_name)
  //   // boost::lock_guard<boost::mutex> guard(dspaces_read_mtx);
  //   r = dspaces_get(var_name, ver, size,
  //                   ndim, lb_, ub_, data_);
  //   // log_(INFO, "got var_name= " << var_name)
  //   // log_(INFO, "dspaces_get done for var_name= " << var_name)
    
  // }
  
  // do_timing("get");
  // {
  //   // boost::lock_guard<boost::mutex> guard(unlock_mtx);
  //   unlock_on_read(var_name);
  // }
  
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   this->get_flag = false;
  // }
  
  // TEST_NZ(pthread_mutex_unlock(&dspaces_put_get_mtx) )
  // {
  //   boost::lock_guard<boost::mutex> guard(property_mtx);
  //   --putget_lock_unlock_count;
  //   log_(INFO, "after unlock putget_lock_unlock_count= " << putget_lock_unlock_count)
  // }
  
  // Note: Works but mutual exclusive get using only mutex does not allow fair access between threads
  // lock_on_read(var_name);
  get_done = false;
  get_success = false;
  boost::thread t(&DSDriver::plain_get, this, var_name, ver, size, ndim, gdim_, lb_, ub_, data_);
  int counter = 0;
  do {
    if (counter)
      sleep(1);
    ++counter;
  } while (!get_done && counter < 10);
  
  int r = 1; // TODO: for now always return success
  if (get_done) {
    if (get_success)
      r = 0;
  }
  else {
    log_(INFO, "get_done= " << get_done << " but plain_get returned failure since counter= " << counter)
    t.interrupt();
  }
  // unlock_on_read(var_name);
  
  boost::this_thread::yield();
  
  return r;
}
int DSDriver::plain_get(const char* var_name, unsigned int ver, int size,
                        int ndim, uint64_t* gdim_, uint64_t* lb_, uint64_t* ub_, void *data_)
{
  int r = dspaces_get(var_name, ver, size, ndim, lb_, ub_, data_);
  if (r) {
    log_(ERROR, "dspaces_get failed for <var_name= " << var_name << ", ver= " << ver << ">")
    get_success = false; // TODO: for now to not mess with dspaces' failures and carry on with the experiments
  }
  else 
    get_success = true;
  get_done = true;
  
  return 0;
}

int DSDriver::get(const char* var_name, unsigned int ver, int size,
                  int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
{
  // lock_on_read(var_name);
  int r = dspaces_get(var_name, ver, size, ndim, lb_, ub_, data_);
  if (r) {
    log_(ERROR, "dspaces_get failed for <var_name= " << var_name << ", ver= " << ver << ">")
  }
  // unlock_on_read(var_name);
  
  return r;
}

// int DSDriver::sync_put_without_lock(const char* var_name, unsigned int ver, int size,
//                                     int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
// {
//   // dspaces_define_gdim(var_name, ndim, gdim_);
//   int r = dspaces_put(var_name, ver, size,
//                       ndim, lb_, ub_, data_);
//   r = dspaces_put_sync();
//   unlock_on_write(var_name);
//   return r;
// }

int DSDriver::del(const char* var_name, unsigned int ver)
{
  log_(ERROR, "dspaces_remove is buggy! this should not have been called.")
  return 1;
  
  // lock_on_write(var_name);
  // int r = dspaces_remove(var_name, ver);
  // unlock_on_write(var_name);
  
  // return r;
}

void DSDriver::lock_on_write(const char* var_name)
{
  log_(INFO, "started; var_name= " << var_name)
  dspaces_lock_on_write(var_name, &mpi_comm);
  log_(INFO, "done; var_name= " << var_name)
  refresh_last_lock_time();
}


void DSDriver::unlock_on_write(const char* var_name)
{
  log_(INFO, "started; var_name= " << var_name)
  dspaces_unlock_on_write(var_name, &mpi_comm);
  log_(INFO, "done; var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::lock_on_read(const char* var_name)
{
  log_(INFO, "started; var_name= " << var_name)
  dspaces_lock_on_read(var_name, &mpi_comm);
  log_(INFO, "done; var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::unlock_on_read(const char* var_name)
{
  log_(INFO, "started; var_name= " << var_name)
  dspaces_unlock_on_read(var_name, &mpi_comm);
  log_(INFO, "done; var_name= " << var_name)
  refresh_last_lock_time();
}

void DSDriver::reg_cb_on_get(std::string var_name, function_cb_on_get cb)
{
  varname_cbonget_map[var_name] = cb;
}

void DSDriver::init_get_thread(std::string var_name, int size)
{
//   boost::shared_ptr<boost::thread> t_(
//     new boost::thread(&DSDriver::t_get, this, var_name, size)
//   );
//   riget_thread_v.push_back(t_);
//   //
//   log_(INFO, "inited for var_name= " << var_name)
}

// void DSDriver::t_get(std::string var_name, int size)
// {
//   //1 dimensional char array is expected
//   // uint64_t gdim = 0; //size;
//   // uint64_t lb = 0;
//   // uint64_t ub = 0; //size-1;
//   uint64_t* gdim_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
//   uint64_t* lb_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
//   uint64_t* ub_ = (uint64_t*)malloc(3*sizeof(uint64_t) );
//   for (int i = 0; i < 3; i++) {
//     gdim_[i] = 0;
//     lb_[i] = 0;
//     ub_[i] = 0;
//   }
  
//   char *data_ = (char*)malloc(size*sizeof(char) );
  
//   // get_(const char* var_name, unsigned int ver, int size, int ndim, uint64_t *gdim_, uint64_t *lb_, uint64_t *ub_, void *data_)
  
//   // while ( get_(var_name.c_str(), 0, size*sizeof(char), 3, gdim_, lb_, ub_, data_) ) {
//   if ( get_(var_name.c_str(), 0, size*sizeof(char), 3, gdim_, lb_, ub_, data_) ) {
//     log_(ERROR, "ri_get failed!")
//     // usleep(INTER_RI_GET_TIME);
//   }
//   else {
//     // log_(INFO, "ri_data=\n" << data)
//     varname_cbonget_map[var_name](data_);
//   }
  
//   free(data_);
//   free(gdim_);
//   free(lb_);
//   free(ub_);
// }

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
  int err;
  return_if_err(gettimeofday(&time_val, NULL), err)
  
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
  int err;
  return_if_err(gettimeofday(&last_lock_time, NULL), err)
  
  return 0;
}