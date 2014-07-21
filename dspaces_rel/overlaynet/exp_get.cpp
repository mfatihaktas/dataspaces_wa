#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

extern "C" {
#include "common.h"
#include "dataspaces.h"
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
  dspaces_init(1,3);

  int ts=0;
  while(ts<10){
    ts++;

		printf("Press enter to read a number\n");
		char enter = 0;
		while (enter != '\r' && enter != '\n') { enter = getchar(); }

    dspaces_lock_on_read("metadata_lock", &gcomm);
    int num;
		common_get(&num);
    dspaces_unlock_on_read("metadata_lock", &gcomm);
		
		printf("You get: %d\n", num);
  }
  dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();

  return 0;
} 

