#include <tests/rts/rts.h>
#include <raze/algorithm/set/Includes.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::includes", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32), size_t(128) }) {
        auto a = rtts::random::vector<T>(sz, 620u + static_cast<unsigned>(sz));
        std::ranges::sort(a);

        std::vector<T> sub;
        for (size_t i = 0; i < sz; i += 2) sub.push_back(a[i]);

        RTTS_EXPECT(raze::algorithm::includes(a, sub));
        RTTS_EXPECT(raze::algorithm::includes(a.begin(), a.end(), sub.begin(), sub.end()));
        RTTS_EXPECT(raze::algorithm::includes[opt::fscalar](a, sub));
        RTTS_EXPECT(raze::algorithm::includes[opt::fstatic](a, sub));

        auto b = rtts::random::vector<T>(sz / 2 + 1, 630u + static_cast<unsigned>(sz));
        std::ranges::sort(b);
        RTTS_EQUAL(raze::algorithm::includes(a, b), std::ranges::includes(a, b));
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(2), T(3), T(4), T(5) };
        std::array<T, 3> s1{ T(2), T(2), T(5) };
        std::array<T, 3> s2{ T(2), T(2), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::includes(a, s1)
            && !raze::algorithm::includes(a, s2)
            && raze::algorithm::includes(a, e)
            && raze::algorithm::includes(e, e)
            && !raze::algorithm::includes(e, s1);
    }());
};

RTTS_CASE("raze::algorithm::includes projections, single-pass input, and non-common") {
    const std::vector<rtts::test_item> a{ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 } };
    const std::vector<int> b{ 2, 4 };
    RTTS_EXPECT(raze::algorithm::includes(a, b, std::less<>{}, &rtts::test_item::key));

    const std::vector<int> v1{ 1, 2, 3, 4, 5 };
    const std::vector<int> v2{ 2, 4 };
    rtts::single_pass_input_range<int> sp1(v1);
    rtts::single_pass_input_range<int> sp2(v2);
    RTTS_EXPECT(raze::algorithm::includes(sp1, sp2));
    RTTS_EXPECT(raze::algorithm::includes(rtts::make_non_common(v1), rtts::make_non_common(v2)));
};

