#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <map>

#include "gridftp.cpp"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  //
  int c;
  
  static struct option long_options[] =
  {
    {"src_url", required_argument, NULL, 0},
    {"dst_url", required_argument, NULL, 1},
    {"port", required_argument, NULL, 2},
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
        opt_map["src_url"] = optarg;
        break;
      case 1:
        opt_map["dst_url"] = optarg;
        break;
      case 2:
        opt_map["port"] = optarg;
        break;
      case 's':
        opt_map["s"] = 
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
  google::InitGoogleLogging("exp_gridftp");
  //
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  
  GridFTP gftp;
  if (opt_map.count("s")){
    gftp.init_server(5000);
  }
  else{
    gftp.init_file_transfer(opt_map["src_url"], opt_map["dst_url"]);
  }
  
  return 0;
}