#include "sdm_control.h"

/****************************************  SimpleMaster  ******************************************/
SimpleMaster::SimpleMaster(char id, std::string type,
                           std::string lip, int lport,
                           std::string joinhost_lip, int joinhost_lport,
                           func_rimsg_recv_cb _rimsg_recv_cb)
: SDMMaster(id, type, lip, lport, joinhost_lip, joinhost_lport,
            _rimsg_recv_cb)
{
  // 
  LOG(INFO) << "SimpleMaster:: constructed";
}

SimpleMaster::~SimpleMaster() { LOG(INFO) << "SimpleMaster:: destructed"; }

void SimpleMaster::handle_conn_up(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_conn_up:: msg_map= \n" << patch_sdm::map_to_str<>(msg_map);
}

void SimpleMaster::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sdm::map_to_str<>(msg_map);
}

/*****************************************  SimpleSlave  ******************************************/
SimpleSlave::SimpleSlave(char id, std::string type,
                         std::string lip, int lport,
                         std::string joinhost_lip, int joinhost_lport,
                         func_rimsg_recv_cb _rimsg_recv_cb)
: SDMSlave(id, type, lip, lport, joinhost_lip, joinhost_lport,
           _rimsg_recv_cb)
{
  // 
  LOG(INFO) << "SimpleSlave:: constructed";
}

SimpleSlave::~SimpleSlave() { LOG(INFO) << "SimpleSlave:: destructed"; }

void SimpleSlave::handle_msg_in(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_msg_in:: msg_map= \n" << patch_sdm::map_to_str<>(msg_map);
}
