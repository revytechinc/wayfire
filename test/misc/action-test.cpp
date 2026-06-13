#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/action/action.hpp>
#include <wayfire/action/action_interface.hpp>
#include <wayfire/variant.hpp>
#include <string>
#include <vector>

using wf::variant_t;
using wf::action_t;
using wf::action_interface_t;
using wf::to_string;
using wf::is_string;
using wf::get_string;

// Mock action interface for testing
class mock_action_interface : public action_interface_t
{
  public:
    std::string last_name;
    std::vector<variant_t> last_args;
    bool should_error = false;

    bool execute(const std::string &name, const std::vector<variant_t> &args) override
    {
        last_name = name;
        last_args = args;
        return should_error;
    }
};

TEST_CASE("action_t construction")
{
    std::vector<variant_t> args = {variant_t(1), variant_t(std::string("test"))};
    action_t action("command", args);

    // Construction should not throw
}

TEST_CASE("action_t to_string")
{
    SUBCASE("action with no args")
    {
        action_t action("test", {});
        auto str = action.to_string();
        CHECK(str.find("test") != std::string::npos);
        CHECK(str.find("name:") != std::string::npos);
    }

    SUBCASE("action with int arg")
    {
        std::vector<variant_t> args = {variant_t(42)};
        action_t action("test", args);
        auto str = action.to_string();
        CHECK(str.find("test") != std::string::npos);
        CHECK(str.find("42") != std::string::npos);
    }

    SUBCASE("action with string arg")
    {
        std::vector<variant_t> args = {variant_t(std::string("hello"))};
        action_t action("test", args);
        auto str = action.to_string();
        CHECK(str.find("test") != std::string::npos);
        CHECK(str.find("hello") != std::string::npos);
    }

    SUBCASE("action with multiple args")
    {
        std::vector<variant_t> args = {
            variant_t(1),
            variant_t(std::string("two")),
            variant_t(3.14)
        };
        action_t action("multi", args);
        auto str = action.to_string();
        CHECK(str.find("multi") != std::string::npos);
    }
}

TEST_CASE("action_t execute")
{
    mock_action_interface interface;

    SUBCASE("execute passes name and args")
    {
        std::vector<variant_t> args = {variant_t(42), variant_t(std::string("test"))};
        action_t action("mycommand", args);

        bool result = action.execute(interface);
        CHECK_FALSE(result);
        CHECK(interface.last_name == "mycommand");
        CHECK(interface.last_args.size() == 2);
        CHECK(wf::get_int(interface.last_args[0]) == 42);
        CHECK(wf::get_string(interface.last_args[1]) == "test");
    }

    SUBCASE("execute with empty args")
    {
        action_t action("empty", {});
        bool result = action.execute(interface);
        CHECK_FALSE(result);
        CHECK(interface.last_name == "empty");
        CHECK(interface.last_args.empty());
    }

    SUBCASE("execute returns error from interface")
    {
        interface.should_error = true;
        std::vector<variant_t> args = {variant_t(1)};
        action_t action("error", args);

        bool result = action.execute(interface);
        CHECK(result); // Should return true indicating error
    }
}
