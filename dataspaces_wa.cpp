#include "dataspaces_wa.h"

WADspacesDriver::WADspacesDriver(int app_id, int num_local_peers)
: app_id(app_id),
  num_local_peers(num_local_peers),
  bc_client_( new BCClient(app_id, num_local_peers, RI_MSG_SIZE, "ri_req_") )
{
  //
  LOG(INFO) << "WADspacesDriver:: constructed.";
}

WADspacesDriver::~WADspacesDriver()
{
  //
  LOG(INFO) << "WADspacesDriver:: destructed.";
}





int main()
{
  LOG(INFO) << "main:: ...";
  return 0;
}