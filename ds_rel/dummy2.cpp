#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

extern "C" {
#include "dataspaces.h"
}

boost::mutex mtx_;

void dummy_put(int num_peers, int appid, MPI_Comm& gcomm)
{
  mtx_.lock();
  std::cout << "dummy_put:: dspaces_init ... \n";
  dspaces_init(num_peers, appid, &gcomm, NULL);
  std::cout << "dummy_put:: dspaces_init done for appid= " << appid << "\n";
  mtx_.unlock();
  /*
  int ts=0;
  while(ts<10){
    mtx_.lock();
    
    int num;
    printf ("User number: ");
    scanf ("%d", &num);
    dspaces_lock_on_write("metadata_lock", &gcomm);
    
    uint64_t for_lb = 0;
    uint64_t *lb = &for_lb;
    uint64_t for_ub = 0;
    uint64_t *ub = &for_ub;
    dspaces_put ("number", 1, sizeof(int), 1, lb, ub, &num);
    dspaces_unlock_on_write("metadata_lock", &gcomm);

    ts++;
    
    mtx_.unlock();
  }
  */
  mtx_.lock();
  dspaces_finalize();
  std::cout << "dummy_put:: dspaces_finalize done for appid= " << appid << "\n";
  mtx_.unlock();
}

void dummy_get(int num_peers, int appid, MPI_Comm& gcomm)
{
  mtx_.lock();
  std::cout << "dummy_get:: dspaces_init ... \n";
  dspaces_init(num_peers, appid, &gcomm, NULL);
  std::cout << "dummy_get:: dspaces_init done for appid= " << appid << "\n";
  mtx_.unlock();
  /*
  int ts=0;
  while(ts<10){
    mtx_.lock();
    
    ts++;
    printf("Press enter to read a number\n");
    char enter = 0;
    while (enter != '\r' && enter != '\n') { enter = getchar(); }

    dspaces_lock_on_read("metadata_lock", &gcomm);
    int num;
    
    uint64_t for_lb = 0;
    uint64_t *lb = &for_lb;
    uint64_t for_ub = 0;
    uint64_t *ub = &for_ub;
    dspaces_get("number", 1, sizeof(int), 1, lb, ub, &num);
    dspaces_unlock_on_read("metadata_lock", &gcomm);

    printf("You get: %d\n", num);
    
    mtx_.unlock();
  }
  */
  mtx_.lock();
  dspaces_finalize();
  std::cout << "dummy_get:: dspaces_finalize done for appid= " << appid << "\n";
  mtx_.unlock();
}

int main(int argc, char **argv){
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
  int num_peers = 2;
  
  //std::vector<boost::shared_ptr<boost::thread> > t_v;
  
  boost::shared_ptr<boost::thread> t_get( new boost::thread(&dummy_get, num_peers, 0, gcomm) );
  boost::shared_ptr<boost::thread> t_put( new boost::thread(&dummy_put, num_peers, 1, gcomm) );
  
  t_get->join();
  t_put->join();
  //
  int i;
  std::cout << "Enter\n";
  std::cin >> i;
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();

  return 0;
} 
