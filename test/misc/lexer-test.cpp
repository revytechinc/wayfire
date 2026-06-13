#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/lexer/lexer.hpp>
#include <wayfire/lexer/symbol.hpp>
#include <string>

using wf::lexer_t;
using wf::symbol_t;
using wf::variant_t;
using wf::is_int;
using wf::is_char;
using wf::is_bool;
using wf::is_float;
using wf::is_double;
using wf::is_string;
using wf::get_int;
using wf::get_char;
using wf::get_bool;
using wf::get_double;
using wf::get_string;

TEST_CASE("lexer basic construction")
{
    lexer_t lexer1;
    CHECK(lexer1.text().empty());

    lexer_t lexer2("hello world");
    CHECK(lexer2.text() == "hello world");
}

TEST_CASE("lexer empty input")
{
    lexer_t lexer("");
    auto symbol = lexer.parse_symbol();
    CHECK(symbol.type == symbol_t::type_t::END);
}

TEST_CASE("lexer single identifier")
{
    lexer_t lexer("hello");
    auto symbol = lexer.parse_symbol();
    CHECK(symbol.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(symbol.value) == "hello");
}

TEST_CASE("lexer multiple identifiers")
{
    lexer_t lexer("hello world foo");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s1.value) == "hello");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s2.value) == "world");

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s3.value) == "foo");

    auto s4 = lexer.parse_symbol();
    CHECK(s4.type == symbol_t::type_t::END);
}

TEST_CASE("lexer identifiers with whitespace")
{
    lexer_t lexer("  alpha   beta");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s1.value) == "alpha");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s2.value) == "beta");

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::END);
}

TEST_CASE("lexer keywords")
{
    lexer_t lexer("is equals contains if else then on all none");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::KEYWORD);
    CHECK(get_string(s1.value) == "is");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::KEYWORD);
    CHECK(get_string(s2.value) == "equals");

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::KEYWORD);
    CHECK(get_string(s3.value) == "contains");
}

TEST_CASE("lexer signals")
{
    lexer_t lexer("created maximized minimized fullscreened");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::SIGNAL);
    CHECK(get_string(s1.value) == "created");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::SIGNAL);
    CHECK(get_string(s2.value) == "maximized");
}

TEST_CASE("lexer operators")
{
    lexer_t lexer("& | !");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::OPERATOR);
    CHECK(get_string(s1.value) == "&");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::OPERATOR);
    CHECK(get_string(s2.value) == "|");

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::OPERATOR);
    CHECK(get_string(s3.value) == "!");
}

TEST_CASE("lexer structural characters")
{
    lexer_t lexer("()");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::STRUCTURAL);
    CHECK(get_string(s1.value) == "(");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::STRUCTURAL);
    CHECK(get_string(s2.value) == ")");
}

TEST_CASE("lexer integer literals")
{
    lexer_t lexer("42 -17 0 999");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::LITERAL);
    CHECK(is_int(s1.value));
    CHECK(get_int(s1.value) == 42);

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::LITERAL);
    CHECK(is_int(s2.value));
    CHECK(get_int(s2.value) == -17);

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::LITERAL);
    CHECK(is_int(s3.value));
    CHECK(get_int(s3.value) == 0);
}

TEST_CASE("lexer string literals")
{
    lexer_t lexer("\"hello\" \"world\" \"\"");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::LITERAL);
    CHECK(is_string(s1.value));
    CHECK(get_string(s1.value) == "hello");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::LITERAL);
    CHECK(is_string(s2.value));
    CHECK(get_string(s2.value) == "world");

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::LITERAL);
    CHECK(is_string(s3.value));
    CHECK(get_string(s3.value) == "");
}

TEST_CASE("lexer boolean literals")
{
    lexer_t lexer("true false");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::LITERAL);
    CHECK(is_bool(s1.value));
    CHECK(get_bool(s1.value) == true);

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::LITERAL);
    CHECK(is_bool(s2.value));
    CHECK(get_bool(s2.value) == false);
}

TEST_CASE("lexer double literals")
{
    lexer_t lexer("3.14 0.5 -1.5");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::LITERAL);
    CHECK(is_double(s1.value));
    CHECK(get_double(s1.value) == doctest::Approx(3.14));

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::LITERAL);
    CHECK(is_double(s2.value));
}

TEST_CASE("lexer mixed input")
{
    lexer_t lexer("is \"window\" & maximized");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::KEYWORD);
    CHECK(get_string(s1.value) == "is");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::LITERAL);
    CHECK(is_string(s2.value));

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::OPERATOR);
    CHECK(get_string(s3.value) == "&");

    auto s4 = lexer.parse_symbol();
    CHECK(s4.type == symbol_t::type_t::SIGNAL);
    CHECK(get_string(s4.value) == "maximized");
}

TEST_CASE("lexer reset")
{
    lexer_t lexer("hello world");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::IDENTIFIER);

    lexer.reset();
    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s2.value) == "hello");
}

TEST_CASE("lexer reset with new text")
{
    lexer_t lexer("hello");
    auto s1 = lexer.parse_symbol();
    CHECK(get_string(s1.value) == "hello");

    lexer.reset("world");
    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(s2.value) == "world");
}

TEST_CASE("lexer reverse basic")
{
    lexer_t lexer("alpha beta gamma");

    // Parse all three tokens
    auto s1 = lexer.parse_symbol();
    CHECK(get_string(s1.value) == "alpha");

    auto s2 = lexer.parse_symbol();
    CHECK(get_string(s2.value) == "beta");

    auto s3 = lexer.parse_symbol();
    CHECK(get_string(s3.value) == "gamma");

    // After parsing all tokens, _parse_position is at end
    // Reverse once - should return gamma again
    lexer.reverse();
    auto s4 = lexer.parse_symbol();
    CHECK(get_string(s4.value) == "gamma");

    // Reverse again - parse_symbol returns gamma again since _reversed is reset after each parse
    lexer.reverse();
    auto s5 = lexer.parse_symbol();
    CHECK(get_string(s5.value) == "gamma");

    // Continue parsing - should be END since we're back at the end of history
    auto s6 = lexer.parse_symbol();
    CHECK(s6.type == symbol_t::type_t::END);
}

TEST_CASE("lexer current symbol position")
{
    lexer_t lexer("  hello");
    auto symbol = lexer.parse_symbol();
    CHECK(symbol.type == symbol_t::type_t::IDENTIFIER);
    CHECK(lexer.current_symbol_position() == 2); // After the two spaces
}

TEST_CASE("lexer reverse at start")
{
    lexer_t lexer("test");
    lexer.reverse(); // Should have no effect
    lexer.reverse(); // Should still have no effect
    auto symbol = lexer.parse_symbol();
    CHECK(symbol.type == symbol_t::type_t::IDENTIFIER);
    CHECK(get_string(symbol.value) == "test");
}

TEST_CASE("lexer complex expression")
{
    lexer_t lexer("( is maximized & true )");
    auto s1 = lexer.parse_symbol();
    CHECK(s1.type == symbol_t::type_t::STRUCTURAL);
    CHECK(get_string(s1.value) == "(");

    auto s2 = lexer.parse_symbol();
    CHECK(s2.type == symbol_t::type_t::KEYWORD);

    auto s3 = lexer.parse_symbol();
    CHECK(s3.type == symbol_t::type_t::SIGNAL);

    auto s4 = lexer.parse_symbol();
    CHECK(s4.type == symbol_t::type_t::OPERATOR);

    auto s5 = lexer.parse_symbol();
    CHECK(s5.type == symbol_t::type_t::LITERAL);
    CHECK(is_bool(s5.value));

    auto s6 = lexer.parse_symbol();
    CHECK(s6.type == symbol_t::type_t::STRUCTURAL);
    CHECK(get_string(s6.value) == ")");
}
