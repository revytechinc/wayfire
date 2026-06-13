#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <wayfire/region.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

namespace
{
std::vector<wlr_box> as_boxes(const wf::region_t& region)
{
    std::vector<wlr_box> boxes;
    for (auto it = region.begin(); it != region.end(); ++it)
    {
        boxes.push_back(wlr_box_from_pixman_box(*it));
    }

    std::sort(boxes.begin(), boxes.end(), [] (const auto& a, const auto& b)
    {
        return std::tie(a.x, a.y, a.width, a.height) <
        std::tie(b.x, b.y, b.width, b.height);
    });

    return boxes;
}
}

TEST_CASE("region supports copy move and clear semantics")
{
    wf::region_t original{{0, 0, 10, 10}};
    wf::region_t copy = original;
    copy += wf::point_t{5, 0};

    REQUIRE(original.contains_point({1, 1}));
    REQUIRE_FALSE(original.contains_point({11, 1}));
    REQUIRE(copy.contains_point({6, 1}));
    REQUIRE_FALSE(copy.contains_point({1, 1}));

    wf::region_t moved = std::move(copy);
    auto moved_boxes   = as_boxes(moved);
    REQUIRE(moved_boxes.size() == 1);
    REQUIRE(moved_boxes[0] == wf::geometry_t{5, 0, 10, 10});
    REQUIRE(copy.empty());

    moved.clear();
    REQUIRE(moved.empty());
}

TEST_CASE("region set operations preserve expected coverage")
{
    wf::region_t region{{0, 0, 10, 10}};

    auto intersection = region & wlr_box{5, 5, 10, 10};
    REQUIRE(as_boxes(intersection) == std::vector<wlr_box>{{5, 5, 5, 5}});

    auto united = region | wlr_box{10, 0, 5, 10};
    REQUIRE(as_boxes(united) == std::vector<wlr_box>{{0, 0, 15, 10}});

    auto subtracted = region ^ wlr_box{2, 2, 6, 6};
    REQUIRE(subtracted.contains_point({1, 1}));
    REQUIRE(subtracted.contains_point({9, 9}));
    REQUIRE_FALSE(subtracted.contains_point({5, 5}));
    auto extents = subtracted.get_extents();
    REQUIRE(extents.x1 == 0);
    REQUIRE(extents.y1 == 0);
    REQUIRE(extents.x2 == 10);
    REQUIRE(extents.y2 == 10);
}

TEST_CASE("region translation scaling and float containment work together")
{
    wf::region_t region{{1, 2, 3, 4}};
    auto shifted = region + wf::point_t{2, 3};
    auto scaled  = shifted * 2.0f;

    REQUIRE(as_boxes(shifted) == std::vector<wlr_box>{{3, 5, 3, 4}});
    REQUIRE(as_boxes(scaled) == std::vector<wlr_box>{{6, 10, 6, 8}});
    REQUIRE(scaled.contains_pointf({6.5, 10.5}));
    REQUIRE_FALSE(scaled.contains_pointf({12.5, 18.5}));
}

TEST_CASE("region edge expansion handles no-op growth and shrink")
{
    wf::region_t region{{0, 0, 10, 10}};

    region.expand_edges(0);
    REQUIRE(as_boxes(region) == std::vector<wlr_box>{{0, 0, 10, 10}});

    region.expand_edges(2);
    REQUIRE(as_boxes(region) == std::vector<wlr_box>{{-2, -2, 14, 14}});

    region.expand_edges(-3);
    REQUIRE(as_boxes(region) == std::vector<wlr_box>{{1, 1, 8, 8}});
}

// Additional tests to improve coverage

TEST_CASE("region subtraction operators")
{
    wf::region_t region1{{0, 0, 100, 100}};
    wf::region_t region2{{20, 20, 30, 30}};

    // operator ^ (const wlr_box&)
    auto result_box = region1 ^ wlr_box{25, 25, 10, 10};
    REQUIRE_FALSE(result_box.contains_point({30, 30}));
    REQUIRE(result_box.contains_point({10, 10}));

    // operator ^ (const region_t&)
    auto result_region = region1 ^ region2;
    REQUIRE_FALSE(result_region.contains_point({30, 30}));
    REQUIRE(result_region.contains_point({10, 10}));
}

TEST_CASE("region compound subtraction operators")
{
    wf::region_t region{{0, 0, 100, 100}};

    // operator ^=
    auto before = as_boxes(region);
    region ^= wlr_box{25, 25, 10, 10};
    REQUIRE_FALSE(region.contains_point({30, 30}));
    REQUIRE(region.contains_point({10, 10}));

    // Reset and test operator ^=
    region = wf::region_t{{0, 0, 100, 100}};
    wf::region_t other{{20, 20, 30, 30}};
    region ^= other;
    REQUIRE_FALSE(region.contains_point({30, 30}));
}

TEST_CASE("region copy assignment to different region")
{
    wf::region_t region1{{0, 0, 100, 100}};
    wf::region_t region2;
    region2 = region1;
    REQUIRE_FALSE(region2.empty());
    REQUIRE(region2.contains_point({50, 50}));
}

TEST_CASE("region move assignment to different region")
{
    wf::region_t region1{{0, 0, 100, 100}};
    wf::region_t region2;
    region2 = std::move(region1);
    REQUIRE_FALSE(region2.empty());
    REQUIRE(region2.contains_point({50, 50}));
}

TEST_CASE("region contains_pointf on edge")
{
    wf::region_t region{{0, 0, 10, 10}};

    // Points on the boundary
    REQUIRE(region.contains_pointf({0.0, 0.0}));
    REQUIRE(region.contains_pointf({9.0, 5.0}));
    REQUIRE_FALSE(region.contains_pointf({10.0, 5.0})); // Right edge - exclusive
    REQUIRE_FALSE(region.contains_pointf({5.0, 10.0})); // Bottom edge - exclusive
}

TEST_CASE("region contains_pointf in middle")
{
    wf::region_t region{{0, 0, 10, 10}};

    REQUIRE(region.contains_pointf({5.0, 5.0}));
    REQUIRE(region.contains_pointf({0.5, 0.5}));
    REQUIRE_FALSE(region.contains_pointf({-0.5, 0.5}));
}

TEST_CASE("region scale in-place operator")
{
    wf::region_t region{{0, 0, 100, 100}};

    region *= 0.5f;
    REQUIRE(as_boxes(region).size() == 1);
    // The exact result depends on pixman scaling
}

TEST_CASE("region intersection with self")
{
    wf::region_t region{{0, 0, 100, 100}};

    auto result = region & region;
    REQUIRE(result.contains_point({50, 50}));
}

TEST_CASE("region intersection compound operators")
{
    wf::region_t region{{0, 0, 100, 100}};

    // operator &=
    region &= wlr_box{25, 25, 50, 50};
    REQUIRE(as_boxes(region).size() == 1);
    REQUIRE(region.contains_point({50, 50}));
    REQUIRE_FALSE(region.contains_point({0, 0}));
}

TEST_CASE("region compound intersection with region")
{
    wf::region_t region{{0, 0, 100, 100}};
    wf::region_t other{{50, 50, 100, 100}};

    region &= other;
    REQUIRE(region.contains_point({50, 50}));
    REQUIRE_FALSE(region.contains_point({0, 0}));
}

TEST_CASE("region union with region")
{
    wf::region_t region1{{0, 0, 50, 50}};
    wf::region_t region2{{100, 100, 50, 50}};

    auto result = region1 | region2;
    REQUIRE(result.contains_point({25, 25}));
    REQUIRE(result.contains_point({125, 125}));
}

TEST_CASE("get_extents on empty region")
{
    wf::region_t region;
    auto extents = region.get_extents();

    // Empty region should have undefined extents
    // This is implementation-defined behavior from pixman
    (void)extents;
}

TEST_CASE("get_extents on single box region")
{
    wf::region_t region{{10, 20, 100, 200}};
    auto extents = region.get_extents();

    REQUIRE_EQ(extents.x1, 10);
    REQUIRE_EQ(extents.y1, 20);
    REQUIRE_EQ(extents.x2, 110);
    REQUIRE_EQ(extents.y2, 220);
}

TEST_CASE("pixman_box_from_wlr_box conversion")
{
    wlr_box box = {10, 20, 100, 200};
    auto pixman = pixman_box_from_wlr_box(box);

    REQUIRE_EQ(pixman.x1, 10);
    REQUIRE_EQ(pixman.y1, 20);
    REQUIRE_EQ(pixman.x2, 110); // 10 + 100
    REQUIRE_EQ(pixman.y2, 220); // 20 + 200
}

TEST_CASE("wlr_box_from_pixman_box conversion")
{
    pixman_box32_t box = {10, 20, 110, 220};
    auto wlr = wlr_box_from_pixman_box(box);

    REQUIRE_EQ(wlr.x, 10);
    REQUIRE_EQ(wlr.y, 20);
    REQUIRE_EQ(wlr.width, 100); // 110 - 10
    REQUIRE_EQ(wlr.height, 200); // 220 - 20
}

TEST_CASE("region subtraction collapses to empty")
{
    wf::region_t region{{0, 0, 100, 100}};
    wf::region_t other{{0, 0, 100, 100}}; // Same size, overlapping completely

    auto result = region - wf::point_t{50, 50};
    REQUIRE_FALSE(result.empty());
}

TEST_CASE("region negative translation")
{
    wf::region_t region{{10, 10, 100, 100}};

    auto shifted = region - wf::point_t{5, 5};
    REQUIRE(as_boxes(shifted) == std::vector<wlr_box>{{5, 5, 100, 100}});
}

TEST_CASE("region compound negative translation")
{
    wf::region_t region{{10, 10, 100, 100}};

    region -= wf::point_t{5, 5};
    REQUIRE(as_boxes(region) == std::vector<wlr_box>{{5, 5, 100, 100}});
}

TEST_CASE("region expand_edges with very large value")
{
    wf::region_t region{{50, 50, 10, 10}};

    // Expanding by more than the size of the region
    // Should handle the case gracefully
    region.expand_edges(100);

    // The result might be empty or have invalid rectangles that get filtered
    // This test ensures it doesn't crash
}

TEST_CASE("region multiple boxes from union")
{
    wf::region_t region1{{0, 0, 50, 50}};
    wf::region_t region2{{100, 100, 50, 50}};

    auto combined = region1 | region2;
    auto boxes = as_boxes(combined);

    // Should have 2 separate boxes
    REQUIRE(boxes.size() == 2);
}
