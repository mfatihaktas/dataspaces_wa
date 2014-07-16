#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "ds_client.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"type", optional_argument, NULL, 0},
    {"num_cnodes", optional_argument, NULL, 1},
    {"app_id", optional_argument, NULL, 2},
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
        opt_map["type"] = optarg;
        break;
      case 1:
        opt_map["num_cnodes"] = optarg;
        break;
      case 2:
        opt_map["app_id"] = optarg;
        break;
      case 's':
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
  for (std::map<std::string, std::string>::iterator it=opt_map.begin(); it!=opt_map.end(); ++it){
    std::cout << it->first << " => " << it->second << '\n';
  }
  return opt_map;
}

int main(int argc , char **argv)
{
  std::string temp;
  google::InitGoogleLogging("exp");
  //
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  int num_cnodes = boost::lexical_cast<int>(opt_map["num_cnodes"]);
  int app_id = boost::lexical_cast<int>(opt_map["app_id"]);
  
  if (opt_map["type"].compare("put") == 0){
    TestClient test_client(num_cnodes-1, app_id);
    for(int i=0; i<3; i++){
      std::cout << "Enter to put_ri_msg\n";
      getline(std::cin, temp);
      
      test_client.put_ri_msg("r_get", "key1");
    }
    //test_client.put_test();
    //test_client.block();
    //std::cout << "Enter to unblock\n";
    //getline(std::cin, temp);
    //test_client.unblock();
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else if(opt_map["type"].compare("get") == 0){
    TestClient test_client(num_cnodes-1, app_id);
    test_client.get_test();
    //test_client.wait();
  }
  else if(opt_map["type"].compare("ri") == 0){
    RIManager ri_manager(app_id, num_cnodes-1, app_id);
    //usleep(1*1000*1000);
    
    std::cout << "Enter\n";
    getline(std::cin, temp);
  }
  else{
    LOG(ERROR) << "main:: unknown type= " << opt_map["type"];
  }
  
  return 0;
}