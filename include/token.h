#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>
#include <cstdint>
#include <string_view>

struct Token {
    uint64_t val;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];
};

struct UnicodeToken {
    const char* data;
    uint64_t hash;
    uint16_t t_id;
    uint8_t kind;
    uint8_t is_ascii;
    uint32_t length;
};

struct ObjToken {
    void* data;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];
};

struct EmptyBlock{
  int64_t prev_f_id;
  int64_t next_f_id;
  uint16_t t_id;
  uint16_t is_lead;
  uint32_t length;
};


Token str_to_token(std::string arg);
Token int_to_token(int64_t arg);
Token float_to_token(double arg);

Token to_token(char* arg);
Token to_token(const char* arg);
Token to_token(std::string arg);
Token to_token(int arg);
Token to_token(long arg);
Token to_token(unsigned arg);
Token to_token(double arg);
Token to_token(float arg);
Token to_token(Token arg);

bool token_get_bool(Token tok);
int64_t token_get_int(Token tok);
double token_get_float(Token tok);
std::string_view token_get_string(Token tok);

std::string repr_token(Token& tok);
std::string to_string(Token& tok);

#endif // _TOKEN_H_