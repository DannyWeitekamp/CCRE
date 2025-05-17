#include "pool_allocator.h"

std::ostream& operator<<(std::ostream& out, const PoolStats& stats){
	return out << "PoolAllocator Statistics (" << stats.n_blocks << " blocks)" << endl
		// << "  n_blocks: " << stats.n_blocks << endl
		<< "  allocated_chunks: " << stats.allocated_chunks << endl
		<< "  used_chunks: " << stats.used_chunks << endl
		<< "  free_chunks: " << stats.free_chunks << endl
		<< "  allocated_bytes: " << stats.allocated_bytes << endl
		<< "  used_bytes: " << stats.used_bytes << endl
		<< "  free_bytes: " << stats.free_bytes << endl
	;
}
