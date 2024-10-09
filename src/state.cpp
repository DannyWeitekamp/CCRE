#include <string>
#include <iostream>
#include <sstream> 
#include <bit>
#include <cstdint>
#include <inttypes.h>
#include <vector>
#include <list>
#include <unordered_map>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <tuple>

// #include "types.h"
// #include "../include/unicode.h"
#include "../include/item.h"
#include "../include/types.h"
#include "../include/state.h"

#include <memory>
#include <stdexcept>
#include <iterator>
#include <cstddef>

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

using namespace std;

// FactHeader Implementation
Item* FactHeader::get(uint32_t a_id){
    Item* data_ptr = bit_cast<Item*>(
        bit_cast<uint64_t>(this) + sizeof(FactHeader)
    );
    return &data_ptr[a_id];
}
  

FactHeader::FactHeader(uint32_t _length){
    // cout << "LL: " << length << "\n";
    f_id = 0;
    length = _length;
    t_id = T_ID_FACT;
}




vector<Item*> fact_get_items(FactHeader& fact){
  vector<Item*> out;
  out.reserve(fact.length);
  for(int i=0; i < fact.length; i++){
    Item* item = fact.get(i);
    out.push_back(item);
  }
  return out;
}

string fact_to_string(FactHeader& fact){
  stringstream ss;
  ss << "FactHeader(";  

  // vector<Item> items = fact_get_items(fact);
  size_t L = fact.length;
  for(int i=0; i < L; i++){
    Item* item = fact.get(i);
    ss << repr_item(*item);
    if(i != L-1){
      ss << ", ";  
    }    
  }
  ss << ")";  
  return ss.str();
}



// struct BlockRange{
//   Block* start;
//   size_t size;
// };




// State Implementation

State::State(size_t _size){
  if(_size < 1){
    throw bad_alloc();
  }
  size = _size;
  // free_blocks = {};
  first_empty_f_id = -1;
  // first_empty_f_id = -1;
  // last_empty_f_id = -1;
  meta_data = (void*) 0;
  data = (FactHeader*) malloc(size * sizeof(Block));

  // ObjItem state_item;
  // state_item.data = (void *) this;
  // state_item.t_id = T_ID_STATE;

  ObjItem* self_data = (ObjItem*) data;// = bit_cast<FactHeader>(state_item);
  self_data->data = (void*) this;//bit_cast<uint64_t>(this);//(void *) &(*this);
  self_data->t_id = T_ID_FACTSET;
  head = data+1;

  // cout << "WEEE";

  if(data == NULL){
    throw bad_alloc();
  }
}

State::~State(){
  cout << "freeing state data..."; 
  free(data);
}

void State::print_layout(){
  FactHeader* fact = data+1;
  // while(head-fact > 0){
  cout << "LAYOUT: " << endl;
  while(fact != head){
    if(fact->t_id == T_ID_EMPTY_BLOCK){
      EmptyBlock* eblock = (EmptyBlock*) fact;
      auto f_id = ((FactHeader*)eblock)-data;
      EmptyBlock* last_block = eblock + eblock->length;

      cout << "  EMPTY[" << f_id << ", " << f_id + eblock->length+1 << "]" << "L_LAST: " << last_block->length << endl;
    }else{
      cout << "  FACT[" << fact->f_id << ", " << fact->f_id + fact->length+1 << "]" << endl;
    }      
    fact += 1+fact->length;
  }
  // cout << "LAST: " << fact-data << " HEAD: " << head-data << endl;
  print_empties();
  
  cout << "LAST: " << fact-data << " HEAD: " << head-data << endl;
  cout << endl;
}

void State::print_empties(){
  int64_t e_f_id = first_empty_f_id; 
  cout << "REACHABLE EMPTIES: ";
  if(e_f_id == -1){
    cout << "None";
  }
  cout << endl;
  
  while(e_f_id != -1){
    // int64_t f_id = ((FactHeader*)eblock)-data;
    EmptyBlock* eblock = (EmptyBlock*) &data[e_f_id];
    // cout << bit_cast<uint64_t>(eblock) << ", " << bit_cast<uint64_t>(data) << endl;

    cout << "  EMPTY[" << e_f_id << ", " << e_f_id + eblock->length+1 << "]" << " NEXT: " << eblock->next_f_id << " PREV: " << eblock->prev_f_id  << endl;

    if(eblock->prev_f_id == 0 || eblock->next_f_id == 0){
      throw bad_alloc();
    }

    if(eblock->next_f_id == e_f_id){
      cout << "Bad self reference" << endl;
      break;
    }
    e_f_id = eblock->next_f_id;
  }
}


FactHeader* alloc_fact_block(State* state, size_t size, bool recycle_blocks){
  size_t alloc_size = size+1;
  // cout << "ALLOC SIZE:" << alloc_size << " RECYLE: " << recycle_blocks <<
  //  " HEAD: " << (state->head-state->data) << " STATE_SIZE: " << state->size << "\n";

  FactHeader* data = state->data;
  EmptyBlock* bdata = (EmptyBlock*) data;

  // /* Look for a free block at head */
  FactHeader* head;
  if(state->size-(state->head-state->data) >= alloc_size){
    // cout << "ALLOC FROM HEAD @" << (state->head-state->data) << endl;    
    head = state->head;
    state->head += alloc_size;
    return head;
  }

  // cout << "ALLOC: " << state->first_empty_f_id << endl;
  // cout << "FIRST EMPTY: " << state->first_empty_f_id << endl;

  // /* Look for a free block in the freed blocks */
  if(recycle_blocks && state->first_empty_f_id != -1){
    int64_t first_empty_f_id = state->first_empty_f_id;
    int64_t free_f_id = first_empty_f_id;
    
    while (free_f_id != -1){

      EmptyBlock* free_block = &bdata[free_f_id];;
      // If block is same size then remove the free block
      if(free_block->length >= size){
        // Remove the block from the linked list
        if(state->first_empty_f_id == free_f_id){
          state->first_empty_f_id = free_block->next_f_id;
        }
        if(free_block->prev_f_id != -1) 
          (&bdata[free_block->prev_f_id])->next_f_id = free_block->next_f_id;
        if(free_block->next_f_id != -1) 
          (&bdata[free_block->next_f_id])->prev_f_id = free_block->prev_f_id;
      }

      // If free_block too large split off the rest into a new block
      if(free_block->length > size){
        
        // BlockRange new_block;
        int64_t new_f_id = free_f_id+alloc_size;
        EmptyBlock* new_block = free_block+alloc_size;
        new_block->length = free_block->length-alloc_size;
        new_block->t_id = T_ID_EMPTY_BLOCK;

        // Make the last block covered by this empty point back to it
        EmptyBlock* last_block = new_block + new_block->length;
        last_block->t_id = T_ID_EMPTY_BLOCK;
        last_block->length = new_block->length;

        // Add the new block to the front of the empty linked list
        new_block->prev_f_id = -1;
        new_block->next_f_id = state->first_empty_f_id;
        if(state->first_empty_f_id != -1){
          (&bdata[state->first_empty_f_id])->prev_f_id = new_f_id;
        }
        state->first_empty_f_id = new_f_id;
      }

      if(free_block->length >= size){
        break;
      }

      // cout << "LOOP:" << ((FactHeader*)free_block)-state->data << endl;

      // If for some reason a corrupted leads to no progress
      //  fail gracefully by exiting the loop allocating from head instead. 
      if(free_block->next_f_id == free_f_id){
        free_f_id = -1;
        break;
      }

      free_f_id = free_block->next_f_id;
      
    }

    // cout << "NEW first_empty_f_id:" << state->first_empty_f_id << endl;

    if(free_f_id != -1){
      EmptyBlock* free_block = &bdata[free_f_id];
      free_block->length = size;
      // cout << "EMIT BLOCK: @" << ((FactHeader*)free_block)-state->data << " L: " << free_block->length << endl;
      return (FactHeader *) free_block;  
    }
  }

  
  
  // If haven't found block then double the size 
  size_t grow = max(state->size, alloc_size);
  size_t realloc_size = (state->size+grow);
  
  // cout << "state data: " << state->data << "\n";
  // cout << "state size: " << state->size << "\n";
  // cout << "state head: " << state->head << "\n";
  // cout << "grow: " << grow << "\n";
  // cout << "Realloc Size: " << realloc_size << "\n";
  size_t head_diff = state->head-state->data;

  FactHeader* new_ptr = (FactHeader*) realloc(state->data, realloc_size * sizeof(Block));
  if(new_ptr == NULL){
    throw bad_alloc();
  }
  state->data = new_ptr;
  state->size = realloc_size;
  FactHeader* new_fact_ptr = state->data + head_diff;
  state->head = new_fact_ptr + alloc_size;


  // cout << "END:" << size << " "<< state->head << "\n";
  return new_fact_ptr;
}

uint32_t empty_fact_header(State* state, uint32_t length){
  // FactHeader allocation size is 1 Block for the header +N Items
  FactHeader* new_fact = alloc_fact_block(state, length);

  // Zero fill 
  memset((void*) new_fact, 0, sizeof(Block)*(1+length));

  // cout << "BLOOP" << new_fact << endl;
  new_fact->t_id = (uint16_t) T_ID_FACT;
  new_fact->f_id = (uint32_t) (new_fact-state->data);
  new_fact->length = length;
  // cout << "LEN" << endl;
  return new_fact->f_id;
}

uint32_t empty_fact_header(uint32_t length){
  State* state = new State(length);
  return empty_fact_header(state, length);
}


// Fact View Implementation

Item* FactView::get(uint32_t a_id){
  FactHeader* header = &state->data[f_id];
  return header->get(a_id);
};

FactView::FactView(State* _state, uint32_t _f_id, uint32_t _length){
  state = _state;
  f_id = _f_id;
  length = _length;
};

FactHeader* FactView::header_ptr(){
  return &state->data[f_id];
};


FactView empty_fact(State* state, uint32_t length){
  uint32_t f_id = empty_fact_header(state, length);
  return FactView(state, f_id, length);
}

// FactView empty_fact(uint32_t length){
//   State* state = new State(length);
//   uint32_t f_id = empty_fact_header(state, length);
//   return FactView(state, f_id, length);
// }


bool _contains_header(State* state, FactHeader* fact){
  uint64_t fact_ptr = bit_cast<uint64_t>(fact);
  uint64_t data_start = bit_cast<uint64_t>(state->data);
  uint64_t data_end = bit_cast<uint64_t>(state->head); 
  if(fact_ptr >= data_start && fact_ptr < data_end){
    return true;
  }
  return false;
}


FactView declare(State* state, FactView fact_view){
    FactHeader* old_ptr = fact_view.header_ptr();
    if(_contains_header(state, old_ptr)){
      return fact_view;
    }

    FactView new_view = empty_fact(state, fact_view.length);
    FactHeader* new_ptr = new_view.header_ptr();

    memcpy(new_ptr, old_ptr, sizeof(Block) * (fact_view.length+1));

    return new_view;
}



void _retract_header(State* state, FactHeader* _fact){
    

    // state->print_layout();
    // cout << "RETRACT" << endl;

    FactHeader* data = state->data;
    EmptyBlock* bdata = (EmptyBlock*) data;

    EmptyBlock* fact = (EmptyBlock*) _fact; 
    EmptyBlock* lead_block = (EmptyBlock*) fact; 

    // cout << "RETRACT:" << lead_block->t_id << endl;

    uint32_t fact_length = fact->length;

    // Clear the fact's block (except length)
    fact->next_f_id = -1;
    fact->prev_f_id = -1;
    fact->t_id = 0;
    fact->is_lead = 0;


    // cout << endl<< "Start F_ID: " << _fact->f_id << " POS: " << _fact-state->data << " LEN: " << _fact->length << endl; 
    if((FactHeader*)_fact-data > 1){ // If not first fact

      // If the prev item is part of an empty block 
      //  then make its header EmptyBlock* the leading one;
      EmptyBlock* prev_block = (EmptyBlock*) fact-1;
      if(prev_block->t_id == T_ID_EMPTY_BLOCK){
        lead_block = prev_block-prev_block->length; 

        int64_t lead_block_f_id = lead_block-bdata;
        lead_block->length += 1+fact_length;
        // fact->length = fact-lead_block;
        // cout << "PREV BLOCK EMPTY @f_id:"<< ((FactHeader*) lead_block)-data << " length: "<< prev_block->length << endl;
        // cout << "NXT: " << lead_block->next_f_id << " PREV: " << lead_block->prev_f_id << endl;

        // Remove lead_block from the linked list 
        //  It will be reinserted to the front later
        if(state->first_empty_f_id == lead_block_f_id){
          state->first_empty_f_id = lead_block->next_f_id;
        }
        if(lead_block->next_f_id != -1)
          (&bdata[lead_block->next_f_id])->prev_f_id = lead_block->prev_f_id;
        if(lead_block->prev_f_id != -1)
          (&bdata[lead_block->prev_f_id])->next_f_id = lead_block->next_f_id;
        lead_block->prev_f_id = -1;
        lead_block->next_f_id = -1;
        
        // cout << "MOO";
        // }

      }//else{
      //  lead_block->length = fact_length;
      //}
    }
    // cout << "B" << endl;

    // If the next block is an empty block then merge it with
    // the lead block are remove it from the linked list of empties
    EmptyBlock* next_block = fact+1+fact_length;
    if(state->head-(FactHeader*)next_block > 0 && next_block->t_id == T_ID_EMPTY_BLOCK){
      // cout << "NEXT BLOCK EMPTY" << endl;
      lead_block->length += 1+next_block->length;
      int64_t next_block_f_id = next_block-bdata;

      // Remove next_block from the linked list 
      if(state->first_empty_f_id == next_block_f_id){
        state->first_empty_f_id = next_block->next_f_id;
      }
      if(next_block->next_f_id != -1)
        (&bdata[next_block->next_f_id])->prev_f_id = next_block->prev_f_id;
      if(next_block->prev_f_id != -1)
        (&bdata[next_block->prev_f_id])->next_f_id = next_block->next_f_id;
      next_block->prev_f_id = -1;
      next_block->next_f_id = -1;
    }

    // cout << "C" << endl;

    // Mark the leading block and its last as empty
    //  set .length on the last item to be the leading block's length;
    //  this is used to extend empty blocks backwards.
    EmptyBlock* last_item = (EmptyBlock*) lead_block+lead_block->length;
    // cout << "LAST ITEM @:"<< ((FactHeader*) last_item)-state->data << endl;
    lead_block->t_id = T_ID_EMPTY_BLOCK;
    last_item->t_id = T_ID_EMPTY_BLOCK;
    lead_block->is_lead = true;
    last_item->is_lead = false;
    last_item->length = lead_block->length;  

    // If the lead block is not a block preceeding the removed fact
    //  then we need to add it into the linked list;
      
    int64_t lead_block_f_id = ((FactHeader*)lead_block)-data;

    // Insert the lead_block into front of the linked list 
    if(state->first_empty_f_id != -1){
      (&bdata[state->first_empty_f_id])->prev_f_id = lead_block_f_id;
      // cout << "SLOOP: " << state->first_empty_f_id << " , " << (&bdata[state->first_empty_f_id])->next_f_id << endl;

      lead_block->next_f_id = state->first_empty_f_id;
      lead_block->prev_f_id = -1;  
    }else{
      lead_block->prev_f_id = -1;  
    }    
    state->first_empty_f_id = lead_block_f_id;
    

    // if(state->first_empty_f_id == -1 ||
    //    lead_block_f_id < state->first_empty_f_id){
    //   state->first_empty_f_id = lead_block_f_id;
    //   // cout << "SET FIRST EMPTY:" << state->first_empty_f_id << endl;
    // }


    // cout << "D" << endl;
    
    // for(int i=0; i < fact->length; i++){
    //   item.t_id = T_ID_EMPTY_BLOCK
    //   item.length = item-lead_block;
    //   item.is_internal = true;
    //   item++;
    // }

    // return true;
}

bool retract(State* state, FactView fact_view){
  FactHeader* fact = fact_view.header_ptr();
  if(!_contains_header(state, fact)){
    return false;
  }
  _retract_header(state, fact);
  return true;
}

bool retract(State* state, uint32_t f_id){
  FactHeader* fact = &state->data[f_id];
  if(!_contains_header(state, fact)){
    return false;
  }
  _retract_header(state, fact);
  return true;
}

FactHeader* get(State* state, uint32_t f_id){
  return &state->data[f_id];
}

State* get_state(FactHeader* fact){
  ObjItem* state_item = (ObjItem*) fact-fact->f_id;
  return (State*) state_item->data;
}
