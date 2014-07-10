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
    
    void dummy_put()
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
        int num;
        printf ("User number: ");
        scanf ("%d", &num);
        
        int *num_ = (int*)malloc(sizeof(int));
        num_[0] = num;
        
        dspaces_lock_on_write("metadata_lock", &gcomm);
        /*
        uint64_t for_lb = 0;
        uint64_t *lb = &for_lb;
        uint64_t for_ub = 0;
        uint64_t *ub = &for_ub;
        */
        dspaces_put ("number", 1, sizeof(int), ndim, lb, ub, num_);
        dspaces_unlock_on_write("metadata_lock", &gcomm);
        
        dspaces_put_sync();
        
        ts++;
      }
      
      free(gdim);
      dspaces_finalize();
      //
      MPI_Barrier(gcomm);
      MPI_Finalize();
    };
    
  private:
    int num_peers, appid;
};

int main(int argc, char **argv){
  Dummy dummy(1, 2);
  dummy.dummy_put();
  
  /*
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
  dspaces_init(1,2, &gcomm, NULL);
  
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
    int num;
    printf ("User number: ");
    scanf ("%d", &num);
    
    int *num_ = (int*)malloc(sizeof(int));
    num_[0] = num;
    
    dspaces_lock_on_write("metadata_lock", &gcomm);
    
    dspaces_put ("number", 1, sizeof(int), ndim, lb, ub, num_);
    dspaces_unlock_on_write("metadata_lock", &gcomm);
    
    dspaces_put_sync();
    
    ts++;
  }
  
  free(gdim);
  dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();
  */

  return 0;
} 
