//  Copyright (c) 2001-2011 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_position_token.hpp>

#include <boost/core/lightweight_test.hpp>
#include <boost/phoenix/object.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl/container.hpp>

namespace lex = boost::spirit::lex;
namespace phoenix = boost::phoenix;
namespace mpl = boost::mpl;

///////////////////////////////////////////////////////////////////////////////
enum tokenids
{
    ID_INT = 1000,
    ID_DOUBLE
};

template <typename Lexer>
struct token_definitions : lex::lexer<Lexer>
{
    token_definitions()
    {
        this->self.add_pattern("HEXDIGIT", "[0-9a-fA-F]");
        this->self.add_pattern("OCTALDIGIT", "[0-7]");
        this->self.add_pattern("DIGIT", "[0-9]");

        this->self.add_pattern("OPTSIGN", "[-+]?");
        this->self.add_pattern("EXPSTART", "[eE][-+]");
        this->self.add_pattern("EXPONENT", "[eE]{OPTSIGN}{DIGIT}+");

        // define tokens and associate them with the lexer
        int_ = "(0x|0X){HEXDIGIT}+|0{OCTALDIGIT}*|{OPTSIGN}[1-9]{DIGIT}*";
        int_.id(ID_INT);

        double_ = "{OPTSIGN}({DIGIT}*\\.{DIGIT}+|{DIGIT}+\\.){EXPONENT}?|{DIGIT}+{EXPONENT}";
        double_.id(ID_DOUBLE);

        whitespace = "[ \t\n]+";

        this->self = 
                double_ 
            |   int_ 
            |   whitespace[ lex::_pass = lex::pass_flags::pass_ignore ]
            ;
    }

    lex::token_def<lex::omit> int_;
    lex::token_def<lex::omit> double_;
    lex::token_def<lex::omit> whitespace;
};

template <typename Lexer>
struct token_definitions_with_state : lex::lexer<Lexer>
{
    token_definitions_with_state()
    {
        this->self.add_pattern("HEXDIGIT", "[0-9a-fA-F]");
        this->self.add_pattern("OCTALDIGIT", "[0-7]");
        this->self.add_pattern("DIGIT", "[0-9]");

        this->self.add_pattern("OPTSIGN", "[-+]?");
        this->self.add_pattern("EXPSTART", "[eE][-+]");
        this->self.add_pattern("EXPONENT", "[eE]{OPTSIGN}{DIGIT}+");

        this->self.add_state();
        this->self.add_state("INT");
        this->self.add_state("DOUBLE");

        // define tokens and associate them with the lexer
        int_ = "(0x|0X){HEXDIGIT}+|0{OCTALDIGIT}*|{OPTSIGN}[1-9]{DIGIT}*";
        int_.id(ID_INT);

        double_ = "{OPTSIGN}({DIGIT}*\\.{DIGIT}+|{DIGIT}+\\.){EXPONENT}?|{DIGIT}+{EXPONENT}";
        double_.id(ID_DOUBLE);

        whitespace = "[ \t\n]+";

        this->self("*") = 
                double_ [ lex::_state = "DOUBLE"] 
            |   int_ [ lex::_state = "INT" ]
            |   whitespace[ lex::_pass = lex::pass_flags::pass_ignore ]
            ;
    }

    lex::token_def<lex::omit> int_;
    lex::token_def<lex::omit> double_;
    lex::token_def<lex::omit> whitespace;
};

///////////////////////////////////////////////////////////////////////////////
template <typename Token>
inline bool 
test_token_ids(int const* ids, std::vector<Token> const& tokens)
{
    for (std::size_t i = 0, len = tokens.size(); i < len; ++i)
    {
        if (*ids == -1)
            return false;           // reached end of expected data

        if (tokens[i].id() != static_cast<std::size_t>(*ids))        // token id must match
            return false;
        ++ids;
    }

    return (*ids == -1) ? true : false;
}

template <typename Token>
inline bool 
test_token_states(std::size_t const* states, std::vector<Token> const& tokens)
{
    for (std::size_t i = 0, len = tokens.size(); i < len; ++i)
    {
        if (*states == std::size_t(-1))
            return false;           // reached end of expected data

        if (tokens[i].state() != *states)            // token state must match
            return false;
        ++states;
    }

    return (*states == std::size_t(-1)) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
struct position_type
{
    std::size_t begin, end;
};

template <typename Iterator, typename Token>
inline bool 
test_token_positions(Iterator begin, position_type const* positions, 
    std::vector<Token> const& tokens)
{
    for (std::size_t i = 0, len = tokens.size(); i < len; ++i)
    {
        if (positions->begin == std::size_t(-1) && 
            positions->end == std::size_t(-1))
        {
            return false;           // reached end of expected data
        }

        boost::iterator_range<Iterator> matched = tokens[i].matched();
        std::size_t start = std::distance(begin, matched.begin());
        std::size_t end = std::distance(begin, matched.end());

        // position must match
        if (start != positions->begin || end != positions->end)
            return false;

        ++positions;
    }

    return (positions->begin == std::size_t(-1) && 
            positions->end == std::size_t(-1)) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
int main()
{
    typedef std::string::iterator base_iterator_type;
    std::string input(" 01 1.2 -2 0x3 2.3e6 -3.4");
    int ids[] = { ID_INT, ID_DOUBLE, ID_INT, ID_INT, ID_DOUBLE, ID_DOUBLE, -1 };
    std::size_t states[] = { 0, 1, 2, 1, 1, 2, std::size_t(-1) };
    position_type positions[] = 
    {
        { 1, 3 }, { 4, 7 }, { 8, 10 }, { 11, 14 }, { 15, 20 }, { 21, 25 }, 
        { std::size_t(-1), std::size_t(-1) }
    };

    // token type: token id, iterator_pair as token value, no state
    {
        typedef lex::lexertl::token<
            base_iterator_type, mpl::vector<>, mpl::false_> token_type;
        typedef lex::lexertl::actor_lexer<token_type> lexer_type;

        token_definitions<lexer_type> lexer;
        std::vector<token_type> tokens;
        base_iterator_type first = input.begin();

        using phoenix::arg_names::_1;
        BOOST_TEST(lex::tokenize(first, input.end(), lexer
          , phoenix::push_back(phoenix::ref(tokens), _1)));

        BOOST_TEST(test_token_ids(ids, tokens));
    }

    {
        typedef lex::lexertl::position_token<
            base_iterator_type, mpl::vector<>, mpl::false_> token_type;
        typedef lex::lexertl::actor_lexer<token_type> lexer_type;

        token_definitions<lexer_type> lexer;
        std::vector<token_type> tokens;
        base_iterator_type first = input.begin();

        using phoenix::arg_names::_1;
        BOOST_TEST(lex::tokenize(first, input.end(), lexer
          , phoenix::push_back(phoenix::ref(tokens), _1)));

        BOOST_TEST(test_token_ids(ids, tokens));
        BOOST_TEST(test_token_positions(input.begin(), positions, tokens));
    }

    // token type: holds token id, state, iterator_pair as token value
    {
        typedef lex::lexertl::token<
            base_iterator_type, mpl::vector<>, mpl::true_> token_type;
        typedef lex::lexertl::actor_lexer<token_type> lexer_type;

        token_definitions_with_state<lexer_type> lexer;
        std::vector<token_type> tokens;
        base_iterator_type first = input.begin();

        using phoenix::arg_names::_1;
        BOOST_TEST(lex::tokenize(first, input.end(), lexer
          , phoenix::push_back(phoenix::ref(tokens), _1)));

        BOOST_TEST(test_token_ids(ids, tokens));
        BOOST_TEST(test_token_states(states, tokens));
    }

    {
        typedef lex::lexertl::position_token<
            base_iterator_type, mpl::vector<>, mpl::true_> token_type;
        typedef lex::lexertl::actor_lexer<token_type> lexer_type;

        token_definitions_with_state<lexer_type> lexer;
        std::vector<token_type> tokens;
        base_iterator_type first = input.begin();

        using phoenix::arg_names::_1;
        BOOST_TEST(lex::tokenize(first, input.end(), lexer
          , phoenix::push_back(phoenix::ref(tokens), _1)));

        BOOST_TEST(test_token_ids(ids, tokens));
        BOOST_TEST(test_token_states(states, tokens));
        BOOST_TEST(test_token_positions(input.begin(), positions, tokens));
    }

    return boost::report_errors();
}
