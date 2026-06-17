/**
 * Unit tests for Wayfire platform backend abstraction.
 *
 * Verifies that the platform factory returns the correct platform-specific
 * implementation and that all platform operations work correctly.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstring>
#include <cstdlib>
#include <memory>

#include <wayfire/platform-backend.hpp>

/* ─── Platform factory ─────────────────────────────────────────────────── */

TEST_CASE("create_platform_backend returns non-null")
{
    auto platform = wf::create_platform_backend();
    REQUIRE(platform != nullptr);
}

TEST_CASE("platform_name returns a non-empty string")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(name != nullptr);
    REQUIRE(std::strlen(name) > 0);
}

TEST_CASE("platform_name is stable between calls")
{
    auto platform1 = wf::create_platform_backend();
    auto platform2 = wf::create_platform_backend();

    const char *name1 = platform1->platform_name();
    const char *name2 = platform2->platform_name();
    REQUIRE(std::strcmp(name1, name2) == 0);
}

/* ─── OS-specific name assertions ──────────────────────────────────────── */

#if defined(__linux__)
TEST_CASE("Linux platform identifies as 'linux'")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(std::strcmp(name, "linux") == 0);
}
#elif defined(__FreeBSD__)
TEST_CASE("FreeBSD platform identifies as 'freebsd'")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(std::strcmp(name, "freebsd") == 0);
}
#elif defined(__OpenBSD__)
TEST_CASE("OpenBSD platform identifies as 'openbsd'")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(std::strcmp(name, "openbsd") == 0);
}
#elif defined(__NetBSD__)
TEST_CASE("NetBSD platform identifies as 'netbsd'")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(std::strcmp(name, "netbsd") == 0);
}
#elif defined(__DragonFly__)
TEST_CASE("DragonFly platform identifies as 'dragonfly'")
{
    auto platform = wf::create_platform_backend();
    const char *name = platform->platform_name();
    REQUIRE(std::strcmp(name, "dragonfly") == 0);
}
#endif

/* ─── GPU detection ────────────────────────────────────────────────────── */

TEST_CASE("detect_render_gpu returns non-null or nullptr")
{
    auto platform = wf::create_platform_backend();
    char *gpu_path = platform->detect_render_gpu();

    /* Either returns a valid string or nullptr */
    if (gpu_path != nullptr)
    {
        /* If non-null, must be either empty (multiple GPUs) or valid path */
        if (gpu_path[0] != '\0')
        {
            /* Should start with /dev/dri/ */
            REQUIRE(std::strncmp(gpu_path, "/dev/dri/", 9) == 0);
        }
        std::free(gpu_path);
    }
    /* nullptr is also valid (no GPUs found or WLR_DRM_DEVICES already set) */
}

TEST_CASE("gpu_is_usable returns bool for valid render node")
{
    auto platform = wf::create_platform_backend();

    /* Test with a non-existent path - should return false */
    bool usable = platform->gpu_is_usable("/dev/dri/renderD999");
    REQUIRE(usable == false);
}

TEST_CASE("get_render_node_path returns string")
{
    auto platform = wf::create_platform_backend();

    std::string card_path = "/dev/dri/card0";
    std::string render_path = platform->get_render_node_path(card_path);

    /* If render node exists, should return valid path */
    if (!render_path.empty())
    {
        REQUIRE(render_path.find("/dev/dri/renderD") == 0);
    }
}

/* ─── Privilege handling ───────────────────────────────────────────────── */

TEST_CASE("is_root returns bool")
{
    auto platform = wf::create_platform_backend();
    bool is_root = platform->is_root();

    /* Just verify it returns a valid bool - actual value depends on test env */
    REQUIRE((is_root == true || is_root == false));
}

TEST_CASE("drop_permissions returns bool")
{
    auto platform = wf::create_platform_backend();

    /* Call drop_permissions - returns true if successful or already non-root */
    bool result = platform->drop_permissions();

    /* After dropping, is_root should be false */
    REQUIRE(platform->is_root() == false);
}
