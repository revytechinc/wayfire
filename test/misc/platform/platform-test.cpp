/**
 * Unit tests for wlroots OS platform abstraction (wlr_platform API).
 *
 * Verifies that the platform factory returns a valid descriptor with
 * non-null vtable functions for the current operating system.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstring>
#include <cstdlib>

extern "C" {
#include <wlr/util/platform.h>
#include <wlr/util/log.h>
}

/* ─── wlr_platform_name ─────────────────────────────────────────────────── */

TEST_CASE("wlr_platform_name returns a non-empty string")
{
    const char *name = wlr_platform_name();
    REQUIRE(name != nullptr);
    REQUIRE(std::strlen(name) > 0);
}

/* ─── wlr_platform singleton ─────────────────────────────────────────────── */

TEST_CASE("wlr_platform returns non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat != nullptr);
}

TEST_CASE("wlr_platform name matches wlr_platform_name")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(std::strcmp(plat->name, wlr_platform_name()) == 0);
}

/* ─── clock vtable (all platforms) ─────────────────────────────────────── */

TEST_CASE("clock vtable is non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->clock != nullptr);
}

TEST_CASE("clock->get_time is non-null and returns valid timespec")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->clock->get_time != nullptr);

    struct timespec ts = {0};
    plat->clock->get_time(&ts);

    /* Reasonable time: after Jan 1 2000 UTC epoch */
    REQUIRE(ts.tv_sec > 946684800L);
    REQUIRE(ts.tv_nsec >= 0);
    REQUIRE(ts.tv_nsec < 1000000000);
}

TEST_CASE("clock->get_time is monotonic between calls")
{
    const struct wlr_platform *plat = wlr_platform();
    struct timespec ts1 = {0}, ts2 = {0};

    plat->clock->get_time(&ts1);
    plat->clock->get_time(&ts2);

    /* Second call must not return earlier time */
    REQUIRE((ts2.tv_sec > ts1.tv_sec ||
             (ts2.tv_sec == ts1.tv_sec && ts2.tv_nsec >= ts1.tv_nsec)));
}

/* ─── unix_socket vtable (Linux and BSD) ───────────────────────────────── */

TEST_CASE("unix_socket vtable is non-null on all supported platforms")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->unix_socket != nullptr);
}

TEST_CASE("unix_socket->format_socket_path is non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->unix_socket->format_socket_path != nullptr);
}

TEST_CASE("unix_socket->socket_dir is non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->unix_socket->socket_dir != nullptr);
}

TEST_CASE("unix_socket->lock_fmt is non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->unix_socket->lock_fmt != nullptr);
}

TEST_CASE("unix_socket->unlink_display_socket is non-null")
{
    const struct wlr_platform *plat = wlr_platform();
    REQUIRE(plat->unix_socket->unlink_display_socket != nullptr);
}

/* ─── unix_socket path formatting ──────────────────────────────────────── */

TEST_CASE("format_socket_path produces a non-empty, null-terminated string")
{
    const struct wlr_platform *plat = wlr_platform();
    char buf[256];
    int len = plat->unix_socket->format_socket_path(buf, sizeof(buf), 42);
    REQUIRE(len > 0);
    REQUIRE(std::strlen(buf) > 0);
}

TEST_CASE("socket_dir returns a non-empty string")
{
    const struct wlr_platform *plat = wlr_platform();
    const char *dir = plat->unix_socket->socket_dir();
    REQUIRE(dir != nullptr);
    REQUIRE(std::strlen(dir) > 0);
}

/* ─── OS-specific name assertions ──────────────────────────────────────── */

#if defined(__linux__)
TEST_CASE("Linux platform identifies as 'linux'")
{
    const char *name = wlr_platform_name();
    REQUIRE(std::strcmp(name, "linux") == 0);
}
#elif defined(__FreeBSD__)
TEST_CASE("FreeBSD platform identifies as 'freebsd'")
{
    const char *name = wlr_platform_name();
    REQUIRE(std::strcmp(name, "freebsd") == 0);
}
#elif defined(__OpenBSD__)
TEST_CASE("OpenBSD platform identifies as 'openbsd'")
{
    const char *name = wlr_platform_name();
    REQUIRE(std::strcmp(name, "openbsd") == 0);
}
#elif defined(__NetBSD__)
TEST_CASE("NetBSD platform identifies as 'netbsd'")
{
    const char *name = wlr_platform_name();
    REQUIRE(std::strcmp(name, "netbsd") == 0);
}
#elif defined(__DragonFly__)
TEST_CASE("DragonFly platform identifies as 'dragonfly'")
{
    const char *name = wlr_platform_name();
    REQUIRE(std::strcmp(name, "dragonfly") == 0);
}
#endif
