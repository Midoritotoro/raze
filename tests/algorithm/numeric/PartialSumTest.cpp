#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/PartialSum.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::partial_sum", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i & 1);

        std::vector<T> exp(sz), o1(sz), o2(sz);
        std::partial_sum(src.begin(), src.end(), exp.begin());

        auto r1 = raze::algorithm::partial_sum(src, o1.begin());
        auto r2 = raze::algorithm::partial_sum[opt::fscalar](src.begin(), src.end(), o2.begin());

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp);
        RTTS_ALL_EQUAL(o2, exp);

        // In-place partial_sum
        auto ip = src;
        raze::algorithm::partial_sum(ip, ip.begin());
        RTTS_ALL_EQUAL(ip, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 5> src{ T(1), T(2), T(3), T(4), T(5) };
        std::array<T, 5> dst{};
        auto r = raze::algorithm::partial_sum(src, dst.begin());
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 5>{ T(1), T(3), T(6), T(10), T(15) };
    }());
};

RTTS_CASE("raze::algorithm::partial_sum custom op, single-pass input, back_inserter, and dangling") {
    const std::vector<int> src{ 1, 2, 3, 4 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::partial_sum(sp, std::back_inserter(out), std::multiplies<>{});
    RTTS_EXPECT((out == std::vector<int>{ 1, 2, 6, 24 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::partial_sum(std::vector<int>{ 2, 3, 4 }, o2.begin());
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 2, 5, 9 }));
};

