#include <tests/rts/rts.h>
#include <raze/algorithm/partition/PartitionCopy.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::partition_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
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

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void partition_copy_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const auto src = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            auto sub = std::ranges::subrange(src.begin() + static_cast<std::ptrdiff_t>(lo),
                                             src.begin() + static_cast<std::ptrdiff_t>(hi));
            const T threshold = val_gen();
            auto pred = [threshold](T x) { return x < threshold; };

            std::vector<T> et(len), ef(len);
            auto re = std::ranges::partition_copy(sub, et.begin(), ef.begin(), pred);
            const auto nt = re.out1 - et.begin();
            const auto nf = re.out2 - ef.begin();

            auto verify = [&](auto fn) {
                std::vector<T> ot(len), of(len);
                auto r = fn(sub, ot.begin(), of.begin(), pred);
                RTTS_EXPECT(r.in == sub.end());
                RTTS_EXPECT(r.out1 - ot.begin() == nt && r.out2 - of.begin() == nf);
                RTTS_EXPECT(std::equal(ot.begin(), r.out1, et.begin()));
                RTTS_EXPECT(std::equal(of.begin(), r.out2, ef.begin()));
            };

            verify([](auto s, auto o1, auto o2, auto p) { return raze::algorithm::partition_copy(s, o1, o2, p); });
            verify([](auto s, auto o1, auto o2, auto p) { return raze::algorithm::partition_copy(s.begin(), s.end(), o1, o2, p); });
            verify([](auto s, auto o1, auto o2, auto p) { return raze::algorithm::partition_copy[opt::fscalar](s, o1, o2, p); });
            verify([](auto s, auto o1, auto o2, auto p) { return raze::algorithm::partition_copy[opt::fstatic](s, o1, o2, p); });

            if (it % 8 == 0 && len <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                std::vector<T> lt, lf;
                raze::algorithm::partition_copy(lst, std::back_inserter(lt), std::back_inserter(lf), pred);
                RTTS_EXPECT(static_cast<std::ptrdiff_t>(lt.size()) == nt && static_cast<std::ptrdiff_t>(lf.size()) == nf);
                RTTS_EXPECT(std::equal(lt.begin(), lt.end(), et.begin()));
                RTTS_EXPECT(std::equal(lf.begin(), lf.end(), ef.begin()));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::partition_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        partition_copy_fuzz<T>(seed, 40);
};
