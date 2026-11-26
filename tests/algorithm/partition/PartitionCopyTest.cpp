#include <tests/rts/rts.h>
#include <raze/algorithm/partition/PartitionCopy.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::partition_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();
    auto pred = [](T x) { return static_cast<int>(x) % 2 == 0; };

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::partition_copy(c.first(), c.last(), c.out(), c.out2(), pred); },
        [pred](auto c) { return std::ranges::partition_copy(c.first(), c.last(), c.out(), c.out2(), pred); });

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::partition_copy(c.range(), c.out(), c.out2(), pred); },
        [pred](auto c) { return std::ranges::partition_copy(c.range(), c.out(), c.out2(), pred); });

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::partition_copy[opt::fscalar](c.range(), c.out(), c.out2(), pred); },
        [pred](auto c) { return std::ranges::partition_copy(c.range(), c.out(), c.out2(), pred); });

    static_assert([]() constexpr {
        std::array<T, 6> src{ T(1), T(2), T(3), T(4), T(5), T(6) };
        std::array<T, 6> ot{}, of{};
        auto r = raze::algorithm::partition_copy(src, ot.begin(), of.begin(),
            [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; });
        return r.in == src.end() && r.out1 == ot.begin() + 3 && r.out2 == of.begin() + 3
            && ot[0] == T(2) && ot[2] == T(6) && of[0] == T(1) && of[2] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::partition_copy single-pass, projections, back_inserter, and dangling") {
    const std::vector<int> src{ 1, 2, 3, 4, 5, 6 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> t_out, f_out;
    raze::algorithm::partition_copy(sp, std::back_inserter(t_out), std::back_inserter(f_out),
        [](int x) { return x % 2 == 0; });
    RTTS_EQUAL(sp.reads(), src.size());
    RTTS_EXPECT((t_out == std::vector<int>{ 2, 4, 6 }));
    RTTS_EXPECT((f_out == std::vector<int>{ 1, 3, 5 }));

    const std::vector<rtts::test_item> items{ { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };
    std::vector<rtts::test_item> ti, fi;
    raze::algorithm::partition_copy(items, std::back_inserter(ti), std::back_inserter(fi),
        [](int k) { return k <= 2; }, &rtts::test_item::key);
    RTTS_EXPECT(ti.size() == 2 && fi.size() == 2 && ti[0].payload == 10 && fi[1].payload == 40);

    std::vector<int> a, b;
    auto dr = raze::algorithm::partition_copy(std::vector<int>{ 1, 2 }, std::back_inserter(a), std::back_inserter(b),
        [](int x) { return x == 1; });
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
};

