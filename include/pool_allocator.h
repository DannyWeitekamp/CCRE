#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

using std::cout;
using std::endl;
#include <sys/mman.h>

// NOTE: Real page size might not be this, but it is on most systems
//  and its a good enough default block size.
const uint64_t DEFUALT_BLOCKSIZE = 4096;//sysconf(_SC_PAGESIZE);
// const uint64_t DEFUALT_BLOCKSIZE = 32768;
// const uint64_t DEFUALT_BLOCKSIZE = 8192;//sysconf(_SC_PAGESIZE);


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

std::ostream& operator<<(std::ostream& out, const PoolStats& stats);

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
		Chunk** free_list_head;
		Chunk** free_list_end;
		// uint64_t free_list_size;
		



		// std::vector<Chunk*> free_list = {};

		// ... rest of data

		Block(size_t _size, size_t items_per_block, size_t _end_offset) : 
			size(_size), 
			capacity(items_per_block), 
			start((void*)((char *) this + sizeof(Block))),
			end((void*)((char *) this + _end_offset)),
			write_head((void*)((char *) this + sizeof(Block))),
			free_list_head( (Chunk**) end ),
			free_list_end ( (Chunk**)((char *) this + _end_offset + sizeof(Chunk*)*items_per_block) )
		{	
			// cout << "free_list_head: " << uint64_t(free_list_head) << endl;
			// cout << "free_list_end:  " << uint64_t(free_list_end) << endl;
			// cout << "free_list_size: " << free_list_size() << endl;
			// free_list.reserve(items_per_block);			
		};

		inline bool is_full(){
		    bool no_free = free_list_head == end;
		    bool head_end = write_head >= end;
		    return no_free & head_end;
		}

		Chunk* free_list_pop(){
	    	Chunk* chunk = free_list_head[0];
			--free_list_head; //(Chunk**)(((char*) free_list_head) - sizeof(Chunk*));
			// free_list_size -= 1;
			return chunk;
	    }
	    void free_list_push(Chunk* &chunk){
			++free_list_head;// = (Chunk**)(((char*) free_list_head) + sizeof(Chunk*));
			std::construct_at(free_list_head, std::move(chunk));
			// free_list_head[0] = chunk;
			// free_list_size += 1;
	    }

	    inline size_t free_list_size(){
	    	// cout << uint64_t(free_list_head) << ", " << uint64_t(free_list_head) - uint64_t(end) << ", " << sizeof(Chunk*) << endl;
	    	return (uint64_t(free_list_head) - uint64_t(end)) / sizeof(Chunk*);
	    }

	    inline bool any_free(){
	    	return free_list_head != end;
	    }

	    inline bool all_free(){
	    	return free_list_head == free_list_end;	
	    }
	};

	struct Chunk {
		Block* block;
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

    Block* alloc_block(size_t _block_size){
    	// cout << "ALLOC BLOCK" << endl;
    	void* data = (void*) malloc(_block_size);
    	// void* data = (void*) aligned_alloc(block_size, DEFUALT_BLOCKSIZE);
    	Block* new_block = new (data) Block(_block_size, items_per_block, end_offset);
    	block_list.push_back(new_block);    	

    	// cout << "new_block:" << new_block << ", " << _block_size << endl;
    	return new_block;
    };
    
public:
    PoolAllocator(size_t _block_size=DEFUALT_BLOCKSIZE) :
    	block_size(_block_size)
    { 
    	// block_size(sizeof(Block) + n*(sizeof(void*)+sizeof(T)) ),
    	// items_per_block(n) {

    	
    	// block_size = _block_size;//PAGE_SIZE*pages_per_block;


    	items_per_block = int((block_size-sizeof(Block)) / (sizeof(Chunk*) + sizeof(Chunk)));
    	end_offset = sizeof(Block) + items_per_block*sizeof(Chunk);

    	// cout << "sizeof(Block): " << sizeof(Block) << endl;
    	// void* aligned_ptr = aligned_alloc(page_size, page_size); 

    	// cout << "block_size: " << block_size << endl;
    	// cout << "chunk size: " << sizeof(Chunk) << endl;
    	// cout << "items_per_block: " << items_per_block << endl;
    	// cout << "end_offset: " << end_offset << endl;
    	// cout << (block_size-sizeof(Block)) / sizeof(Chunk) << endl;

    	// A PoolAllocator has an empty block at instantiation

    	curr_block = alloc_block(block_size);
    	// cout << "START POOL ALLOC" << uint64_t(curr_block) << endl;
    };

    ~PoolAllocator(){
    	// cout << "DESTROY POOL ALLOC" << uint64_t(curr_block) << endl;
    	// cout << "DESTROY POOL ALLOC:" << vacant_block_list.size() << endl;
    	// cout << "DESTROY POOL ALLOC:" << block_list.size() << endl;
    	if(curr_block != nullptr){
    		free(curr_block);	
    	}
    	for(auto block : vacant_block_list){
    		free(block);
    	}
    }


    

    T* alloc(){
    	// cout << "Alloc" << endl;
    	// auto& free_list = curr_block->free_list;

    	// If current block's free_list is not empty
    	//  grab the next chunk from its free_list
    	Chunk* __restrict chunk = nullptr;
    	if(curr_block->any_free()){
    		chunk = curr_block->free_list_pop();

    		// chunk = free_list.back();
    		// free_list.pop_back();

    		// cout << "reuse chunk:" << chunk << " block: " << curr_block << " end: " << curr_block->end << endl;

    	// Otherwise take the next chunk from the write_head of the block
    	}else{
    		chunk = (Chunk*) curr_block->write_head;
    		chunk->block = curr_block;

    		// cout << "write chunk:" << chunk << " block: " << curr_block << " end: " << curr_block->end << endl;

    		// Advance the write head by one Chunk.
    		curr_block->write_head = (void*) (uint64_t(curr_block->write_head) + sizeof(Chunk));
    	}

    	// If curr_block is full then ...
		if(curr_block->is_full()){
			// Set a block with vacancy as the current block
			if(vacant_block_list.size() > 0){
				curr_block = vacant_block_list.back();
				vacant_block_list.pop_back();

			// If there are no vacant blocks allocate a new one
			}else{
				curr_block = alloc_block(block_size);
			}
		}    	

    	return &(chunk->data);	
    };

    void force_fresh_block(){
    	vacant_block_list.push_back(curr_block);
    	curr_block = alloc_block(block_size);
    }

    T* alloc_forward(){
		Chunk* __restrict chunk = (Chunk*) curr_block->write_head;
		chunk->block = curr_block;
		// cout << "write chunk:" << chunk << " block: " << curr_block << " end: " << curr_block->end << endl;

		// Advance the write head by one Chunk.
		curr_block->write_head = (void*) ( ((char*) curr_block->write_head) + sizeof(Chunk) );

		if(curr_block->write_head >= curr_block->end){
			curr_block = alloc_block(block_size);
		}    	
		return &(chunk->data);	
    }


    void dealloc(T* ptr){
    	Chunk* chunk = (Chunk*) ((char*) ptr - sizeof(void*));
    	Block* block = chunk->block;

    	// cout << "dealloc chunk:" << uint64_t(chunk) << " block: " << block << endl;
    	// cout << "free_size:" << block->free_list_size() << endl;

    	// bool no_free = block->free_list.size() == 0;
    	// bool head_end = block->write_head >= block->end;
    	bool was_full = block->is_full();

    	// block->free_list.push_back(chunk);
    	block->free_list_push(chunk);
    	

    	if(was_full){
    		vacant_block_list.push_back(block);

    	// Never free the current block 
    	}else if(block->all_free() && block != curr_block){
    		// cout << "FREE BLOCK" << endl;
    		// delete block;
    		free(block);

    		// Remove the block from the block_list and vacant_block_list
			block_list.erase(std::remove(block_list.begin(), block_list.end(), block), block_list.end());
			vacant_block_list.erase(std::remove(vacant_block_list.begin(), vacant_block_list.end(), block), vacant_block_list.end());
			
    	}
    };

    void _accum_block_stats(PoolStats& stats, Block* b){
    	int64_t alloc_span = (uint64_t(b->end) - uint64_t(b->start));
    	int64_t write_span = (uint64_t(b->write_head) - uint64_t(b->start));
    	int64_t unwrittren_span = (uint64_t(b->end) - uint64_t(b->write_head));
    	int64_t n_free = b->free_list_size();
    	int64_t C = sizeof(Chunk);

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
			_accum_block_stats(stats, block);
		}
		return stats;
    }

    // -------------------------------------------
    // : BatchIterator

    class BatchIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

    private:
        PoolAllocator<T>* pool;
        size_t remaining;
        size_t block_size;
        Block* curr_block;
        Chunk* curr_chunk;
        Chunk* block_end;

    public:
    	inline void _next_block(){
    		// cout << "ALLOC BLOCK: " << block_size << endl;
    		curr_block = pool->alloc_block(block_size);
        	curr_chunk = (Chunk*) curr_block->write_head;
        	// curr_chunk->block = curr_block;
        	block_end = (Chunk*) curr_block->end;
    	}

        BatchIterator(PoolAllocator<T>* _pool, size_t n, size_t _block_size) :
        	pool(_pool),
        	remaining(n),
        	block_size(_block_size),
        	curr_block(nullptr),
        	curr_chunk(nullptr),
        	block_end(nullptr)
        {
        	if(n == 0){
        		curr_chunk = nullptr;
        	}else{
        		_next_block();	
        	}
        	
        }

        ~BatchIterator(){
        	if(curr_block && !curr_block->is_full()){
        		pool->vacant_block_list.push_back(curr_block);
        	}
        }

        reference operator*() const {curr_chunk->block = curr_block; return curr_chunk->data; }
        pointer operator->() const {curr_chunk->block = curr_block; return &(curr_chunk->data); }
        BatchIterator& operator++() {
        	curr_chunk++;
        	remaining--;
        	curr_block->write_head = curr_chunk;
        	// curr_chunk->block = curr_block;

        	if(remaining > 0){
        		if(curr_chunk >= block_end){
        			_next_block();
        		}
        	}else{
        		curr_chunk = nullptr;
        	}
        	return *this;
        }
        bool operator!=(const BatchIterator& other) const { return curr_chunk != other.curr_chunk;}
        bool operator==(const BatchIterator& other) const { return curr_chunk == other.curr_chunk;}
    };

    struct BatchSpec {
    	PoolAllocator<T>* pool;
    	size_t total;
    	size_t block_size;

    	BatchSpec(PoolAllocator<T>* _pool, size_t n, size_t _block_size) : 
    		pool(_pool), total(n),
    		block_size(_block_size)
    	{}

		BatchIterator begin() {return BatchIterator(pool, total, block_size);}
		BatchIterator end() {return BatchIterator(pool, 0, block_size);} 	
    };

    // Iterator begin() { return  BatchIterator(facts.begin(), facts.end());}
    // Iterator end() { return  Iterator(facts.end(), facts.end());}


public:
    // Add this new method
    BatchSpec alloc_batch(size_t n, size_t _block_size=0) {
    	if(_block_size == 0){
    		_block_size = std::min(
    			sizeof(Block) + n*(sizeof(Chunk)+sizeof(Chunk*)),
    			DEFUALT_BLOCKSIZE*2
    		);
    	}
        return BatchSpec(this, n, _block_size);
    }


};

