#ifndef _XOR_H_
#define _XOR_H_

#include "mpi.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "patch.h"

class XORER {
  private:
    int num_thread, num_mpi_procs, mpi_rank;
    MPI_Comm mpi_comm;
    
  public:
    XORER(int num_thread);
    XORER(int num_thread, int argc, char** argv__);
    ~XORER();
    std::string to_str();
    
    int _xor(int size_1, void*& _1_, int size_2, void*& _2_, void*& r_);
    int _xor(int num_word, int word_size, void**& word__, void* r_);
    int parallel_xor(int num_chunk, int chunk_size, void** chunk__, void* r_);
};

#endif // _XOR_H_