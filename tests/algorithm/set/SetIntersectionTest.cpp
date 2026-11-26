#include <tests/rts/rts.h>
#include <raze/algorithm/set/SetIntersection.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::set_intersection", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t n1 : { size_t(0), size_t(1), size_t(15), size_t(64) }) {
        for (size_t n2 : { size_t(0), size_t(1), size_t(17), size_t(64) }) {
            auto a = rtts::random::vector<T>(n1, 660u + static_cast<unsigned>(n1));
            auto b = rtts::random::vector<T>(n2, 670u + static_cast<unsigned>(n2));
            std::ranges::sort(a);
            std::ranges::sort(b);

            std::vector<T> exp, o1, o2;
            std::ranges::set_intersection(a, b, std::back_inserter(exp));
            auto r1 = raze::algorithm::set_intersection(a, b, std::back_inserter(o1));
            auto r2 = raze::algorithm::set_intersection[opt::fscalar](a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(o2));

            RTTS_EXPECT(r1.in1 == a.end() && r1.in2 == b.end());
            RTTS_EXPECT(r2.in1 == a.end() && r2.in2 == b.end());
            RTTS_ALL_EQUAL(o1, exp);
            RTTS_ALL_EQUAL(o2, exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(2), T(3), T(5) };
        std::array<T, 5> b{ T(2), T(2), T(2), T(4), T(5) };
        std::array<T, 5> out{};
        auto r = raze::algorithm::set_intersection(a, b, out.begin());
        return r.in1 == a.end() && r.in2 == b.end() && r.out == out.begin() + 3
            && out[0] == T(2) && out[1] == T(2) && out[2] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::set_intersection projections, single-pass, and dangling") {
    const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 11 }, { 3, 12 } };
    const std::vector<rtts::test_item> b{ { 2, 20 }, { 3, 21 } };
    std::vector<rtts::test_item> out;
    raze::algorithm::set_intersection(a, b, std::back_inserter(out), std::less<>{},
        &rtts::test_item::key, &rtts::test_item::key);
    RTTS_EQUAL(out.size(), size_t(2));
    RTTS_EXPECT(out[0].payload == 11 && out[1].payload == 12);

    const std::vector<int> v1{ 1, 2, 3, 4 };
    const std::vector<int> v2{ 2, 4, 6 };
    rtts::single_pass_input_range<int> sp1(v1);
    rtts::single_pass_input_range<int> sp2(v2);
    std::vector<int> o2;
    raze::algorithm::set_intersection(sp1, sp2, std::back_inserter(o2));
    RTTS_EXPECT((o2 == std::vector<int>{ 2, 4 }));

    std::vector<int> o3;
    auto dr = raze::algorithm::set_intersection(std::vector<int>{ 1, 2 }, std::vector<int>{ 2, 3 }, std::back_inserter(o3));
    static_assert(std::same_as<decltype(dr.in1), std::ranges::dangling>);
    static_assert(std::same_as<decltype(dr.in2), std::ranges::dangling>);
};

