#ifndef _PATCH_EXP_H_
#define _PATCH_EXP_H_

// typedef boost::tuple<BOOST_PP_ENUM(NDIM, FIXED_REP, int)> ptuple;
// typedef std::pair<ptuple, ptuple> luptuple_pair;

// ptuple arr_to_ptuple(int* coor_)
// {
//   return boost::make_tuple(BOOST_PP_ENUM(NDIM, ARR_TO_ARG_LIST_REP, coor_) );
// }

// luptuple_pair lucoor_to_luptuple_pair(int* lcoor_, int* ucoor_)
// {
//   return std::make_pair(arr_to_ptuple(lcoor_), arr_to_ptuple(ucoor_) );
// }

// std::string luptuple_pair_to_str(luptuple_pair pair)
// {
//   std::stringstream ss;
//   ss << "<" << pair.first << ", " << pair.second << ">";
//   return ss.str();
// }

// int coor_0_[] = {0, 0};
// int coor_1_[] = {1, 1};
// int coor_2_[] = {0, 1};

// ptuple pt0 = arr_to_ptuple(coor_0_);
// ptuple pt1 = arr_to_ptuple(coor_1_);
// ptuple pt2 = arr_to_ptuple(coor_2_);
// std::cout << "pt0= " << pt0 << "\n"
//           << "pt1= " << pt1 << "\n";
// std::map<ptuple, int> point_map;
// point_map[pt0] = 0;
// point_map[pt1] = 1;
// for (std::map<ptuple, int>::iterator it = point_map.begin(); it != point_map.end(); it++)
//   std::cout << "<p= " << it->first << " : " << it->second << "> \n";

// int coor_0c_[] = {0, 0};
// int coor_1c_[] = {1, 1};
// ptuple pt0c = arr_to_ptuple(coor_0c_);
// std::cout << "point_map.count(pt0c)= " << point_map.count(pt0c) << "\n";
// std::cout << "point_map.count(pt2)= " << point_map.count(pt2) << "\n";

// std::map<luptuple_pair, int> luptuple_pair_map;
// luptuple_pair luptuple_pair_0 = lucoor_to_luptuple_pair(coor_0_, coor_1_);
// luptuple_pair luptuple_pair_1 = lucoor_to_luptuple_pair(coor_1_, coor_2_);
// luptuple_pair luptuple_pair_2 = lucoor_to_luptuple_pair(coor_0_, coor_2_);
// luptuple_pair luptuple_pair_0c = lucoor_to_luptuple_pair(coor_0c_, coor_1c_);
// // std::cout << "luptuple_pair_0= " << luptuple_pair_to_str(luptuple_pair_0) << "\n";
// luptuple_pair_map[luptuple_pair_0] = 0;
// luptuple_pair_map[luptuple_pair_1] = 1;

// std::cout << "luptuple_pair_map.count(luptuple_pair_0c)= " << luptuple_pair_map.count(luptuple_pair_0c) << "\n";
// std::cout << "luptuple_pair_map.count(luptuple_pair_2)= " << luptuple_pair_map.count(luptuple_pair_2) << "\n";

std::string intf_to_ip(std::string intf)
{
  int fd;
  struct ifreq ifr;
  // 
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  // Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name, intf.c_str(), IFNAMSIZ - 1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  // 
  return boost::lexical_cast<std::string>(inet_ntoa( ( (struct sockaddr_in*)&ifr.ifr_addr)->sin_addr) );
}

std::string temp;

void handle_char_recv(char* msg_)
{
  LOG(INFO) << "handle_char_recv:: msg_= " << msg_;
}

void handle_rimsg_recv(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_rimsg_recv:: msg_map= \n" << patch::map_to_str<std::string, std::string>(msg_map);
}

void handle_cp_recv(boost::shared_ptr<Packet> p_)
{
  LOG(INFO) << "handle_cp_recv:: p= \n" << p_->to_str();
}

void handle_dm_act(std::map<std::string, std::string> msg_map)
{
  LOG(INFO) << "handle_dm_act:: msg_map= \n" << patch::map_to_str<std::string, std::string>(msg_map);
}

void master_test(std::map<std::string, std::string> opt_map)
{
  int max_num_key_ver_in_mpbuffer = 100;
  int pexpand_length = 1;
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 10) };
  
  MSDMMaster master(boost::lexical_cast<int>(opt_map["id"] ), intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ), opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                    boost::bind(&handle_rimsg_recv, _1), boost::bind(&handle_dm_act, _1),
                    MALGO_W_PPM, max_num_key_ver_in_mpbuffer, false);
  
  // COOR_T m_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  // COOR_T m_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 2) };
  // master.reg_app(0);
  // master.put(true, "dummy", 0, m_lcoor_, m_ucoor_, 0);
  
  // std::cout << "Enter for test... \n";
  // getline(std::cin, temp);
  // COOR_T mquery_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  // COOR_T mquery_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 2) };
  // master.sdm_mquery(mquery_lcoor_, mquery_ucoor_);
  // 
  std::cout << "Enter \n";
  getline(std::cin, temp);
}

void slave_test(std::map<std::string, std::string> opt_map)
{
  MSDMSlave slave(boost::lexical_cast<int>(opt_map["id"] ), intf_to_ip(opt_map["lintf"] ), boost::lexical_cast<int>(opt_map["lport"] ), opt_map["joinhost_lip"], boost::lexical_cast<int>(opt_map["joinhost_lport"] ),
                  boost::bind(&handle_rimsg_recv, _1), boost::bind(&handle_dm_act, _1) );
  std::cout << "Enter for test... \n";
  getline(std::cin, temp);
  
  COOR_T s_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T s_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 2) };
  if (boost::lexical_cast<int>(opt_map["id"] ) == 1) {
    slave.reg_app(0);
    slave.put(true, "dummy", 0, s_lcoor_, s_ucoor_, 0);
  }
  else if (boost::lexical_cast<int>(opt_map["id"] ) == 2) {
    if (slave.get(0, true, "dummy", 0, s_lcoor_, s_ucoor_) )
      LOG(ERROR) << "main:: slave.get failed; " << LUCOOR_TO_STR(s_lcoor_, s_ucoor_);
    else
      LOG(ERROR) << "main:: slave.get succeeded; " << LUCOOR_TO_STR(s_lcoor_, s_ucoor_);
  }
  // 
  std::cout << "Enter \n";
  getline(std::cin, temp);
}

#endif //_PATCH_EXP_H_