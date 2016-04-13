#include "xor.h"

XORER::XORER(int num_thread)
: num_thread(num_thread),
  num_mpi_procs(0), mpi_rank(0)
{
  log_(INFO, "constructed; \n" << to_str() )
}

XORER::XORER(int num_thread, int argc, char** argv__)
: num_thread(num_thread)
{
  MPI_Init(&argc, &argv__);
  MPI_Comm_size(MPI_COMM_WORLD, &num_mpi_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Barrier(MPI_COMM_WORLD);
  mpi_comm = MPI_COMM_WORLD;
  
  log_(INFO, "constructed; \n" << to_str() )
}

XORER::~XORER() { log_(INFO, "destructed.") }

std::string XORER::to_str()
{
  std::stringstream ss;
  ss << "num_thread= " << num_thread << "\n"
     << "num_mpi_procs= " << num_mpi_procs << "\n"
     << "mpi_rank= " << mpi_rank << "\n";
  
  return ss.str();
}

int XORER::_xor(int size_1, void*& _1_, int size_2, void*& _2_, void*& r_)
{
  int size_diff = size_1 - size_2;
  if (size_diff != 0) {
    log_(WARNING, "size_diff= " << size_diff)
  }
  
  int size = size_1;
  if (size_diff > 0) {
    void* chunk_2_ = malloc(size_1*sizeof(char) );
    memcpy(chunk_2_, _2_, size_2);
    memset(static_cast<char*>(chunk_2_) + size_2, 0, size_diff);
    free(_2_);
    _2_ = chunk_2_;
  }
  else if (size_diff < 0) {
    void* chunk_1_ = malloc(size_2*sizeof(char) );
    memcpy(chunk_1_, _1_, size_1);
    memset(static_cast<char*>(chunk_1_) + size_1, 0, abs(size_diff) );
    free(_1_);
    _1_ = chunk_1_;
    size = size_2;
  }
  
  char* chunk_1_ = static_cast<char*>(_1_);
  char* chunk_2_ = static_cast<char*>(_2_);
  char* t_r_ = static_cast<char*>(r_);
  
  // r_ = malloc(size*sizeof(char) );
  for (int i = 0; i < size; i++)
    t_r_[i] = chunk_1_[i] ^ chunk_2_[i];
  
  return 0;
}

int XORER::_xor(int num_word, int word_size, void**& word__, void* r_)
{
  memset(r_, 0, word_size*sizeof(char) );
  char* t_r_ = static_cast<char*>(r_);
  
  for (int w = 0; w < num_word; w++) {
    char* word_ = static_cast<char*>(word__[w] );
    // std::cout << "word_" << w << "= " << patch::arr_to_str<>(word_size, word_) << "\n";
    for (int i = 0; i < word_size; i++)
      t_r_[i] ^= word_[i];
    
    // std::cout << "t_r_= " << patch::arr_to_str<>(word_size, t_r_) << "\n";
  }
  
  free(word__); word__ = NULL;
  return 0;
}

int XORER::parallel_xor(int num_chunk, int chunk_size, void** chunk__, void* r_)
{
  void** t_chunk__ = (void**)malloc(num_chunk*sizeof(void*) );
  for (int i = 0; i < num_chunk; i++)
    t_chunk__[i] = chunk__[i];
  // for (int i = 0; i < num_chunk; i++)
  //   std::cout << "t_chunk_" << i << "= " << patch::arr_to_str<>(chunk_size, (char*)t_chunk__[i] ) << "\n";
  
  char* t_r_ = static_cast<char*>(r_);
  
  std::vector<boost::shared_ptr<boost::thread> > thread_v;
  for (int i = 0; i < num_thread; i++) {
    int word_size = (i < num_thread - 1) ? (chunk_size / num_thread) : (chunk_size / num_thread) + chunk_size - num_thread*(chunk_size / num_thread);
    
    void** word__ = (void**)malloc(num_chunk*sizeof(void*) );
    for (int c = 0; c < num_chunk; c++)
      word__[c] = t_chunk__[c];
    
    log_(INFO, "thread_id= " << i << ", num_word= " << num_chunk << ", word_size= " << word_size)
    // boost::thread(&XORER::_xor, this, num_chunk, word_size, word__, t_r_);
    thread_v.push_back(boost::make_shared<boost::thread>(boost::bind(&XORER::_xor, this, num_chunk, word_size, word__, t_r_) ) );
    
    if (i < num_thread - 1) {
      for (int c = 0; c < num_chunk; c++)
        t_chunk__[c] = static_cast<char*>(t_chunk__[c] ) + word_size;
      t_r_ += word_size;
    }
  }
  
  for (std::vector<boost::shared_ptr<boost::thread> >::iterator it = thread_v.begin(); it != thread_v.end(); it++)
    (*it)->join();
  
  return 0;
}

