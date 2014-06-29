#ifndef _DSPACES_DRIVE_H_
#define _DSPACES_DRIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

extern "C" {
  #include "dataspaces.h"
}

class DSpacesDriver
{
  public:
    DSpacesDriver();
    ~DSpacesDriver();
    int dspaces_init_(int num_peers, int appid);
    
  private:
    int nprocs, rank;
    MPI_Comm mpi_comm;
    
};

#endif //end of _DSPACES_DRIVE_H_