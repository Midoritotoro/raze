#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/InclusiveScan.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::inclusive_scan", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i & 1);

        std::vector<T> exp1(sz), exp2(sz), o1(sz), o2(sz);
        std::inclusive_scan(src.begin(), src.end(), exp1.begin());
        std::inclusive_scan(src.begin(), src.end(), exp2.begin(), std::plus<>{}, T(10));

        auto r1 = raze::algorithm::inclusive_scan(src, o1.begin());
        auto r2 = raze::algorithm::inclusive_scan[opt::fscalar](src.begin(), src.end(), o2.begin(), std::plus<>{}, T(10));

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp1);
        RTTS_ALL_EQUAL(o2, exp2);

        // In-place
        auto ip = src;
        raze::algorithm::inclusive_scan(ip, ip.begin(), std::plus<>{}, T(10));
        RTTS_ALL_EQUAL(ip, exp2);
    }

    static_assert([]() constexpr {
        std::array<T, 4> src{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> d1{}, d2{};
        raze::algorithm::inclusive_scan(src, d1.begin());
        raze::algorithm::inclusive_scan(src, d2.begin(), std::plus<>{}, T(10));
        return d1 == std::array<T, 4>{ T(1), T(3), T(6), T(10) }
            && d2 == std::array<T, 4>{ T(11), T(13), T(16), T(20) };
    }());
};

RTTS_CASE("raze::algorithm::inclusive_scan single-pass input, back_inserter, and dangling") {
    const std::vector<int> src{ 2, 3, 4 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::inclusive_scan(sp, std::back_inserter(out), std::multiplies<>{}, 10);
    RTTS_EXPECT((out == std::vector<int>{ 20, 60, 240 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::inclusive_scan(std::vector<int>{ 1, 2, 3 }, o2.begin());
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 1, 3, 6 }));
};

