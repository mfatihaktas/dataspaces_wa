#include "dspaces_drive.h"

void debug_print(const char* var_name, unsigned int ver, int datasize, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "\n"
            << "var_name= " << var_name << "\n"
            << "ver= " << ver << "\n"
            << "datasize= " << datasize << "\n"
            << "ndim= " << ndim << "\n";
  std::cout << "gdim=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << gdim[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "lb=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << lb[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "ub=";
  for (int i=0; i<ndim; i++){
    std::cout << "\t" << ub[i] << ", ";
  }
  std::cout << "\n";
  
  std::cout << "data=";
  for (int i=0; i<datasize; i++){
    std::cout << "\t" << data[i] << ", ";
  }
  std::cout << "\n";
}


//************************************  DspacesDriver  *******************************//
DSpacesDriver::DSpacesDriver(int num_peers, int appid)
: finalized(false),
  num_peers(num_peers),
  appid(appid)
{
  /*
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  */
  //
  LOG(INFO) << "DSpacesDriver:: constructed.";
}

DSpacesDriver::~DSpacesDriver()
{
  if (!finalized){
    finalize();
  }
  //
  LOG(INFO) << "DSpacesDriver:: destructed.";
}

int DSpacesDriver::finalize()
{
  if (finalized){
    LOG(INFO) << "finalize:: already finalized !";
    return 2;
  }
  try{
    /*
    dspaces_finalize();
    MPI_Barrier(mpi_comm);
    MPI_Finalize();
    */
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
                            int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data)
{
  MPI_Comm mpi_comm;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  //
  //init(num_peers, appid);
  dspaces_init(num_peers, appid, &mpi_comm, NULL);
  
  dspaces_define_gdim("dummy", ndim, gdim);
  
  LOG(INFO) << "sync_put:: before put;";
  debug_print(var_name, ver, size, ndim, gdim, lb, ub, (int*) data);
  
  dspaces_lock_on_write("metadata_lock", &mpi_comm);
  dspaces_put("dummy", ver, size*sizeof(int),
              ndim, lb, ub, data);
  dspaces_unlock_on_write("metadata_lock", &mpi_comm);
  
  LOG(INFO) << "sync_put:: after put;";
  debug_print(var_name, ver, size, ndim, gdim, lb, ub, (int*) data);
  
  int result = dspaces_put_sync();
  //
  dspaces_finalize();
  MPI_Barrier(mpi_comm);
  MPI_Finalize();
  
  return result;
}

int DSpacesDriver::get(const char* var_name, unsigned int ver, int size,
                       int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data)
{
  MPI_Comm mpi_comm;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  //
  //init(num_peers, appid);
  dspaces_init(num_peers, appid, &mpi_comm, NULL);
  
  usleep(1000*1000);
  
  dspaces_define_gdim("dummy", ndim, gdim);
  
  LOG(INFO) << "get:: before get;";
  debug_print(var_name, ver, size, ndim, gdim, lb, ub, (int*) data);
  
  dspaces_lock_on_read("metadata_lock", &mpi_comm);
  int result= dspaces_get("dummy", ver, size*sizeof(int),
                          ndim, lb, ub, data);
  dspaces_unlock_on_read("metadata_lock", &mpi_comm);
  
  LOG(INFO) << "get:: after get;";
  debug_print(var_name, ver, size, ndim, gdim, lb, ub, (int*) data);
  //
  dspaces_finalize();
  MPI_Barrier(mpi_comm);
  MPI_Finalize();
  
  return result;
}

//************************************  TestClient  *******************************//

TestClient::TestClient(int num_peers, int app_id)
: ds_driver_ ( new DSpacesDriver(num_peers, app_id) )
{
  this->num_peers = num_peers;
  this->app_id = app_id;
  
  //
  LOG(INFO) << "TestClient:: constructed.";
}

TestClient::~TestClient()
{
  //
  LOG(INFO) << "TestClient:: destructed.";
}

#define TEST_SIZE 1
#define TEST_NDIM 1
#define TEST_DATASIZE TEST_SIZE*TEST_NDIM
#define TEST_VER 1
#define TEST_SGDIM 10

void TestClient::put_test()
{
  LOG(INFO) << "put_test:: started.";
  //
  //ds_driver_->init(num_peers, app_id);
  LOG(INFO) << "dspaces inited.";
  
  //generics
  const char* var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  for (int i=0; i<TEST_DATASIZE; i++){
    data[i] = (i+1)*111;
  }
  
  int result = ds_driver_->sync_put(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "put_test:: sync_put returned " << result;
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  ds_driver_->finalize();
  //
  LOG(INFO) << "put_test:: done.";
}

void TestClient::get_test()
{
  LOG(INFO) << "get_test:: started.";
  //
  //ds_driver_->init(num_peers, app_id);
  LOG(INFO) << "get_test:: dspaces inited.";
  
  //generics
  const char* var_name = "dummy";
  uint64_t* gdim = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    gdim[i] = TEST_SGDIM;
  }
  //specifics
  int *data = (int*)malloc(TEST_DATASIZE*sizeof(int));
  uint64_t *lb = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(TEST_NDIM*sizeof(uint64_t));
  for (int i=0; i<TEST_NDIM; i++){
    lb[i] = 0;
    ub[i] = TEST_SIZE-1;
  }
  
  //
  usleep(1000*1000);
  
  int result = ds_driver_->get(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "get_test:: get returned " << result;
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  ds_driver_->finalize();
  //
  LOG(INFO) << "get_test:: done.";
}

void TestClient::dummy_put()
{
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
  dspaces_init(num_peers,app_id, &gcomm, NULL);
  
  int ndim = 1;  
  uint64_t* gdim = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  for (int i=0; i<ndim; i++){
    gdim[i] = 10;
  }
  
  uint64_t *lb = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  for (int i=0; i<ndim; i++){
    lb[i] = 0;
    ub[i] = 0;
  }
  
  dspaces_define_gdim("dummy", ndim, gdim);
  
  int *num_ = (int*)malloc(sizeof(int));
  num_[0] = 444;
  
  dspaces_lock_on_write("metadata_lock", &gcomm);
  dspaces_put ("dummy", 1, sizeof(int), ndim, lb, ub, num_);
  dspaces_unlock_on_write("metadata_lock", &gcomm);
  
  free(gdim);
  dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();
}

void TestClient::dummy_get()
{
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
  dspaces_init(num_peers,app_id, &gcomm, NULL);
  
  int ndim = 1;  
  uint64_t* gdim = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  for (int i=0; i<ndim; i++){
    gdim[i] = 10;
  }
  
  uint64_t *lb = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  uint64_t *ub = (uint64_t*)malloc(ndim*sizeof(uint64_t));
  for (int i=0; i<ndim; i++){
    lb[i] = 0;
    ub[i] = 0;
  }
  
  usleep(1000*1000);
  
  dspaces_define_gdim("dummy", ndim, gdim);
  
  int* num_ = (int*)malloc(sizeof(int));
  
  dspaces_lock_on_read("metadata_lock", &gcomm);
  dspaces_get("dummy", 1, sizeof(int), 1, lb, ub, num_);
  dspaces_unlock_on_read("metadata_lock", &gcomm);
  
  printf("You get: %d\n", num_[0]);
  
  free(gdim);
  dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();
}

/*
void TestClient::init_all()
{
  LOG(INFO) << "init_all:: started.";
  //
  std::vector<boost::shared_ptr<boost::thread> > t_v;
  
  for (int appid=0; appid<num_peers; appid++){
    boost::shared_ptr<DSpacesDriver> dsdriver_ = appid_dsdriver_map[appid];
    boost::shared_ptr< boost::thread > t_(
      new boost::thread(&DSpacesDriver::init, *dsdriver_, num_peers, appid)
    );
    t_v.push_back(t_);
  }
  //wait for threads to complete
  for (int i = 0; i < t_v.size(); i++){
    t_v[i]->join();
  }
  
  //
  LOG(INFO) << "init_all:: done.";
}

void TestClient::finalize_all()
{
  for (int appid=0; appid<num_peers; appid++){
    appid_dsdriver_map[appid]->finalize();
  }
  //
  LOG(INFO) << "finalize_all:: done.";
}
*/