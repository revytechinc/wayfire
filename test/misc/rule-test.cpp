#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/rule/rule.hpp>
#include <wayfire/action/action.hpp>
#include <wayfire/action/action_interface.hpp>
#include <wayfire/condition/condition.hpp>
#include <wayfire/condition/access_interface.hpp>
#include <wayfire/condition/logic_condition.hpp>
#include <wayfire/condition/test_condition.hpp>
#include <wayfire/variant.hpp>
#include <string>
#include <vector>
#include <memory>

using wf::variant_t;
using wf::rule_t;
using wf::action_t;
using wf::action_interface_t;
using wf::condition_t;
using wf::access_interface_t;
using wf::true_condition_t;
using wf::false_condition_t;
using wf::or_condition_t;

// Mock interfaces for testing
class mock_access_interface : public access_interface_t
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

class mock_action_interface : public action_interface_t
{
  public:
    std::string last_name;
    std::vector<variant_t> last_args;
    int execute_count = 0;

    bool execute(const std::string &name, const std::vector<variant_t> &args) override
    {
        last_name = name;
        last_args = args;
        execute_count++;
        return false;
    }
};

TEST_CASE("rule_t construction")
{
    auto condition = std::make_shared<true_condition_t>();
    auto if_action = std::make_shared<action_t>("test", std::vector<variant_t>{});
    auto else_action = std::make_shared<action_t>("else", std::vector<variant_t>{});

    rule_t rule("signal", condition, if_action, else_action);

    // Construction should not throw
}

TEST_CASE("rule_t to_string")
{
    SUBCASE("full rule")
    {
        auto condition = std::make_shared<true_condition_t>();
        auto if_action = std::make_shared<action_t>("if_cmd", std::vector<variant_t>{});
        auto else_action = std::make_shared<action_t>("else_cmd", std::vector<variant_t>{});

        rule_t rule("my_signal", condition, if_action, else_action);
        auto str = rule.to_string();

        CHECK(str.find("my_signal") != std::string::npos);
        CHECK(str.find("signal:") != std::string::npos);
    }

    SUBCASE("rule without else")
    {
        auto condition = std::make_shared<true_condition_t>();
        auto if_action = std::make_shared<action_t>("cmd", std::vector<variant_t>{});

        rule_t rule("sig", condition, if_action, nullptr);
        auto str = rule.to_string();

        CHECK(str.find("sig") != std::string::npos);
        CHECK(str.find("else_action:") != std::string::npos);
    }
}

TEST_CASE("rule_t apply")
{
    mock_access_interface access;
    mock_action_interface action;

    SUBCASE("condition true - if_action executed")
    {
        auto condition = std::make_shared<true_condition_t>();
        auto if_action = std::make_shared<action_t>("if_cmd", std::vector<variant_t>{});
        auto else_action = std::make_shared<action_t>("else_cmd", std::vector<variant_t>{});

        rule_t rule("test_signal", condition, if_action, else_action);

        bool error = rule.apply("test_signal", access, action);

        CHECK_FALSE(error);
        CHECK(action.last_name == "if_cmd");
        CHECK(action.execute_count == 1);
    }

    SUBCASE("condition false - else_action executed")
    {
        auto condition = std::make_shared<false_condition_t>();
        auto if_action = std::make_shared<action_t>("if_cmd", std::vector<variant_t>{});
        auto else_action = std::make_shared<action_t>("else_cmd", std::vector<variant_t>{});

        rule_t rule("test_signal", condition, if_action, else_action);

        bool error = rule.apply("test_signal", access, action);

        CHECK_FALSE(error);
        CHECK(action.last_name == "else_cmd");
        CHECK(action.execute_count == 1);
    }

    SUBCASE("signal mismatch - no action executed")
    {
        auto condition = std::make_shared<true_condition_t>();
        auto if_action = std::make_shared<action_t>("if_cmd", std::vector<variant_t>{});

        rule_t rule("my_signal", condition, if_action, nullptr);

        bool error = rule.apply("other_signal", access, action);

        CHECK_FALSE(error);
        CHECK(action.execute_count == 0);
    }

    SUBCASE("empty signal - returns error")
    {
        auto condition = std::make_shared<true_condition_t>();
        auto if_action = std::make_shared<action_t>("cmd", std::vector<variant_t>{});

        rule_t rule("sig", condition, if_action, nullptr);

        bool error = rule.apply("", access, action);

        CHECK(error); // Returns error for empty signal
        CHECK(action.execute_count == 0);
    }

    SUBCASE("null condition - returns error")
    {
        auto if_action = std::make_shared<action_t>("cmd", std::vector<variant_t>{});

        rule_t rule("sig", nullptr, if_action, nullptr);

        bool error = rule.apply("sig", access, action);

        CHECK(error);
        CHECK(action.execute_count == 0);
    }

    SUBCASE("null if_action - returns error")
    {
        auto condition = std::make_shared<true_condition_t>();

        rule_t rule("sig", condition, nullptr, nullptr);

        bool error = rule.apply("sig", access, action);

        CHECK(error);
    }

    SUBCASE("condition false with no else_action")
    {
        auto condition = std::make_shared<false_condition_t>();
        auto if_action = std::make_shared<action_t>("if_cmd", std::vector<variant_t>{});

        rule_t rule("sig", condition, if_action, nullptr);

        bool error = rule.apply("sig", access, action);

        CHECK_FALSE(error);
        CHECK(action.execute_count == 0); // else is nullptr, so nothing executed
    }
}
