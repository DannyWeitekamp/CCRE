#include <vector>
#include <iostream>
#include <iterator>
#include <typeinfo>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/core/demangle.hpp>

#include "mpi_test_utils.hpp"

//#include "debugger.cpp"
namespace mpi = boost::mpi;

template<typename T>
bool test(mpi::communicator const& comm, std::vector<T> const& ref, bool iswap, bool alloc)
{
  
  int rank = comm.rank();
  if (rank == 0) {
    std::cout << "Testing with type " << boost::core::demangle(typeid(T).name()) << '\n';
    if (iswap) {
      std::cout << "Blockin send, non blocking receive.\n";
    } else {
      std::cout << "Non blockin send, blocking receive.\n";
    }
    if (alloc) {
      std::cout << "Explicitly allocate space for the receiver.\n";
    } else {
      std::cout << "Do not explicitly allocate space for the receiver.\n";
    }
  }
  if (rank == 0) {
    std::vector<T> data;
    if (alloc) {
      data.resize(ref.size());
    }
    if (iswap) {
      mpi::request req = comm.irecv(1, 0, data);
      req.wait();
    } else {
      comm.recv(1, 0, data);
    }
    std::cout << "Process 0 received " << data.size() << " elements :" << std::endl;
    std::copy(data.begin(), data.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "While expecting " << ref.size() << " elements :" << std::endl;
    std::copy(ref.begin(),  ref.end(),  std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
    return (data == ref);
  } else {
    if (rank == 1) {
      std::vector<T> vec = ref;
      if (iswap) {
        comm.send(0, 0, vec);
      } else {
        mpi::request req = comm.isend(0, 0, vec);
        req.wait();
      }
    } 
    return true;
  }
}

int
main() {
  mpi::environment env;
  mpi::communicator world;
 
  if (world.size() == 1) {
    return -1;
  }
  
  std::vector<int> integers(13); // don't assume we're lucky
  for(int i = 0; i < int(integers.size()); ++i) {
    integers[i] = i;
  }

  std::vector<std::string> strings(13); // don't assume we're lucky
  for(int i = 0; i < int(strings.size()); ++i) {
    std::ostringstream fmt;
    fmt << "S" << i;
    strings[i] = fmt.str();
  }

  std::vector<int> empty;
  int failed = 0;
  BOOST_MPI_CHECK(test(world, empty, false, true), failed);
  BOOST_MPI_CHECK(test(world, empty, false, false), failed);

  BOOST_MPI_CHECK(test(world, integers, true,  true), failed);
  BOOST_MPI_CHECK(test(world, integers, true,  false), failed);
  BOOST_MPI_CHECK(test(world, strings, true,  true), failed);
  BOOST_MPI_CHECK(test(world, strings, true,  false), failed);

  BOOST_MPI_CHECK(test(world, integers, false,  true), failed);
  BOOST_MPI_CHECK(test(world, integers, false,  false), failed);
  BOOST_MPI_CHECK(test(world, strings, false,  true), failed);
  BOOST_MPI_CHECK(test(world, strings, false,  false), failed);
  return failed;
}
