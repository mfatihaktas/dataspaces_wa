//for intf_to_ip
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
//
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
//
#include "ib_delivery.h"

char* intf_to_ip(const char* intf)
{
  int fd;
  struct ifreq ifr;
  //
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  //Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;
  //Copy the interface name in the ifreq structure
  std::memcpy(ifr.ifr_name , intf , IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  //
  return inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
}

std::map<char*, char*> parse_opts(int argc, char** argv)
{
  std::map<char*, char*> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"port", optional_argument, NULL, 1},
    {"s_addr", optional_argument, NULL, 2},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    int option_index = 0;
    c = getopt_long (argc, argv, "s",
                     long_options, &option_index);

    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c)
    {
      case 0:
        opt_map[(char*)"type"] = optarg;
        break;
      case 1:
        opt_map[(char*)"port"] = optarg;
        break;
      case 2:
        opt_map[(char*)"s_addr"] = optarg;
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc){
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  //
  std::cout << "opt_map=\n";
  for (std::map<char*, char*>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

#define data_type int
const std::string data_type_str = "int";

size_t total_recved_size = 0;
void recv_handler(std::string key, unsigned int ver, size_t data_size, void* data_)
{
  total_recved_size += data_size;
  LOG(INFO) << "recv_handler:: for <key= " << key << ", ver= " << ver << ">, recved data_size= " << data_size << ", total_recved_size= " << (float)total_recved_size/(1024*1024) << "MB";
  
  // size_t length = data_size/sizeof(data_type);
  // for (int i = 0; i < length; i++){
  //   std::cout << static_cast<data_type*>(data_)[i] << ",";
  // }
  // std::cout << "\n";
  
  free(data_);
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<char*, char*> opt_map = parse_opts(argc, argv);
  
  std::string ib_lports[] = {"1234","1235","1236","1237"};
  std::list<std::string> ib_lport_list(ib_lports, ib_lports + sizeof(ib_lports) / sizeof(std::string) );
  
  DDManager dd_manager(ib_lport_list);
  if (strcmp(opt_map[(char*)"type"], (char*)"server") == 0){
    // boost::shared_ptr<IBServer<data_type> > ib_server_{
    //   new IBServer<data_type>(opt_map[(char*)"port"], boost::bind(&recv_handler, _1, _2) );
    // };
    // IBServer<data_type> *ib_server_ = new IBServer<data_type>(opt_map[(char*)"port"], boost::bind(&recv_handler, _1, _2) );
    // ib_server_->init();
    
    // IBServer<data_type> ib_server(opt_map[(char*)"port"], boost::bind(&recv_handler, _1, _2) );
    // ib_server.init();
    
    dd_manager.init_ib_server("dummy", 0, data_type_str, opt_map[(char*)"port"], boost::bind(&recv_handler, _1, _2, _3, _4) );
    
    std::cout << "main:: deneme...\n";
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"client") == 0){
    size_t data_length = 1024*1024*256;
    void* data_ = (void*)malloc(sizeof(data_type)*data_length);
    
    for (int i = 0; i < data_length; i++){
      static_cast<data_type*>(data_)[i] = (data_type)i*1.2;
    }
    
    // IBClient<data_type> ib_client(opt_map[(char*)"s_addr"], opt_map[(char*)"port"],
    //                               data_length, data_);
    // ib_client.init();
    
    dd_manager.init_ib_client(opt_map[(char*)"s_addr"], opt_map[(char*)"port"],
                              data_type_str, data_length, data_);
    
    free(data_);
    // std::cout << "Enter\n";
    // getline(std::cin, temp);
  }
  else if (strcmp(opt_map[(char*)"type"], (char*)"bqueue") == 0){
    // BQueue<int> bq;
    //bq.create_timed_push_thread(12);
    
    std::string ib_lport = dd_manager.get_next_avail_ib_lport();
    LOG(INFO) << "main:: ib_lport= " << ib_lport;
    ib_lport = dd_manager.get_next_avail_ib_lport();
    LOG(INFO) << "main:: ib_lport= " << ib_lport;
  }
  else{
     LOG(ERROR) << "main:: unknown type= " << opt_map[(char*)"type"];
  }
  
  return 0;
}