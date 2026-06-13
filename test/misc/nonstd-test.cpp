#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/nonstd/observer_ptr.h>
#include <wayfire/nonstd/reverse.hpp>
#include <wayfire/utils.hpp>

#include <vector>
#include <memory>
#include <array>
#include <set>

TEST_CASE("observer_ptr default construction")
{
    nonstd::observer_ptr<int> p;
    REQUIRE(p.get() == nullptr);
    REQUIRE_FALSE(p);
}

TEST_CASE("observer_ptr from raw pointer")
{
    int value = 42;
    nonstd::observer_ptr<int> p(&value);
    REQUIRE(p.get() == &value);
    REQUIRE(*p == 42);
}

TEST_CASE("observer_ptr from nullptr")
{
    nonstd::observer_ptr<int> p(nullptr);
    REQUIRE(p.get() == nullptr);
    REQUIRE_FALSE(p);
}

TEST_CASE("observer_ptr from shared_ptr")
{
    auto sp = std::make_shared<int>(123);
    nonstd::observer_ptr<int> p(sp);
    REQUIRE(p.get() == sp.get());
    REQUIRE(*p == 123);
}

TEST_CASE("observer_ptr from unique_ptr")
{
    auto up = std::make_unique<int>(456);
    nonstd::observer_ptr<int> p(up);
    REQUIRE(p.get() == up.get());
    REQUIRE(*p == 456);
}

TEST_CASE("observer_ptr reset")
{
    int value = 42;
    nonstd::observer_ptr<int> p;
    REQUIRE(p.get() == nullptr);
    p.reset(&value);
    REQUIRE(p.get() == &value);
}

TEST_CASE("observer_ptr release")
{
    int value = 42;
    nonstd::observer_ptr<int> p(&value);
    int* released = p.release();
    REQUIRE(released == &value);
    REQUIRE(p.get() == nullptr);
}

TEST_CASE("observer_ptr swap")
{
    int a = 1, b = 2;
    nonstd::observer_ptr<int> p1(&a), p2(&b);
    p1.swap(p2);
    REQUIRE(p1.get() == &b);
    REQUIRE(p2.get() == &a);
}

TEST_CASE("observer_ptr comparison with nullptr")
{
    int value = 42;
    nonstd::observer_ptr<int> p1;
    nonstd::observer_ptr<int> p2(&value);

    REQUIRE(p1 == nullptr);
    REQUIRE(nullptr == p1);
    REQUIRE_FALSE(p2 == nullptr);
    REQUIRE_FALSE(nullptr == p2);

    REQUIRE(p1 != p2);
}

TEST_CASE("observer_ptr make_observer")
{
    int value = 99;
    auto p = nonstd::make_observer(&value);
    REQUIRE(p.get() == &value);
    REQUIRE(*p == 99);
}

TEST_CASE("observer_ptr arrow operator")
{
    struct S { int x; } s{42};
    nonstd::observer_ptr<S> p(&s);
    REQUIRE(p->x == 42);
}

TEST_CASE("observer_ptr explicit bool conversion")
{
    int value = 42;
    nonstd::observer_ptr<int> p_empty;
    nonstd::observer_ptr<int> p_valid(&value);

    if (p_empty)
    {
        REQUIRE(false); // should not reach here
    }

    if (p_valid)
    {
        // should reach here
    } else
    {
        REQUIRE(false);
    }
}

TEST_CASE("observer_ptr equality")
{
    int value = 42;
    nonstd::observer_ptr<int> p1(&value);
    nonstd::observer_ptr<int> p2(&value);
    nonstd::observer_ptr<int> p3;

    REQUIRE(p1 == p2);
    REQUIRE_FALSE(p1 != p2);
    REQUIRE(p1 != p3);
}

TEST_CASE("observer_ptr ordering")
{
    // Allocate two values - their addresses might not be predictable
    auto sp1 = std::make_shared<int>(1);
    auto sp2 = std::make_shared<int>(2);
    nonstd::observer_ptr<int> p1(sp1.get()), p2(sp2.get());

    // Verify ordering operators compile and work
    // We just verify they return consistent results
    bool less = p1 < p2;
    bool greater = p1 > p2;

    // Exactly one of these should be true (unless they're equal)
    if (p1.get() != p2.get())
    {
        REQUIRE(less != greater); // XOR - one must be true
    }
}

// Test reverse iteration
TEST_CASE("reverse iteration basic")
{
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> result;

    for (int x : wf::reverse(v))
    {
        result.push_back(x);
    }

    REQUIRE(result.size() == 5);
    REQUIRE(result[0] == 5);
    REQUIRE(result[1] == 4);
    REQUIRE(result[2] == 3);
    REQUIRE(result[3] == 2);
    REQUIRE(result[4] == 1);
}

TEST_CASE("reverse iteration empty container")
{
    std::vector<int> v;
    int count = 0;

    for (int x : wf::reverse(v))
    {
        (void)x;
        count++;
    }

    REQUIRE(count == 0);
}

TEST_CASE("reverse iteration single element")
{
    std::vector<int> v = {42};
    int count = 0;
    int last = 0;

    for (int x : wf::reverse(v))
    {
        count++;
        last = x;
    }

    REQUIRE(count == 1);
    REQUIRE(last == 42);
}

TEST_CASE("reverse iteration with std::array")
{
    std::array<int, 3> arr = {1, 2, 3};
    std::vector<int> result;

    for (int x : wf::reverse(arr))
    {
        result.push_back(x);
    }

    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 3);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 1);
}

TEST_CASE("reverse with array")
{
    int arr[] = {10, 20, 30};
    std::vector<int> result;

    for (int x : wf::reverse(arr))
    {
        result.push_back(x);
    }

    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 30);
    REQUIRE(result[1] == 20);
    REQUIRE(result[2] == 10);
}

// Test utility functions
TEST_CASE("contains with vector")
{
    std::vector<int> v = {1, 2, 3, 4, 5};
    REQUIRE(wf::contains(v, 3));
    REQUIRE(wf::contains(v, 1));
    REQUIRE(wf::contains(v, 5));
    REQUIRE_FALSE(wf::contains(v, 0));
    REQUIRE_FALSE(wf::contains(v, 6));
}

TEST_CASE("contains with string")
{
    std::string s = "hello world";
    REQUIRE(wf::contains(s, 'h'));
    REQUIRE(wf::contains(s, 'o'));
    REQUIRE(wf::contains(s, 'l'));
    REQUIRE_FALSE(wf::contains(s, 'x'));
}

TEST_CASE("contains with set")
{
    std::set<std::string> s = {"apple", "banana", "cherry"};
    REQUIRE(wf::contains(s, "banana"));
    REQUIRE_FALSE(wf::contains(s, "orange"));
}

TEST_CASE("ltrim")
{
    std::string s1 = "   hello";
    wf::ltrim(s1);
    REQUIRE(s1 == "hello");

    std::string s2 = "\t\n  world";
    wf::ltrim(s2);
    REQUIRE(s2 == "world");

    std::string s3 = "no trim";
    wf::ltrim(s3);
    REQUIRE(s3 == "no trim");

    std::string s4 = "   ";
    wf::ltrim(s4);
    REQUIRE(s4.empty());
}

TEST_CASE("rtrim")
{
    std::string s1 = "hello   ";
    wf::rtrim(s1);
    REQUIRE(s1 == "hello");

    std::string s2 = "world\t\n  ";
    wf::rtrim(s2);
    REQUIRE(s2 == "world");

    std::string s3 = "no trim";
    wf::rtrim(s3);
    REQUIRE(s3 == "no trim");

    std::string s4 = "   ";
    wf::rtrim(s4);
    REQUIRE(s4.empty());
}

TEST_CASE("trim")
{
    std::string s1 = "   hello   ";
    wf::trim(s1);
    REQUIRE(s1 == "hello");

    std::string s2 = "\t\n  world\t\n  ";
    wf::trim(s2);
    REQUIRE(s2 == "world");

    std::string s3 = "no trim";
    wf::trim(s3);
    REQUIRE(s3 == "no trim");

    std::string s4 = "   ";
    wf::trim(s4);
    REQUIRE(s4.empty());

    std::string s5 = "";
    wf::trim(s5);
    REQUIRE(s5.empty());
}
