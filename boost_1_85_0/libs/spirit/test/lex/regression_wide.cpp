//  Copyright (c) 2001-2010 Hartmut Kaiser
//  Copyright (c) 2010 Sergey "GooRoo" Olendarenko
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <locale>
#include <string>

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/phoenix/function.hpp>
#include <boost/phoenix/operator.hpp>

#include <boost/core/lightweight_test.hpp>

namespace lex = boost::spirit::lex;
namespace phoenix = boost::phoenix;

typedef std::basic_string<wchar_t> wstring_type;

///////////////////////////////////////////////////////////////////////////////
enum tokenids 
{
    ID_IDENT = 1,
    ID_CONSTANT,
    ID_OPERATION,
    ID_BRACKET
};

///////////////////////////////////////////////////////////////////////////////
struct test_data
{
    tokenids     tokenid;
    wstring_type value;
};

// alpha+x1*(2.836-x2[i])
test_data data[] = 
{
    { ID_IDENT, L"alpha" },
    { ID_OPERATION, L"+" },
    { ID_IDENT, L"x1" },
    { ID_OPERATION, L"*" },
    { ID_BRACKET, L"(" },
    { ID_CONSTANT, L"2.836" },
    { ID_OPERATION, L"-" },
    { ID_IDENT, L"x2" },
    { ID_BRACKET, L"[" },
    { ID_IDENT, L"i" },
    { ID_BRACKET, L"]" },
    { ID_BRACKET, L")" }
};

///////////////////////////////////////////////////////////////////////////////
struct test_impl
{
    typedef void result_type;
    template <typename TokenId, typename Value>
    struct result { typedef void type; };

    template <typename TokenId, typename Value>
    void operator()(TokenId const& tokenid, Value const& val) const
    {
        BOOST_TEST(sequence_counter < sizeof(data)/sizeof(data[0]));
        BOOST_TEST(data[sequence_counter].tokenid == tokenids(tokenid));
        BOOST_TEST(0 == val.which());

        typedef boost::iterator_range<wstring_type::iterator> iterator_range;
        iterator_range r = boost::get<iterator_range>(val);
        BOOST_TEST(data[sequence_counter].value == 
            wstring_type(r.begin(), r.end()));

        ++sequence_counter;
    }

    static std::size_t sequence_counter;
};
std::size_t test_impl::sequence_counter = 0;

phoenix::function<test_impl> const test = test_impl();

///////////////////////////////////////////////////////////////////////////////
template <typename Lexer>
struct mega_tokens : lex::lexer<Lexer>
{
    mega_tokens()
        : identifier(L"[a-zA-Z_][a-zA-Z0-9_]*", ID_IDENT)
        , constant  (L"[0-9]+(\\.[0-9]+)?", ID_CONSTANT)
        , operation (L"[\\+\\-\\*/]", ID_OPERATION)
        , bracket   (L"[\\(\\)\\[\\]]", ID_BRACKET)
    {
        using lex::_tokenid;
        using lex::_val;

        this->self
            = operation  [ test(_tokenid, _val) ]
            | identifier [ test(_tokenid, _val) ]
            | constant   [ test(_tokenid, _val) ]
            | bracket    [ test(_tokenid, _val) ]
        ;
    }

    lex::token_def<wstring_type, wchar_t, tokenids> identifier;
    lex::token_def<double, wchar_t, tokenids> constant;
    lex::token_def<wchar_t, wchar_t, tokenids> operation;
    lex::token_def<wchar_t, wchar_t, tokenids> bracket;
};

///////////////////////////////////////////////////////////////////////////////
int main()
{
    typedef wstring_type::iterator base_iterator;
    typedef lex::lexertl::token<
        base_iterator, boost::mpl::vector<wchar_t, wstring_type, double>
      , boost::mpl::true_, tokenids
    > token_type;
    typedef lex::lexertl::actor_lexer<token_type> lexer_type;

    mega_tokens<lexer_type> mega_lexer;

    wstring_type exampleStr = L"alpha+x1*(2.836-x2[i])";
    base_iterator first = exampleStr.begin();

    BOOST_TEST(lex::tokenize(first, exampleStr.end(), mega_lexer));
    BOOST_TEST(test_impl::sequence_counter == sizeof(data)/sizeof(data[0]));

    return boost::report_errors();
}
