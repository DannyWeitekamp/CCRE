#include "types.h"
#include "token.h"
#include <bit>
#include <sstream>
#include <functional>

// Assuming these are defined elsewhere
extern std::tuple<void*, const char*> intern(std::string);

// extern const int T_ID_STR;
// extern const int T_ID_INT;
// extern const int T_ID_FLOAT;
// extern const int T_ID_BOOL;

Token str_to_token(std::string arg) {
    auto tup = intern(arg);
    const char* data = std::get<1>(tup);

    UnicodeToken tok;
    tok.data = data;
    tok.hash = std::hash<std::string>{}(arg);
    tok.kind = 1; // TODO
    tok.is_ascii = 1; // TODO
    tok.t_id = T_ID_STR;
    tok.length = arg.length();

    return std::bit_cast<Token>(tok);
}

Token int_to_token(int64_t arg) {
    Token tok;
    tok.val = std::bit_cast<uint64_t>(arg);
    tok.hash = std::bit_cast<uint64_t>(arg);
    tok.t_id = T_ID_INT;
    return tok;
}

Token float_to_token(double arg) {
    Token tok;
    tok.val = std::bit_cast<uint64_t>(arg);
    tok.hash = std::bit_cast<uint64_t>(arg);
    tok.t_id = T_ID_FLOAT;
    return tok;
}

Token to_token(char* arg) { return str_to_token(arg); }
Token to_token(const char* arg) { return str_to_token(arg); }
Token to_token(std::string arg) { return str_to_token(arg); }
Token to_token(int arg) { return int_to_token(arg); }
Token to_token(long arg) { return int_to_token(arg); }
Token to_token(unsigned arg) { return int_to_token(arg); }
Token to_token(double arg) { return float_to_token(arg); }
Token to_token(float arg) { return float_to_token(arg); }
Token to_token(Token arg) { return arg; }

bool token_get_bool(Token tok) {
    return (bool) tok.val;
}

int64_t token_get_int(Token tok) {
    return std::bit_cast<int64_t>(tok.val);
}

double token_get_float(Token tok) {
    return std::bit_cast<double>(tok.val);
}

std::string_view token_get_string(Token tok) {
    UnicodeToken ut = std::bit_cast<UnicodeToken>(tok);
    return std::string_view(ut.data, ut.length);
}

std::string repr_token(Token& tok) {
    uint16_t t_id = tok.t_id;
    std::stringstream ss;
    switch(t_id) {
        case T_ID_BOOL:
            ss << std::boolalpha << token_get_bool(tok);
            break;
        case T_ID_INT:
            ss << token_get_int(tok);
            break;
        case T_ID_FLOAT:
            ss << std::to_string(token_get_float(tok));
            break;
        case T_ID_STR:
            ss << "'" << token_get_string(tok) << "'";
            break;
        default:
            ss << "<token t_id=" << t_id << " @" << std::bit_cast<uint64_t>(&tok) << ">";     
    }  
    return ss.str();
}

std::string to_string(Token& tok) {
    return repr_token(tok);
}