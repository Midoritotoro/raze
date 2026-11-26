#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/ExclusiveScan.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::exclusive_scan", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i & 1);

        std::vector<T> exp(sz), o1(sz), o2(sz);
        std::exclusive_scan(src.begin(), src.end(), exp.begin(), T(10));

        auto r1 = raze::algorithm::exclusive_scan(src, o1.begin(), T(10));
        auto r2 = raze::algorithm::exclusive_scan[opt::fscalar](src.begin(), src.end(), o2.begin(), T(10));

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp);
        RTTS_ALL_EQUAL(o2, exp);

        // In-place
        auto ip = src;
        raze::algorithm::exclusive_scan(ip, ip.begin(), T(10));
        RTTS_ALL_EQUAL(ip, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> src{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> dst{};
        auto r = raze::algorithm::exclusive_scan(src, dst.begin(), T(10));
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 4>{ T(10), T(11), T(13), T(16) };
    }());
};

RTTS_CASE("raze::algorithm::exclusive_scan custom op, single-pass input, and dangling") {
    const std::vector<int> src{ 2, 3, 4 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::exclusive_scan(sp, std::back_inserter(out), 1, std::multiplies<>{});
    RTTS_EXPECT((out == std::vector<int>{ 1, 2, 6 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::exclusive_scan(std::vector<int>{ 1, 2, 3 }, o2.begin(), 5);
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 5, 6, 8 }));
};

