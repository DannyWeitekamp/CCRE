#ifndef _State_H_
#define _State_H_

// Program to calculate the sum of n numbers entered by the user
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

#include "types.h"
#include "unicode.h"

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





// Tokens are 
struct Token{
  uint64_t val;
  uint64_t hash;
  uint16_t t_id;
  uint16_t pad[3];
};

struct UnicodeToken{
  const char* data;
  uint64_t hash;
  uint16_t t_id;
  uint8_t kind;
  uint8_t is_ascii;
  uint32_t length;
};

// Object Token 
struct ObjToken{
  void* data;
  uint64_t hash;
  uint16_t t_id;
  uint16_t pad[3];
};

struct EmptyBlock{
  EmptyBlock* prev;
  EmptyBlock* next;
  uint16_t t_id;
  uint16_t is_lead;
  uint32_t length;
};


// STR CASE
Token str_to_token(string arg){
    
    // Copy the string data
    // char* cstr = (char *) malloc(sizeof(char) * arg.length());
    // memcpy(cstr, arg.data(), arg.length());
    auto tup = intern(arg);
    // InternStr* s_id = get<0>(tup);
    const char* data = get<1>(tup);

    UnicodeToken tok;
    // tok.data = cstr;
    tok.data = data;
    tok.hash = hash<string>{}(arg);//hash<string>{}(cstr); // Hash immediately
    tok.kind = 1; // TODO
    tok.is_ascii = 1; // TODO
    tok.t_id = T_ID_STR;
    tok.length = arg.length();

    // cout << "IN: " << arg << " " << arg.length() << endl;

    Token out = bit_cast<Token>(tok);
    return out;
}
Token to_token(char* arg){return str_to_token(arg);};
Token to_token(const char* arg){return str_to_token(arg);};
Token to_token(string arg){return str_to_token(arg);};


// INT CASE
Token int_to_token(int64_t arg){
    Token tok;
    tok.val = bit_cast<uint64_t>(arg);
    tok.hash = bit_cast<uint64_t>(arg);
    tok.t_id = T_ID_INT;
    return tok;
}

Token to_token(int arg){return int_to_token(arg);};
Token to_token(long arg){return int_to_token(arg);};
Token to_token(unsigned arg){return int_to_token(arg);};

// FLOAT CASE
Token float_to_token(double arg){
    Token tok;
    tok.val = bit_cast<uint64_t>(arg);
    tok.hash = bit_cast<uint64_t>(arg);
    tok.t_id = T_ID_FLOAT;
    return tok;
}

Token to_token(double arg){return float_to_token(arg);};
Token to_token(float arg){return float_to_token(arg);};

// Edge case Token is itself
Token to_token(Token arg){return arg;};


// --------------------------------------
// Token to builtin types 

bool token_get_bool(Token tok){
    return (bool) tok.val;
}

int64_t token_get_int(Token tok){
    return bit_cast<int64_t>(tok.val);
}

double token_get_float(Token tok){
    return bit_cast<double>(tok.val);
}

string_view token_get_string(Token tok){
    UnicodeToken ut = bit_cast<UnicodeToken>(tok);
    string_view s = string_view(ut.data, ut.length);
    // s.assign(ut.data, ut.length);
    // cout << "get_str(" << ut.data[0] << ") L:" << ut.length << "\n";
    return s;
}

string repr_token(Token& tok){
  uint16_t t_id = tok.t_id;
  stringstream ss;
  switch(t_id){
    case T_ID_BOOL:
      ss << token_get_bool(tok);
      break;
    case T_ID_INT:
      ss << token_get_int(tok);
      break;
    case T_ID_FLOAT:
      ss << string_format("%.6g", token_get_float(tok));
      break;
    case T_ID_STR:
      ss << "'" << token_get_string(tok) << "'";
      break;
    default:
      ss << "<token t_id=" << t_id << " @" << bit_cast<uint64_t>(&tok) << ">";     
  }  
  return ss.str();
}

string to_string(Token& tok){return repr_token(tok);}


struct Fact{
  uint32_t f_id;
  uint16_t pad0[2];
  uint64_t hash;
  uint16_t t_id;
  uint16_t pad1[1];
  uint32_t length;

  template<class T>
  void set(uint32_t a_id, T val){
    if(a_id >= length){
      throw out_of_range("Setting fact member beyond its length.");
    }
    Token tok = to_token(val);

    // Pointer to the 0th Token of the Fact
    Token* data_ptr = bit_cast<Token*>(
        bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    data_ptr[a_id] = tok;
    // uint64_t tok_ptr = this_ptr + sizeof(Fact) + a_id * sizeof(Token);
    // cout << this_ptr << "," << tok_ptr << "\n";
    // Token* ptr = bit_cast<Token*>()
    // memcpy()
    // *ptr = tok;
  };

  Token* get(uint32_t a_id){
    Token* data_ptr = bit_cast<Token*>(
        bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return &data_ptr[a_id];
  }

  // void get(uint32_t a_id, T val){
  //   if(a_id >= length){
  //     throw out_of_range("Setting fact member beyond its length.");
  //   }
  //   Token tok = to_token(val);
  //   uint64_t this_ptr = bit_cast<uint64_t>(this);
  //   uint64_t tok_ptr = this_ptr + sizeof(Fact) + a_id * sizeof(Token);
  //   // cout << this_ptr << "," << tok_ptr << "\n";
  //   // Token* ptr = bit_cast<Token*>()
  //   // *ptr = tok;
  // };

  // Fact(Type* type){
  //   f_id;
  //   t_id = type.t_id;
  //   length = type.length;
  // }
  Fact(uint32_t _length){
    // cout << "LL: " << length << "\n";
    f_id = 0;
    length = _length;
    t_id = T_ID_FACT;
  }
};



vector<Token*> fact_get_tokens(Fact& fact){
  vector<Token*> out;
  out.reserve(fact.length);
  for(int i=0; i < fact.length; i++){
    Token* tok = fact.get(i);
    out.push_back(tok);
  }
  return out;
}

string fact_to_string(Fact& fact){
  stringstream ss;
  ss << "Fact(";  

  // vector<Token> tokens = fact_get_tokens(fact);
  size_t L = fact.length;
  for(int i=0; i < L; i++){
    Token* tok = fact.get(i);
    ss << repr_token(*tok);
    if(i != L-1){
      ss << ", ";  
    }    
  }
  ss << ")";  
  return ss.str();
}

struct Block{
  uint64_t data[3];
};

struct BlockRange{
  Block* start;
  size_t size;
};




struct State{
  size_t                       size;
  //unordered_map<Token, Fact> indexer;
  EmptyBlock*                  first_empty_block;
  // uint32_t                     last_empty_f_id;
  void*                        meta_data;
  Fact*                        data;
  Fact*                        head;

  State(size_t _size=1){
    if(_size < 1){
      throw bad_alloc();
    }
    size = _size;
    // free_blocks = {};
    first_empty_block = NULL;
    // first_empty_f_id = -1;
    // last_empty_f_id = -1;
    meta_data = (void*) 0;
    data = (Fact*) malloc(size * sizeof(Block));

    // ObjToken state_token;
    // state_token.data = (void *) this;
    // state_token.t_id = T_ID_STATE;

    ObjToken* self_data = (ObjToken*) data;// = bit_cast<Fact>(state_token);
    self_data->data = (void*) this;//bit_cast<uint64_t>(this);//(void *) &(*this);
    self_data->t_id = T_ID_STATE;
    head = data+1;

    // cout << "WEEE";

    if(data == NULL){
      throw bad_alloc();
    }
  }
  ~State(){
    cout << "freeing state data..."; 
    free(data);
  }

  void print_layout(){
    Fact* fact = data+1;
    // while(head-fact > 0){
    while(fact != head){
      if(fact->t_id == T_ID_EMPTY_BLOCK){
        EmptyBlock* eblock = (EmptyBlock*) fact;
        auto f_id = ((Fact*)eblock)-data;
        cout << "EMPTY[" << f_id << ", " << f_id + eblock->length << "]" << endl;
      }else{
        cout << "FACT[" << fact->f_id << ", " << fact->f_id + fact->length << "]" << endl;
      }      
      fact += 1+fact->length;
    }
    cout << "LAST: " << fact-data << " HEAD: " << head-data << endl;

    cout << "REACHABLE EMPTIES: " << endl; 
    EmptyBlock* eblock = first_empty_block; 
    while(eblock != NULL){
      int64_t f_id = ((Fact*)eblock)-data;
      cout << bit_cast<uint64_t>(eblock) << ", " << bit_cast<uint64_t>(data) << endl;
      cout << "EMPTY[" << f_id << ", " << f_id + eblock->length << "]" << endl;
      if(eblock->next == eblock){
        cout << "Bad self reference";
        break;
      }
      eblock = eblock->next;
    }
    cout << "LAST: " << fact-data << " HEAD: " << head-data << endl;
  }

  //struct Iterator;

  
  struct Iterator  {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Fact;
        using pointer           = Fact*;
        using reference         = Fact&;

        Iterator(Fact* ptr) : f_ptr(ptr) {}

        reference operator*() const { return *f_ptr; }
        Fact* operator->() { return f_ptr; }
        Iterator& operator++() {
          Fact fact = *f_ptr;
          do {
            // cout << "F_PTR: " << bit_cast<uint64_t>(f_ptr) << endl;
            // cout << "LENGTH: " << f_ptr->length << endl;
            f_ptr += 1+f_ptr->length;
          
            fact = *f_ptr;
          }
          while (fact.t_id == T_ID_EMPTY_BLOCK);
            //&& bit_cast<uint64_t>(f_ptr) < this->end());
          return *this;
        };
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; };

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.f_ptr == b.f_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.f_ptr != b.f_ptr; };  
    

      private:
        Fact* f_ptr;

    };

    Iterator begin() { return Iterator(data+1); }
    Iterator end()   { return Iterator(head); }
};


Fact* alloc_fact_block(State* state, size_t size, bool recycle_blocks=true){
  cout << "ALLOC SIZE:" << size << " RECYLE: " << recycle_blocks <<
   " HEAD: " << (state->head-state->data) << " STATE_SIZE: " << state->size << "\n";

  

  // cout << "ALLOC: " << state->first_empty_f_id << endl;
  // cout << "FIRST EMPTY: " << state->first_empty_f_id << endl;

  // /* Look for a free block in the freed blocks */
  if(recycle_blocks && state->first_empty_block != NULL){
    // list<BlockRange> free_blocks = state->free_blocks;
    // list<BlockRange>::iterator itr;
    // EmptyBlock* first_free_block = (EmptyBlock*) &state->data[state->first_empty_f_id];
    EmptyBlock* first_empty_block = state->first_empty_block;
    EmptyBlock* free_block = first_empty_block;

    
    // EmptyBlock* free_block = first_free_block;
    // for (itr=free_blocks.begin(); itr!=free_blocks.end(); ++itr){
    while (free_block != NULL){
      // If block is same size then remove the free block
      if(free_block->length == size){
        if(free_block->prev != NULL) free_block->prev->next = free_block->next;          
        if(free_block->next != NULL) free_block->next->prev = free_block->prev;

        // Set next first_empty_block
        if(free_block == first_empty_block){
          if(first_empty_block->next != NULL){
            state->first_empty_block = first_empty_block->next;
          }else{
            state->first_empty_block = NULL;
          }
        }
        break;

      // Otherwise replace with a reduced block
      }else if(free_block->length > size){
        // BlockRange new_block;
        EmptyBlock* new_block = free_block+size;
        new_block->length = free_block->length-size;
        new_block->t_id = T_ID_EMPTY_BLOCK;
        // free_block->prev = new_block;
        new_block->prev = free_block->prev;
        new_block->next = free_block->next;

        // if(free_block->next != NULL) prev_block->next->prev = prev_block->prev;
        // if(free_block->prev != NULL) prev_block->prev->next = prev_block->next;

        cout << "NEW BLOCK: @" << ((Fact*)new_block)-state->data << " L: " << new_block->length << endl;
        if(free_block->prev != NULL) free_block->prev->next = new_block;
        if(free_block->next != NULL) free_block->next->prev = new_block;
        free_block->prev = NULL;
        free_block->next = NULL;

        if(free_block == first_empty_block){
          state->first_empty_block = new_block;//((Fact*)new_block)-state->data;  
        }

        break;
      }
      cout << "LOOP:" << ((Fact*)free_block)-state->data << endl;

      // If for some reason a corrupted leads to no progress
      //  fail gracefully by exiting the loop allocating from head instead. 
      if(free_block->next == free_block){
        free_block = NULL;
        break;
        // throw runtime_error("Free block has self reference.");
      }
      free_block = free_block->next;
      
    }

    cout << "NEW first_empty_f_id:" << ((Fact*) state->first_empty_block)-state->data << endl;

    if(free_block != NULL){
      free_block->length = size;
      cout << "EMIT BLOCK: @" << ((Fact*)free_block)-state->data << " L: " << free_block->length << endl;
      return (Fact *) free_block;  
    }
  }

  // /* Look for a free block at head */
  Fact* head;
  if(state->size-(state->head-state->data) >= size){
    cout << "ALLOC FROM HEAD @" << (state->head-state->data) << endl;    
    head = state->head;
    state->head += size;
    return head;
  }
  
  // If haven't found block then double the size 
  size_t grow = max(state->size, size);
  size_t realloc_size = (state->size+grow);
  
  // cout << "state data: " << state->data << "\n";
  // cout << "state size: " << state->size << "\n";
  // cout << "state head: " << state->head << "\n";
  // cout << "grow: " << grow << "\n";
  // cout << "Realloc Size: " << realloc_size << "\n";
  size_t head_diff = state->head-state->data;

  Fact* new_ptr = (Fact*) realloc(state->data, realloc_size * sizeof(Block));
  if(new_ptr == NULL){
    throw bad_alloc();
  }
  state->data = new_ptr;
  state->size = realloc_size;
  Fact* new_fact_ptr = state->data + head_diff;
  state->head = new_fact_ptr + size;

  cout << "END:" << size << " "<< state->head << "\n";
  return new_fact_ptr;
}

Fact* empty_fact(State* state, uint32_t length){
  // Fact allocation size is 1 Block for the header +N Tokens
  Fact* new_fact = alloc_fact_block(state, 1+length);
  // cout << "BLOOP" << new_fact << endl;
  new_fact->t_id = (uint16_t) T_ID_FACT;
  new_fact->f_id = (uint32_t) (new_fact-state->data);
  new_fact->length = length;
  // cout << "LEN" << endl;
  return new_fact;
}

Fact* empty_fact(uint32_t length){
  State* state = new State(length);
  return empty_fact(state, length);
}


bool contains_fact(State* state, Fact* fact){
  uint64_t fact_ptr = bit_cast<uint64_t>(fact);
  uint64_t data_start = bit_cast<uint64_t>(state->data);
  uint64_t data_end = bit_cast<uint64_t>(state->head); 
  if(fact_ptr >= data_start && fact_ptr < data_end){
    return true;
  }
  return false;
}

Fact* declare(State* state, Fact* fact){
    cout << "declare" << endl;
    if(contains_fact(state, fact)){
      return fact;
    }

    Fact* new_fact = empty_fact(state, fact->length);
    new_fact->f_id = (uint32_t) (new_fact-state->data);
    memcpy(new_fact, fact, sizeof(Block) * (fact->length+1));

    return new_fact;
}

bool retract(State* state, Fact* _fact){
    if(!contains_fact(state, _fact)){
      return false;
    }
    cout << "RETRACT" << endl;

    EmptyBlock* fact = (EmptyBlock*) _fact; 
    EmptyBlock* lead_block = (EmptyBlock*) fact; 

    uint32_t fact_length = fact->length;

    // Clear the fact's block
    fact->next = NULL;
    fact->prev = NULL;
    fact->t_id = 0;
    fact->is_lead = 0;
    fact->length = 0;

    // cout << endl<< "Start F_ID: " << _fact->f_id << " POS: " << _fact-state->data << " LEN: " << _fact->length << endl; 
    if((Fact*)_fact-state->data > 1){ // If not first fact

      // If the prev token is part of an empty block 
      //  then make its header EmptyBlock* the leading one;
      EmptyBlock* prev_block = (EmptyBlock*) fact-1;
      if(prev_block->t_id == T_ID_EMPTY_BLOCK){
        lead_block = prev_block-prev_block->length; 
        lead_block->length += 1+fact_length;
        // fact->length = fact-lead_block;
        cout << "PREV BLOCK EMPTY @f_id:"<< ((Fact*) lead_block)-state->data << " length: "<< lead_block->length << endl;
        cout << "NXT: " << prev_block->next << " PREV: " << prev_block->prev << endl;
        // Remove lead_block from the linked list 
        //  It will be reinserted to the front later
        if(lead_block->next != NULL) lead_block->next->prev = lead_block->prev;
        if(lead_block->prev != NULL) lead_block->prev->next = lead_block->next;
        lead_block->prev = NULL;
        lead_block->next = NULL;

      }else{
        lead_block->length = fact_length;
      }
    }
    // cout << "B" << endl;

    // If the next block is an empty block then merge it with
    // the lead block are remove it from the linked list of empties
    EmptyBlock* next_block = fact+1+fact_length;
    if(state->head-(Fact*)next_block > 0 && next_block->t_id == T_ID_EMPTY_BLOCK){
      // cout << "NEXT BLOCK EMPTY" << endl;
      lead_block->length += 1+next_block->length;

      // Remove next_block from the linked list 
      if(next_block->next != NULL) next_block->next->prev = next_block->prev;
      if(next_block->prev != NULL) next_block->prev->next = next_block->next;
      next_block->prev = NULL;
      next_block->next = NULL;
    }

    // cout << "C" << endl;

    // Mark the leading block and its last as empty
    //  set .length on the last token to be the leading block's length;
    //  this is used to extend empty blocks backwards.
    EmptyBlock* last_tok = (EmptyBlock*) lead_block+lead_block->length;
    // cout << "LAST TOK @:"<< ((Fact*) last_tok)-state->data << endl;
    lead_block->t_id = T_ID_EMPTY_BLOCK;
    last_tok->t_id = T_ID_EMPTY_BLOCK;
    lead_block->is_lead = true;
    last_tok->is_lead = false;
    last_tok->length = lead_block->length;  

    // If the lead block is not a block preceeding the removed fact
    //  then we need to add it into the linked list;
      
    int64_t lead_block_f_id = ((Fact*)lead_block)-state->data;

    // Insert the lead_block into front of the linked list 
    
    if(state->first_empty_block != NULL){
      state->first_empty_block->prev = lead_block;
      lead_block->next = state->first_empty_block;
      lead_block->prev = NULL;  
    }else{
      lead_block->prev = NULL;  
    }    
    state->first_empty_block = lead_block;
    

    // if(state->first_empty_f_id == -1 ||
    //    lead_block_f_id < state->first_empty_f_id){
    //   state->first_empty_f_id = lead_block_f_id;
    //   // cout << "SET FIRST EMPTY:" << state->first_empty_f_id << endl;
    // }


    // cout << "D" << endl;
    
    // for(int i=0; i < fact->length; i++){
    //   tok.t_id = T_ID_EMPTY_BLOCK
    //   tok.length = tok-lead_block;
    //   tok.is_internal = true;
    //   tok++;
    // }

    return true;
}

bool retract(State* state, uint32_t f_id){
  Fact* fact = &state->data[f_id];
  return retract(state, fact);
}

Fact* get(State* state, uint32_t f_id){
  return &state->data[f_id];
}

State* get_state(Fact* fact){
  ObjToken* state_token = (ObjToken*) fact-fact->f_id;
  return (State*) state_token->data;
}


#endif /* _State_H_ */


