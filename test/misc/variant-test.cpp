#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/variant.hpp>

using wf::variant_t;
using wf::variant_type_t;

TEST_CASE("variant type checks")
{
    SUBCASE("int variant")
    {
        variant_t v = 42;
        REQUIRE(wf::is_int(v));
        REQUIRE_FALSE(wf::is_char(v));
        REQUIRE_FALSE(wf::is_bool(v));
        REQUIRE_FALSE(wf::is_float(v));
        REQUIRE_FALSE(wf::is_double(v));
        REQUIRE_FALSE(wf::is_string(v));
        REQUIRE(wf::type(v) == variant_type_t::INT);
    }

    SUBCASE("char variant")
    {
        variant_t v = 'x';
        REQUIRE(wf::is_char(v));
        REQUIRE_FALSE(wf::is_int(v));
    }

    SUBCASE("bool variant")
    {
        variant_t v = true;
        REQUIRE(wf::is_bool(v));
        REQUIRE_FALSE(wf::is_int(v));
    }

    SUBCASE("float variant")
    {
        variant_t v = 3.14f;
        REQUIRE(wf::is_float(v));
        REQUIRE_FALSE(wf::is_double(v));
        REQUIRE_FALSE(wf::is_int(v));
    }

    SUBCASE("double variant")
    {
        variant_t v = 3.14159265358979;
        REQUIRE(wf::is_double(v));
        REQUIRE_FALSE(wf::is_float(v)); // float and double are distinct types
        REQUIRE_FALSE(wf::is_int(v));
    }

    SUBCASE("string variant")
    {
        variant_t v = std::string("hello");
        REQUIRE(wf::is_string(v));
        REQUIRE_FALSE(wf::is_int(v));
    }
}

TEST_CASE("variant getters")
{
    SUBCASE("get int")
    {
        variant_t v = 42;
        REQUIRE(wf::get_int(v) == 42);
    }

    SUBCASE("get char")
    {
        variant_t v = 'Z';
        REQUIRE(wf::get_char(v) == 'Z');
    }

    SUBCASE("get bool")
    {
        variant_t v1 = true;
        variant_t v2 = false;
        REQUIRE(wf::get_bool(v1) == true);
        REQUIRE(wf::get_bool(v2) == false);
    }

    SUBCASE("get float")
    {
        variant_t v = 2.5f;
        REQUIRE(wf::get_float(v) == doctest::Approx(2.5f));
    }

    SUBCASE("get double")
    {
        variant_t v = 2.718281828;
        REQUIRE(wf::get_double(v) == doctest::Approx(2.718281828));
    }

    SUBCASE("get string")
    {
        variant_t v = std::string("test");
        REQUIRE(wf::get_string(v) == "test");
    }
}

TEST_CASE("variant to_string")
{
    SUBCASE("int to string")
    {
        variant_t v = 100;
        auto str = wf::to_string(v);
        REQUIRE(str.find("int") != std::string::npos);
        REQUIRE(str.find("100") != std::string::npos);
    }

    SUBCASE("char to string")
    {
        variant_t v = 'A';
        auto str = wf::to_string(v);
        REQUIRE(str.find("char") != std::string::npos);
        REQUIRE(str.find("65") != std::string::npos); // 'A' is ASCII 65
    }

    SUBCASE("bool true to string")
    {
        variant_t v = true;
        auto str = wf::to_string(v);
        REQUIRE(str.find("bool") != std::string::npos);
        REQUIRE(str.find("1") != std::string::npos);
    }

    SUBCASE("bool false to string")
    {
        variant_t v = false;
        auto str = wf::to_string(v);
        REQUIRE(str.find("bool") != std::string::npos);
        REQUIRE(str.find("0") != std::string::npos);
    }

    SUBCASE("float to string")
    {
        variant_t v = 1.5f;
        auto str = wf::to_string(v);
        REQUIRE(str.find("float") != std::string::npos);
        REQUIRE(str.find("1.5") != std::string::npos);
    }

    SUBCASE("double to string")
    {
        variant_t v = 2.718;
        auto str = wf::to_string(v);
        REQUIRE(str.find("double") != std::string::npos);
        REQUIRE(str.find("2.718") != std::string::npos);
    }

    SUBCASE("string to string")
    {
        variant_t v = std::string("hello world");
        auto str = wf::to_string(v);
        REQUIRE(str.find("string") != std::string::npos);
        REQUIRE(str.find("hello world") != std::string::npos);
    }
}

TEST_CASE("variant comparisons")
{
    SUBCASE("int equality")
    {
        variant_t a = 10;
        variant_t b = 10;
        variant_t c = 20;
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }

    SUBCASE("string equality")
    {
        variant_t a = std::string("hello");
        variant_t b = std::string("hello");
        variant_t c = std::string("world");
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }

    SUBCASE("bool equality")
    {
        variant_t a = true;
        variant_t b = true;
        variant_t c = false;
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }

    SUBCASE("mixed type inequality")
    {
        variant_t a = 42;
        variant_t b = std::string("42");
        REQUIRE_FALSE(a == b);
    }
}
