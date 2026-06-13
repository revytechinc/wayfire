#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/condition/condition.hpp>
#include <wayfire/condition/access_interface.hpp>
#include <wayfire/condition/logic_condition.hpp>
#include <wayfire/condition/test_condition.hpp>
#include <wayfire/variant.hpp>
#include <string>

using wf::variant_t;
using wf::condition_t;
using wf::or_condition_t;
using wf::and_condition_t;
using wf::not_condition_t;
using wf::true_condition_t;
using wf::false_condition_t;
using wf::equals_condition_t;
using wf::contains_condition_t;
using wf::access_interface_t;
using wf::is_string;
using wf::get_string;

// Simple test access interface that returns fixed values
class test_access_interface : public access_interface_t
{
  public:
    std::map<std::string, variant_t> values;

    variant_t get(const std::string &identifier, bool &error) override
    {
        auto it = values.find(identifier);
        if (it == values.end())
        {
            error = true;
            return variant_t(0);
        }
        return it->second;
    }
};

TEST_CASE("true_condition_t")
{
    test_access_interface interface;
    bool error = false;

    true_condition_t condition;
    CHECK(condition.evaluate(interface, error));
    CHECK_FALSE(error);

    error = true;
    CHECK_FALSE(condition.evaluate(interface, error));
}

TEST_CASE("false_condition_t")
{
    test_access_interface interface;
    bool error = false;

    false_condition_t condition;
    CHECK_FALSE(condition.evaluate(interface, error));
    CHECK_FALSE(error);
}

TEST_CASE("equals_condition_t")
{
    test_access_interface interface;
    interface.values["name"] = variant_t(std::string("test"));

    bool error = false;

    SUBCASE("matching value")
    {
        equals_condition_t condition("name", variant_t(std::string("test")));
        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("non-matching value")
    {
        equals_condition_t condition("name", variant_t(std::string("other")));
        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("missing identifier")
    {
        equals_condition_t condition("missing", variant_t(std::string("test")));
        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }
}

TEST_CASE("contains_condition_t")
{
    test_access_interface interface;
    interface.values["name"] = variant_t(std::string("hello world"));

    bool error = false;

    SUBCASE("contains substring")
    {
        contains_condition_t condition("name", variant_t(std::string("world")));
        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("does not contain substring")
    {
        contains_condition_t condition("name", variant_t(std::string("foo")));
        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("missing identifier")
    {
        contains_condition_t condition("missing", variant_t(std::string("world")));
        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }

    SUBCASE("non-string value")
    {
        interface.values["number"] = variant_t(42);
        contains_condition_t condition("number", variant_t(std::string("42")));
        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }
}

TEST_CASE("or_condition_t")
{
    test_access_interface interface;
    interface.values["a"] = variant_t(true);
    interface.values["b"] = variant_t(false);

    bool error = false;

    SUBCASE("left true right false")
    {
        auto left = std::make_unique<true_condition_t>();
        auto right = std::make_unique<false_condition_t>();
        or_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("both false")
    {
        auto left = std::make_unique<false_condition_t>();
        auto right = std::make_unique<false_condition_t>();
        or_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("both true")
    {
        auto left = std::make_unique<true_condition_t>();
        auto right = std::make_unique<true_condition_t>();
        or_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("missing left")
    {
        auto right = std::make_unique<true_condition_t>();
        or_condition_t condition;
        condition.right = std::move(right);

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }
}

TEST_CASE("and_condition_t")
{
    test_access_interface interface;

    bool error = false;

    SUBCASE("both true")
    {
        auto left = std::make_unique<true_condition_t>();
        auto right = std::make_unique<true_condition_t>();
        and_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("left false right true")
    {
        auto left = std::make_unique<false_condition_t>();
        auto right = std::make_unique<true_condition_t>();
        and_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("missing right")
    {
        auto left = std::make_unique<true_condition_t>();
        and_condition_t condition;
        condition.left = std::move(left);

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }
}

TEST_CASE("not_condition_t")
{
    test_access_interface interface;

    bool error = false;

    SUBCASE("not true")
    {
        auto child = std::make_unique<true_condition_t>();
        not_condition_t condition;
        condition.child = std::move(child);

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("not false")
    {
        auto child = std::make_unique<false_condition_t>();
        not_condition_t condition;
        condition.child = std::move(child);

        CHECK(condition.evaluate(interface, error));
        CHECK_FALSE(error);
    }

    SUBCASE("missing child")
    {
        not_condition_t condition;

        CHECK_FALSE(condition.evaluate(interface, error));
        CHECK(error);
    }
}

TEST_CASE("condition to_string")
{
    SUBCASE("true_condition_t to_string")
    {
        true_condition_t condition;
        CHECK(condition.to_string() == "true");
    }

    SUBCASE("false_condition_t to_string")
    {
        false_condition_t condition;
        CHECK(condition.to_string() == "false");
    }

    SUBCASE("or_condition_t to_string")
    {
        auto left = std::make_unique<true_condition_t>();
        auto right = std::make_unique<false_condition_t>();
        or_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK(condition.to_string() == "(true | false)");
    }

    SUBCASE("and_condition_t to_string")
    {
        auto left = std::make_unique<true_condition_t>();
        auto right = std::make_unique<true_condition_t>();
        and_condition_t condition;
        condition.left = std::move(left);
        condition.right = std::move(right);

        CHECK(condition.to_string() == "(true & true)");
    }

    SUBCASE("not_condition_t to_string")
    {
        auto child = std::make_unique<true_condition_t>();
        not_condition_t condition;
        condition.child = std::move(child);

        CHECK(condition.to_string() == "(!true)");
    }

    SUBCASE("or_condition_t with nullptr")
    {
        or_condition_t condition;
        CHECK(condition.to_string() == "(nullptr | nullptr)");
    }

    SUBCASE("not_condition_t with nullptr")
    {
        not_condition_t condition;
        CHECK(condition.to_string() == "(!nullptr)");
    }
}
