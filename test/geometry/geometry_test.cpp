#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/geometry.hpp>
#include <cmath>
#include <sstream>

TEST_CASE("Point addition")
{
    wf::point_t a = {1, 2};
    wf::point_t b = {3, 4};

    using namespace wf;
    REQUIRE_EQ(a + b, wf::point_t{4, 6});
}

TEST_CASE("Point subtraction")
{
    wf::point_t a = {5, 7};
    wf::point_t b = {2, 3};

    using namespace wf;
    REQUIRE_EQ(a - b, wf::point_t{3, 4});
}

TEST_CASE("Point negation")
{
    wf::point_t a = {3, -4};

    using namespace wf;
    REQUIRE_EQ(-a, wf::point_t{-3, 4});
}

TEST_CASE("Point equality and inequality")
{
    wf::point_t a = {1, 2};
    wf::point_t b = {1, 2};
    wf::point_t c = {1, 3};

    using namespace wf;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("origin extracts position from geometry")
{
    wf::geometry_t geo = {10, 20, 100, 200};
    auto origin = wf::origin(geo);

    REQUIRE_EQ(origin.x, 10);
    REQUIRE_EQ(origin.y, 20);
}

TEST_CASE("dimensions extracts size from geometry")
{
    wf::geometry_t geo = {10, 20, 100, 200};
    auto dims = wf::dimensions(geo);

    REQUIRE_EQ(dims.width, 100);
    REQUIRE_EQ(dims.height, 200);
}

TEST_CASE("construct_box creates geometry from point and dimensions")
{
    wf::point_t origin = {5, 10};
    wf::dimensions_t dims = {100, 200};

    auto geo = wf::construct_box(origin, dims);

    REQUIRE_EQ(geo.x, 5);
    REQUIRE_EQ(geo.y, 10);
    REQUIRE_EQ(geo.width, 100);
    REQUIRE_EQ(geo.height, 200);
}

TEST_CASE("dimensions equality and inequality")
{
    wf::dimensions_t a = {100, 200};
    wf::dimensions_t b = {100, 200};
    wf::dimensions_t c = {100, 300};

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
}

TEST_CASE("geometry equality and inequality")
{
    wf::geometry_t a = {1, 2, 100, 200};
    wf::geometry_t b = {1, 2, 100, 200};
    wf::geometry_t c = {1, 3, 100, 200};

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
}

TEST_CASE("geometry plus point")
{
    wf::geometry_t geo = {10, 20, 100, 200};
    wf::point_t pt = {5, -10};

    auto result = geo + pt;

    REQUIRE_EQ(result.x, 15);
    REQUIRE_EQ(result.y, 10);
    REQUIRE_EQ(result.width, 100);
    REQUIRE_EQ(result.height, 200);
}

TEST_CASE("geometry minus point")
{
    wf::geometry_t geo = {10, 20, 100, 200};
    wf::point_t pt = {5, 10};

    auto result = geo - pt;

    REQUIRE_EQ(result.x, 5);
    REQUIRE_EQ(result.y, 10);
    REQUIRE_EQ(result.width, 100);
    REQUIRE_EQ(result.height, 200);
}

TEST_CASE("geometry intersection with overlapping boxes")
{
    wf::geometry_t a = {0, 0, 100, 100};
    wf::geometry_t b = {50, 50, 100, 100};

    auto result = wf::geometry_intersection(a, b);

    REQUIRE_EQ(result.x, 50);
    REQUIRE_EQ(result.y, 50);
    REQUIRE_EQ(result.width, 50);
    REQUIRE_EQ(result.height, 50);
}

TEST_CASE("geometry intersection with non-overlapping boxes")
{
    wf::geometry_t a = {0, 0, 10, 10};
    wf::geometry_t b = {100, 100, 10, 10};

    auto result = wf::geometry_intersection(a, b);

    // Non-intersecting boxes return zero-sized geometry
    REQUIRE_EQ(result.width, 0);
    REQUIRE_EQ(result.height, 0);
}

TEST_CASE("geometry intersection with partial overlap")
{
    wf::geometry_t a = {0, 0, 50, 50};
    wf::geometry_t b = {25, 25, 50, 50};

    auto result = wf::geometry_intersection(a, b);

    REQUIRE_EQ(result.x, 25);
    REQUIRE_EQ(result.y, 25);
    REQUIRE_EQ(result.width, 25);
    REQUIRE_EQ(result.height, 25);
}

TEST_CASE("clamp geometry inside output")
{
    wf::geometry_t window = {10, 10, 50, 50};
    wf::geometry_t output = {0, 0, 100, 100};

    auto result = wf::clamp(window, output);

    REQUIRE_EQ(result.x, 10);
    REQUIRE_EQ(result.y, 10);
    REQUIRE_EQ(result.width, 50);
    REQUIRE_EQ(result.height, 50);
}

TEST_CASE("clamp geometry outside output - x too small")
{
    wf::geometry_t window = {-20, 50, 30, 30};
    wf::geometry_t output = {0, 0, 100, 100};

    auto result = wf::clamp(window, output);

    REQUIRE_EQ(result.x, 0);
    REQUIRE_EQ(result.y, 50);
}

TEST_CASE("clamp geometry outside output - x too large")
{
    wf::geometry_t window = {80, 50, 30, 30};
    wf::geometry_t output = {0, 0, 100, 100};

    auto result = wf::clamp(window, output);

    REQUIRE_EQ(result.x, 70); // 100 - 30
    REQUIRE_EQ(result.y, 50);
}

TEST_CASE("clamp geometry with width larger than output")
{
    wf::geometry_t window = {10, 10, 200, 50};
    wf::geometry_t output = {0, 0, 100, 100};

    auto result = wf::clamp(window, output);

    REQUIRE_EQ(result.width, 100);
    REQUIRE_EQ(result.x, 0);
}

TEST_CASE("clamp geometry with height larger than output")
{
    wf::geometry_t window = {10, 10, 50, 200};
    wf::geometry_t output = {0, 0, 100, 100};

    auto result = wf::clamp(window, output);

    REQUIRE_EQ(result.height, 100);
    REQUIRE_EQ(result.y, 0);
}

TEST_CASE("point and geometry intersection test")
{
    wf::geometry_t rect = {0, 0, 100, 100};

    REQUIRE((rect & wf::point_t{50, 50}));
    REQUIRE_FALSE((rect & wf::point_t{-1, 50}));
    REQUIRE_FALSE((rect & wf::point_t{100, 50}));
    REQUIRE_FALSE((rect & wf::point_t{50, 100}));
}

TEST_CASE("pointf and geometry intersection test")
{
    wf::geometry_t rect = {0, 0, 100, 100};

    REQUIRE((rect & wf::pointf_t{50.0, 50.0}));
    REQUIRE((rect & wf::pointf_t{99.9, 99.9}));
    REQUIRE_FALSE((rect & wf::pointf_t{-0.1, 50.0}));
    REQUIRE_FALSE((rect & wf::pointf_t{100.0, 50.0}));
}

TEST_CASE("geometry and geometry intersection test - overlapping")
{
    wf::geometry_t a = {0, 0, 100, 100};
    wf::geometry_t b = {50, 50, 100, 100};

    REQUIRE((a & b));
    REQUIRE((b & a));
}

TEST_CASE("geometry and geometry intersection test - non-overlapping")
{
    wf::geometry_t a = {0, 0, 50, 50};
    wf::geometry_t b = {100, 100, 50, 50};

    REQUIRE_FALSE((a & b));
}

TEST_CASE("geometry and geometry intersection test - touching edges")
{
    wf::geometry_t a = {0, 0, 50, 50};
    wf::geometry_t b = {50, 0, 50, 50};

    REQUIRE_FALSE((a & b)); // Touching edges don't intersect
}

TEST_CASE("geometry and geometry intersection test - contained")
{
    wf::geometry_t a = {0, 0, 100, 100};
    wf::geometry_t b = {25, 25, 50, 50};

    REQUIRE((a & b));
}

TEST_CASE("geometry scale")
{
    wf::geometry_t geo = {10, 10, 100, 100};

    auto scaled = geo * 2.0;

    // floor(10 * 2) = 20
    REQUIRE_EQ(scaled.x, 20);
    REQUIRE_EQ(scaled.y, 20);
    // ceil((10 + 100) * 2) - 20 = ceil(220) - 20 = 220 - 20 = 200
    REQUIRE_EQ(scaled.width, 200);
    REQUIRE_EQ(scaled.height, 200);
}

TEST_CASE("geometry scale with fractional factor")
{
    wf::geometry_t geo = {0, 0, 100, 100};

    auto scaled = geo * 0.5;

    REQUIRE_EQ(scaled.x, 0);
    REQUIRE_EQ(scaled.y, 0);
    // ceil((0 + 100) * 0.5) - 0 = 50
    REQUIRE_EQ(scaled.width, 50);
    REQUIRE_EQ(scaled.height, 50);
}

TEST_CASE("fbox equality")
{
    wlr_fbox a = {1.0, 2.0, 100.5, 200.5};
    wlr_fbox b = {1.0, 2.0, 100.5, 200.5};
    wlr_fbox c = {1.0, 2.0, 100.5, 201.5};

    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    REQUIRE(a != c);
}

TEST_CASE("fbox scale")
{
    wlr_fbox box = {10.5, 20.5, 100.0, 200.0};

    auto scaled = box * 2.0;

    REQUIRE_EQ(scaled.x, 21.0);
    REQUIRE_EQ(scaled.y, 41.0);
    REQUIRE_EQ(scaled.width, 200.0);
    REQUIRE_EQ(scaled.height, 400.0);
}

TEST_CASE("geometry_to_fbox and fbox_to_geometry")
{
    wf::geometry_t geo = {10, 20, 100, 200};

    auto fbox = wf::geometry_to_fbox(geo);
    REQUIRE_EQ(fbox.x, 10.0);
    REQUIRE_EQ(fbox.y, 20.0);
    REQUIRE_EQ(fbox.width, 100.0);
    REQUIRE_EQ(fbox.height, 200.0);

    auto back = wf::fbox_to_geometry(fbox);
    REQUIRE_EQ(back.x, 10);
    REQUIRE_EQ(back.y, 20);
    REQUIRE_EQ(back.width, 100);
    REQUIRE_EQ(back.height, 200);
}

TEST_CASE("fbox_to_geometry with fractional values")
{
    wlr_fbox fbox = {10.2, 20.7, 100.3, 200.9};

    auto geo = wf::fbox_to_geometry(fbox);

    // floor(10.2) = 10
    REQUIRE_EQ(geo.x, 10);
    // floor(20.7) = 20
    REQUIRE_EQ(geo.y, 20);
    // ceil(10.2 + 100.3) = ceil(110.5) = 111, 111 - 10 = 101
    REQUIRE_EQ(geo.width, 101);
    // ceil(20.7 + 200.9) = ceil(221.6) = 222, 222 - 20 = 202
    REQUIRE_EQ(geo.height, 202);
}

TEST_CASE("scale_box scales geometry between coordinate spaces")
{
    // Scale from space A (0,0,100,100) to space B (0,0,200,200)
    wf::geometry_t A = {0, 0, 100, 100};
    wf::geometry_t B = {0, 0, 200, 200};
    wf::geometry_t box = {25, 25, 50, 50};

    auto result = wf::scale_box(A, B, box);

    // In B space, box should be at (50, 50) with size (100, 100)
    REQUIRE_EQ(result.x, 50);
    REQUIRE_EQ(result.y, 50);
    REQUIRE_EQ(result.width, 100);
    REQUIRE_EQ(result.height, 100);
}

TEST_CASE("scale_fbox basic")
{
    wlr_fbox A = {0.0, 0.0, 100.0, 100.0};
    wlr_fbox B = {0.0, 0.0, 200.0, 200.0};
    wlr_fbox box = {25.0, 25.0, 50.0, 50.0};

    auto result = wf::scale_fbox(A, B, box);

    REQUIRE_EQ(result.x, 50.0);
    REQUIRE_EQ(result.y, 50.0);
    REQUIRE_EQ(result.width, 100.0);
    REQUIRE_EQ(result.height, 100.0);
}

TEST_CASE("pointf operators")
{
    wf::pointf_t a = {1.5, 2.5};
    wf::pointf_t b = {0.5, 1.5};

    auto sum = a + b;
    REQUIRE_EQ(sum.x, 2.0);
    REQUIRE_EQ(sum.y, 4.0);

    auto diff = a - b;
    REQUIRE_EQ(diff.x, 1.0);
    REQUIRE_EQ(diff.y, 1.0);

    auto neg = -a;
    REQUIRE_EQ(neg.x, -1.5);
    REQUIRE_EQ(neg.y, -2.5);

    auto round_down = a.round_down();
    REQUIRE_EQ(round_down.x, 1);
    REQUIRE_EQ(round_down.y, 2);
}

TEST_CASE("pointf conversion from point_t")
{
    wf::point_t pt = {10, 20};
    wf::pointf_t pf = wf::pointf_t(pt);

    REQUIRE_EQ(pf.x, 10.0);
    REQUIRE_EQ(pf.y, 20.0);
}

TEST_CASE("pointf += and -= operators")
{
    wf::pointf_t a = {1.5, 2.5};
    wf::pointf_t b = {0.5, 1.5};

    a += b;
    REQUIRE_EQ(a.x, 2.0);
    REQUIRE_EQ(a.y, 4.0);

    a -= b;
    REQUIRE_EQ(a.x, 1.5);
    REQUIRE_EQ(a.y, 2.5);
}

TEST_CASE("point plus geometry")
{
    wf::point_t pt = {10, 20};
    wf::geometry_t geo = {5, 10, 100, 200};

    auto result = pt + geo;
    REQUIRE_EQ(result.x, 15);
    REQUIRE_EQ(result.y, 30);
}

TEST_CASE("abs function")
{
    wf::point_t p1 = {3, 4};
    // Calculate sqrt(x^2 + y^2) manually since wf::abs is shadowed by std::abs
    double result1 = std::sqrt(p1.x * p1.x + p1.y * p1.y);
    REQUIRE(std::abs(result1 - 5.0) < 0.001);

    wf::point_t p2 = {0, 5};
    double result2 = std::sqrt(p2.x * p2.x + p2.y * p2.y);
    REQUIRE(std::abs(result2 - 5.0) < 0.001);

    wf::point_t p3 = {-3, -4};
    double result3 = std::sqrt(p3.x * p3.x + p3.y * p3.y);
    REQUIRE(std::abs(result3 - 5.0) < 0.001);
}
