// Copyright (C) 2005, 2006 Douglas Gregor.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// A test of the broadcast() collective.
#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <algorithm>
#include "gps_position.hpp"
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/mpi/skeleton_and_content.hpp>
#include <boost/iterator/counting_iterator.hpp>
//#include "debugger.hpp"

#include "mpi_test_utils.hpp"

using boost::mpi::communicator;

using boost::mpi::packed_skeleton_iarchive;
using boost::mpi::packed_skeleton_oarchive;

template<typename T>
int
broadcast_test(const communicator& comm, const T& bc_value,
               const char* kind, int root = -1)
{
  int failed = 0;
  if (root == -1) {
    for (root = 0; root < comm.size(); ++root)
      broadcast_test(comm, bc_value, kind, root);
  } else {
    using boost::mpi::broadcast;

    T value;
    if (comm.rank() == root) {
      value = bc_value;
      std::cout << "Broadcasting " << kind << " from root " << root << "...";
      std::cout.flush();
    }

    broadcast(comm, value, root);
    BOOST_MPI_CHECK(value == bc_value, failed);
    if (comm.rank() == root && value == bc_value)
      std::cout << "OK." << std::endl;
  }

  (comm.barrier)();
  return failed;
}

int
test_skeleton_and_content(const communicator& comm, int root = 0)
{
  using boost::mpi::content;
  using boost::mpi::get_content;
  using boost::make_counting_iterator;
  using boost::mpi::broadcast;

  int failed = 0;
  int list_size = comm.size() + 7;
  if (comm.rank() == root) {
    // Fill in the seed data
    std::list<int> original_list;
    for (int i = 0; i < list_size; ++i)
      original_list.push_back(i);

    // Build up the skeleton
    packed_skeleton_oarchive oa(comm);
    oa << original_list;

    // Broadcast the skeleton
    std::cout << "Broadcasting integer list skeleton from root " << root
              << "..." << std::flush;
    broadcast(comm, oa, root);
    std::cout << "OK." << std::endl;

    // Broadcast the content
    std::cout << "Broadcasting integer list content from root " << root
              << "..." << std::flush;
    {
      content c = get_content(original_list);
      broadcast(comm, c, root);
    }
    std::cout << "OK." << std::endl;

    // Reverse the list, broadcast the content again
    std::reverse(original_list.begin(), original_list.end());
    std::cout << "Broadcasting reversed integer list content from root "
              << root << "..." << std::flush;
    {
      content c = get_content(original_list);
      broadcast(comm, c, root);
    }
    std::cout << "OK." << std::endl;

  } else {
    // Allocate some useless data, to try to get the addresses of the
    // list<int>'s used later to be different across processes.
    std::list<int> junk_list(comm.rank() * 3 + 1, 17);

    // Receive the skeleton
    packed_skeleton_iarchive ia(comm);
    broadcast(comm, ia, root);

    // Build up a list to match the skeleton, and make sure it has the
    // right structure (we have no idea what the data will be).
    std::list<int> transferred_list;
    ia >> transferred_list;
    BOOST_MPI_CHECK((int)transferred_list.size() == list_size, failed);

    // Receive the content and check it
    broadcast(comm, get_content(transferred_list), root);
    bool list_content_ok = std::equal(make_counting_iterator(0),
				      make_counting_iterator(list_size),
				      transferred_list.begin());
    BOOST_MPI_CHECK(list_content_ok, failed);

    // Receive the reversed content and check it
    broadcast(comm, get_content(transferred_list), root);
    bool rlist_content_ok = std::equal(make_counting_iterator(0),
				       make_counting_iterator(list_size),
				       transferred_list.rbegin());
    BOOST_MPI_CHECK(rlist_content_ok, failed);
    if (!(list_content_ok && rlist_content_ok)) {
      if (comm.rank() == 1) {
	std::cout
	  << "\n##### You might want to check for BOOST_MPI_BCAST_BOTTOM_WORKS_FINE "
	  << "in boost/mpi/config.hpp.\n\n";
      }
    }
  }

  (comm.barrier)();
  return failed;
}

int main() 
{
  boost::mpi::environment env;
  communicator comm;
  
  int failed = 0;
  BOOST_MPI_CHECK(comm.size() > 1, failed);
  if (failed == 0) {

    // Check transfer of individual objects
    BOOST_MPI_COUNT_FAILED(broadcast_test(comm, 17, "integers"), failed);
    BOOST_MPI_COUNT_FAILED(broadcast_test(comm, gps_position(39,16,20.2799), "GPS positions"), failed);
    BOOST_MPI_COUNT_FAILED(broadcast_test(comm, gps_position(26,25,30.0), "GPS positions"), failed);
    BOOST_MPI_COUNT_FAILED(broadcast_test(comm, std::string("Rosie"), "string"), failed);
    
    std::list<std::string> strings;
    strings.push_back("Hello");
    strings.push_back("MPI");
    strings.push_back("World");
    BOOST_MPI_COUNT_FAILED(broadcast_test(comm, strings, "list of strings"), failed);
    
    BOOST_MPI_COUNT_FAILED(test_skeleton_and_content(comm, 0), failed);
    BOOST_MPI_COUNT_FAILED(test_skeleton_and_content(comm, 1), failed);
  }
  return failed;
}
