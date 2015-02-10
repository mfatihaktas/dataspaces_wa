#include "lzprefetch.h"

LZAlgo::LZAlgo(int alphabet_length, char* alphabet_)
: alphabet_length(alphabet_length),
  alphabet_(alphabet_),
  parse_tree()
{
  // 
  LOG(INFO) << "LZAlgo:: constructed.";
}

int LZAlgo::add_access(char key)
{
  access_seq_vector.push_back(key);
  parse_tree.add_access(key);
  
  return 0;
}

