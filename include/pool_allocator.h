#include <iostream>

using std::cout;
using std::endl;
#include <sys/mman.h>

// NOTE: Real page size might not be this, but it is on most systems
//  and its a good enough default block size.
const uint64_t PAGE_SIZE = 4096;//sysconf(_SC_PAGESIZE);


struct PoolStats {
	uint64_t n_blocks = 0;
	uint64_t allocated_chunks = 0;
	uint64_t used_chunks = 0;
	uint64_t free_chunks = 0;
	uint64_t allocated_bytes = 0;
	uint64_t used_bytes = 0;
	uint64_t free_bytes = 0;

	// PoolStats() : 
	// 	n_blocks(0),
	// 	allocated_chunks(0),
	// 	used_chunks(0),
	// 	free_chunks(0),
	// 	allocated_bytes(0),
	// 	used_bytes(0),
	// 	free_bytes(0)
	// {};
};

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

template<class T>
class PoolAllocator {
public:
	// Forward Declaration
	struct Chunk;

	struct Block{
		size_t size;
		size_t capacity;
		void* start;
		void* end;
		void* write_head;
		Chunk* next_free;
		size_t free_count;
		size_t pad[1]; // Just to keep things cache-line aligned
		// std::vector<Chunk*> free_list = {};

		// ... rest of data

		Block(size_t _size, size_t items_per_block, size_t _end_offset) : 
			size(_size), 
			capacity(items_per_block), 
			start((void*)((char *) this + sizeof(Block))),
			end((void*)((char *) this + _end_offset)),
			write_head((void*)((char *) this + sizeof(Block))),
			next_free(nullptr),
			free_count(0)
		{};
	};

	struct Chunk {
		Block* block;
		Chunk* next_free;
		T data;
	};

private:
    size_t block_size;
    size_t items_per_block;
    size_t end_offset;
    // size_t page_size;
    // std::vector<T*> free_list = {};
    // std::vector<Block*> block_list = {};
    std::vector<Block*> vacant_block_list = {};
    std::vector<Block*> block_list = {};
    Block* curr_block;
    // void* write_head;

    Block* alloc_block(){
    	void* data = (void*) malloc(block_size);//aligned_alloc(block_size, page_size);
    	Block* new_block = new (data) Block(block_size, items_per_block, end_offset);
    	block_list.push_back(new_block);    	

    	// cout << "new_block:" << new_block << ", " << new_block->write_head << endl;
    	return new_block;
    };
    
public:
    PoolAllocator(size_t _block_size=PAGE_SIZE) { 
    	// block_size(sizeof(Block) + n*(sizeof(void*)+sizeof(T)) ),
    	// items_per_block(n) {

    	
    	block_size = _block_size;//PAGE_SIZE*pages_per_block;


    	items_per_block = int((block_size-sizeof(Block)) / sizeof(Chunk));
    	end_offset = sizeof(Block) + items_per_block*sizeof(Chunk);

    	// cout << "sizeof(Block): " << sizeof(Block) << endl;
    	// void* aligned_ptr = aligned_alloc(page_size, page_size); 

    	// cout << "block_size: " << block_size << endl;
    	// cout << "chunk size: " << sizeof(Chunk) << endl;
    	// cout << "items_per_block: " << items_per_block << endl;
    	// cout << "end_offset: " << end_offset << endl;
    	// cout << (block_size-sizeof(Block)) / sizeof(Chunk) << endl;

    	// A PoolAllocator has an empty block at instantiation
    	curr_block = alloc_block();
    };

    T* alloc(){
    	// auto& free_list = curr_block->next_free;

    	// If current block's free_list is not empty
    	//  grab the next chunk from its free_list
    	Chunk* __restrict chunk = nullptr;
    	if(curr_block->free_count > 0){
    		chunk = curr_block->next_free;
    		curr_block->free_count -= 1;
    		curr_block->next_free = chunk->next_free;
    		chunk->next_free = nullptr;
    		// free_list.pop_back();

    		// cout << "reuse chunk:" << chunk << " block: " << curr_block << " end: " << curr_block->end << endl;

    	// Otherwise take the next chunk from the write_head of the block
    	}else{
    		chunk = (Chunk*) curr_block->write_head;
    		chunk->block = curr_block;
    		chunk->next_free = nullptr;

    		// cout << "write chunk:" << chunk << " block: " << curr_block << " end: " << curr_block->end << endl;

    		// Advance the write head by one Chunk.
    		curr_block->write_head = ((char*) curr_block->write_head) + sizeof(Chunk);
    	}

    	bool no_free = curr_block->free_count == 0;
    	bool head_end = curr_block->write_head >= curr_block->end;
    	bool is_full = no_free & head_end;

    	// If curr_block is full then ...
		if(is_full){
			// Set a block with vacancy as the current block
			if(vacant_block_list.size() > 0){
				curr_block = vacant_block_list.back();
				vacant_block_list.pop_back();

			// If there are no vacant blocks allocate a new one
			}else{
				curr_block = alloc_block();
			}
		}    		

    	return &(chunk->data);	
    };

    void dealloc(T* ptr){
    	Chunk* chunk = (Chunk*) ((char*) ptr - (sizeof(Chunk) - sizeof(T)));
    	Block* block = chunk->block;

    	// cout << "dealloc chunk:" << chunk << " block: " << block << endl;

    	bool no_free = block->free_count == 0;
    	bool head_end = block->write_head >= curr_block->end;
    	bool was_full = no_free & head_end;

    	// Add block to free linked list
    	chunk->next_free = block->next_free;
    	block->next_free = chunk;
    	block->free_count += 1;
    	// block->free_list.push_back(chunk);

    	if(was_full){
    		vacant_block_list.push_back(block);
    	}else if(block->free_count == block->capacity){
    		// cout << "FREE BLOCK" << endl;
    		delete block;
    		// free(block);

    		// Remove the block from the block_list and vacant_block_list
			block_list.erase(std::remove(block_list.begin(), block_list.end(), block), block_list.end());
			vacant_block_list.erase(std::remove(vacant_block_list.begin(), vacant_block_list.end(), block), vacant_block_list.end());
			
    	}
    };

    void _accum_block(PoolStats& stats, Block* b){
    	int64_t alloc_span = (uint64_t(b->end) - uint64_t(b->start));
    	int64_t write_span = (uint64_t(b->write_head) - uint64_t(b->start));
    	int64_t unwrittren_span = (uint64_t(b->end) - uint64_t(b->write_head));
    	// int64_t n_free = b->free_list.size();
    	int64_t n_free = b->free_count;
    	int64_t C = sizeof(Chunk);

    	// cout << "sizeof(Block): " << sizeof(Block) << endl;
    	// cout << "sizeof(Chunk): " << sizeof(Chunk) << endl;
    	// cout << "alloc_span: " << alloc_span << endl;
    	// cout << "write_span: " << write_span << endl;
    	// cout << "unwrittren_span: " << unwrittren_span << endl;
    	// cout << "b->capacity: " << b->capacity << endl;
    	// cout << "C: " << C << endl;
    	// cout << "--: " << unwrittren_span / C << endl;
    	stats.n_blocks += 1;
    	stats.allocated_chunks += b->capacity;
    	stats.used_chunks += (write_span / C) - n_free;
    	stats.free_chunks += (unwrittren_span / C) + n_free;

    	stats.allocated_bytes += alloc_span;
    	stats.used_bytes += write_span - (n_free * C);
    	stats.free_bytes += unwrittren_span + (n_free * C);
    }

    PoolStats get_stats(){
    	PoolStats stats;
    	// _accum_block(stats, curr_block);

    	for (auto it = block_list.begin(); it != block_list.end(); ++it) {
			Block* block = *it;
			_accum_block(stats, block);
		}
		return stats;
    }


};

