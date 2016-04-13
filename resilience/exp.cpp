#include <getopt.h>
#include <stdlib.h> // srand, rand

#include "xor.h"
#include "profiler.h"

std::map<std::string, std::string> parse_opts(int argc, char** argv)
{
  std::map<std::string, std::string> opt_map;
  // 
  int c;
  
  static struct option long_options[] = {
    {"type", optional_argument, NULL, 0},
    {0, 0, 0, 0}
  };
  
  while (1) {
    int option_index = 0;
    c = getopt_long (argc, argv, "s", long_options, &option_index);
  
    if (c == -1) //Detect the end of the options.
      break;
    
    switch (c) {
      case 0:
        opt_map["type"] = optarg;
        break;
      case '?':
        break; //getopt_long already printed an error message.
      default:
        break;
    }
  }
  if (optind < argc) {
    std::cout << "parse_opts:: Non-option ARGV-elements= \n";
    while (optind < argc)
      std::cout << argv[optind++] << "\n";
  }
  // 
  std::cout << "parse_opts:: opt_map= \n" << patch::map_to_str<>(opt_map);
  
  return opt_map;
}

int main(int argc, char **argv)
{
  std::string temp;
  google::InitGoogleLogging(argv[0] );
  srand(time(NULL) );
  // 
  std::map<std::string, std::string> opt_map = parse_opts(argc, argv);
  TProfiler<int> t_profiler;
  
  if (str_cstr_equals(opt_map["type"], "xor") ) {
    int num_chunk = 2; // 2 + 1;
    int chunk_size = 1024*1024*1024; // 50;
    char** chunk__ = (char**)malloc(num_chunk*sizeof(char*) );
    for (int i = 0; i < num_chunk; i++) {
      chunk__[i] = (char*)malloc(chunk_size*sizeof(char) );
      // for (int j = 0; j < chunk_size; j++)
      //   chunk__[i][j] = boost::lexical_cast<char>(rand() % 2);
    }
    // for (int j = 0; j < chunk_size; j++)
    //   chunk__[1][j] = chunk__[0][j];
    
    char* r_ = (char*)malloc(chunk_size*sizeof(char) );
    
    int num_thread = 16;
    for (int t = 1; t <= num_thread; t++) {
      XORER xorer(t);
      
      t_profiler.add_event(t, "num_thread_" + boost::lexical_cast<std::string>(t) );
      xorer.parallel_xor(num_chunk, chunk_size, (void**)chunk__, (void*)r_);
      t_profiler.end_event(t);
      // for (int i = 0; i < num_chunk; i++)
      //   std::cout << i << "_= " << patch::arr_to_str<>(chunk_size, chunk__[i] ) << "\n";
      // std::cout << "r_= " << patch::arr_to_str<char>(chunk_size, (char*)r_) << "\n";
      
      // int n = chunk_size;
      // while (--n && chunk__[num_chunk - 1][n] == r_[n] );
      // log_(INFO, "xor checked out?= " << (n == 0) )
    }
    for (int i = 0; i < num_chunk; i++) {
      free(chunk__[i] ); chunk__[i] = NULL;
    }
    free(chunk__); chunk__ = NULL;
    free(r_); r_ = NULL;
    
    log_(INFO, "t_profiler= \n" << t_profiler.to_str() )
    // 
    // char a ='a', b = 'b';
    // log_(INFO, "a^b^b= " << (char)(a ^ b ^ b) )
  }
  else {
    log_(ERROR, "unknown type= " << opt_map["type"] )
  }
  
  return 0;
}
