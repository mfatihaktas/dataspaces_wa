#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

extern "C" {
  #include "dataspaces.h"
}

class Dummy
{
  public:
    Dummy(int num_peers, int appid)
    : num_peers(num_peers),
      appid(appid)
    {
      std::cout << "Dummy:: constructed.\n";
    };
    
    ~Dummy()
    {
      std::cout << "Dummy:: desstructed.\n";
    };
    
    void dummy_get()
    {
      int err, nprocs, rank;
      MPI_Comm gcomm;
      MPI_Init(NULL, NULL);
      MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      MPI_Barrier(MPI_COMM_WORLD);
      gcomm = MPI_COMM_WORLD;
      //
      dspaces_init(num_peers,appid, &gcomm, NULL);
      
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
      
      dspaces_define_gdim("number", ndim, gdim);
      
      int ts=0;
      while(ts<10){
        ts++;
    
        printf("Press enter to read a number\n");
        char enter = 0;
        while (enter != '\r' && enter != '\n') { enter = getchar(); }
    
        //int num;
        int* num_ = (int*)malloc(sizeof(int));
        
        dspaces_lock_on_read("metadata_lock", &gcomm);
        dspaces_get("number", 1, sizeof(int), 1, lb, ub, num_);
        dspaces_unlock_on_read("metadata_lock", &gcomm);
        
        printf("You get: %d\n", num_[0]);
        
        free(num_);
      }
      dspaces_finalize();
      //
      MPI_Barrier(gcomm);
      MPI_Finalize();
    };
    
  private:
    int num_peers, appid;
};

int main(int argc, char **argv){
  Dummy dummy(1, 3);
  dummy.dummy_get();
  /*
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
  dspaces_init(1,3, &gcomm, NULL);
  
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
  
  dspaces_define_gdim("number", ndim, gdim);
  
  int ts=0;
  while(ts<10){
    ts++;

    printf("Press enter to read a number\n");
    char enter = 0;
    while (enter != '\r' && enter != '\n') { enter = getchar(); }

    //int num;
    int* num_ = (int*)malloc(sizeof(int));
    
    dspaces_lock_on_read("metadata_lock", &gcomm);
    dspaces_get("number", 1, sizeof(int), 1, lb, ub, num_);
    dspaces_unlock_on_read("metadata_lock", &gcomm);
    
    printf("You get: %d\n", num_[0]);
    
    free(num_);
  }
  dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();
  */
  return 0;
} 
