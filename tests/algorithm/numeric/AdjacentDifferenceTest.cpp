#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/AdjacentDifference.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::adjacent_difference", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(32), size_t(128) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i % 50);

        std::vector<T> exp(sz), o1(sz), o2(sz);
        std::adjacent_difference(src.begin(), src.end(), exp.begin());

        auto r1 = raze::algorithm::adjacent_difference(src, o1.begin());
        auto r2 = raze::algorithm::adjacent_difference[opt::fscalar](src.begin(), src.end(), o2.begin());

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp);
        RTTS_ALL_EQUAL(o2, exp);

        // In-place
        auto ip = src;
        raze::algorithm::adjacent_difference(ip, ip.begin());
        RTTS_ALL_EQUAL(ip, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 5> src{ T(2), T(4), T(7), T(11), T(16) };
        std::array<T, 5> dst{};
        auto r = raze::algorithm::adjacent_difference(src, dst.begin());
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 5>{ T(2), T(2), T(3), T(4), T(5) };
    }());
};

RTTS_CASE("raze::algorithm::adjacent_difference custom op, single-pass input, and dangling") {
    const std::vector<int> src{ 1, 2, 3, 4 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::adjacent_difference(sp, std::back_inserter(out), std::plus<>{});
    RTTS_EXPECT((out == std::vector<int>{ 1, 3, 5, 7 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::adjacent_difference(std::vector<int>{ 10, 15, 25 }, o2.begin());
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 10, 5, 10 }));
};

