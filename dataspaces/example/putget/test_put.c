#include "dataspaces.h"
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "common.h"

int main(int argc, char **argv){
  int err, nprocs, rank;
  MPI_Comm gcomm;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Barrier(MPI_COMM_WORLD);
  gcomm = MPI_COMM_WORLD;
  //
	dspaces_init(1,2);
	int ts=0;
  while(ts<10){
		int num;
		printf ("User number: ");
		scanf ("%d", &num);
    dspaces_lock_on_write("metadata_lock", &gcomm);

		common_put(num);
		dspaces_unlock_on_write("metadata_lock", &gcomm);
		
    ts++;
  }
	dspaces_finalize();
  //
  MPI_Barrier(gcomm);
  MPI_Finalize();

	return 0;
}
