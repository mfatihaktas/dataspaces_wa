#include "dspaces_drive.h"

void debug_print(const char* var_name, unsigned int ver, int datasize, int ndim, 
                 uint64_t* gdim, uint64_t* lb, uint64_t* ub, int* data)
{
  LOG(INFO) << "debug_print::";
  std::cout << "var_name= " << var_name << "\n"
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
  
  //
  if (data == NULL){
    return;
  }
  
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
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
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
                            int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data)
{
  //init(num_peers, appid);
  
  dspaces_define_gdim(var_name, ndim, gdim);
  
  dspaces_lock_on_write(var_name, &mpi_comm);
  int result = dspaces_put(var_name, ver, sizeof(int),
                           ndim, lb, ub, data);
  dspaces_put_sync();
  dspaces_unlock_on_write(var_name, &mpi_comm);
  
  return result;
}

int DSpacesDriver::get(const char* var_name, unsigned int ver, int size,
                       int ndim, uint64_t *gdim, uint64_t *lb, uint64_t *ub, void *data)
{
  //init(num_peers, appid);
  usleep(1000*1000);
  
  dspaces_define_gdim(var_name, ndim, gdim);
  
  dspaces_lock_on_read(var_name, &mpi_comm);
  int result= dspaces_get(var_name, ver, sizeof(int),
                          ndim, lb, ub, data);
  dspaces_unlock_on_read(var_name, &mpi_comm);
  //
  
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

float TestClient::get_avg(int size, int* data)
{
  float sum = 0;
  for (int i=0; i<size; i++){
    sum += data[i];
  }
  
  return sum / (float) size;
}

#define TEST_SIZE 50
#define TEST_NDIM 3
#define TEST_DATASIZE pow(TEST_SIZE, TEST_NDIM)
#define TEST_VER 1
#define TEST_SGDIM 100

void TestClient::put_test()
{
  LOG(INFO) << "put_test:: started.";
  //
  ds_driver_->init(num_peers, app_id);
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
    data[i] = (i+1);
  }
  
  LOG(INFO) << "put_test:: before put;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg = " << get_avg(TEST_DATASIZE, data) << "\n";
  
  int result = ds_driver_->sync_put(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "put_test:: sync_put returned " << result;
  
  LOG(INFO) << "put_test:: after put;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg= " << get_avg(TEST_DATASIZE, data) << "\n";
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  //ds_driver_->finalize();
  //
  LOG(INFO) << "put_test:: done.";
}

void TestClient::get_test()
{
  LOG(INFO) << "get_test:: started.";
  //
  ds_driver_->init(num_peers, app_id);
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
  LOG(INFO) << "get_test:: before get;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg= " << get_avg(TEST_DATASIZE, data) << "\n";
  
  
  int result = ds_driver_->get(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, data);
  LOG(INFO) << "get_test:: get returned " << result;
  
  LOG(INFO) << "get_test:: after get;";
  //debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, (int*) data);
  debug_print(var_name, TEST_VER, TEST_DATASIZE, TEST_NDIM, gdim, lb, ub, NULL);
  std::cout << "avg = " << get_avg(TEST_DATASIZE, data) << "\n";
  
  free(gdim);
  free(lb);
  free(ub);
  free(data);
  
  //ds_driver_->finalize();
  //
  LOG(INFO) << "get_test:: done.";
}

