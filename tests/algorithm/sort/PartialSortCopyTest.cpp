#include <tests/rts/rts.h>
#include <raze/algorithm/sort/PartialSortCopy.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::partial_sort_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t src_sz : { size_t(0), size_t(1), size_t(7), size_t(32), size_t(100) }) {
        const auto src = rtts::random::vector<T>(src_sz, 830u + static_cast<unsigned>(src_sz));
        for (size_t dst_sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(120) }) {
            std::vector<T> exp(dst_sz, T(0));
            auto er = std::ranges::partial_sort_copy(src, exp);

            std::vector<T> o1(dst_sz, T(0)), o2(dst_sz, T(0));
            auto r1 = raze::algorithm::partial_sort_copy(src, o1);
            auto r2 = raze::algorithm::partial_sort_copy[opt::fscalar](
                src.begin(), src.end(), o2.begin(), o2.end());

            RTTS_EXPECT(r1.in == src.end() && (r1.out - o1.begin()) == (er.out - exp.begin()));
            RTTS_EXPECT(r2.in == src.end() && (r2.out - o2.begin()) == (er.out - exp.begin()));
            RTTS_ALL_EQUAL(o1, exp);
            RTTS_ALL_EQUAL(o2, exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> src{ T(9), T(3), T(7), T(1), T(5), T(2) };
        std::array<T, 3> dst{};
        auto r = raze::algorithm::partial_sort_copy(src, dst);
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 3>{ T(1), T(2), T(3) };
    }());
};

RTTS_CASE("raze::algorithm::partial_sort_copy single-pass input, projections, and dangling") {
    const std::vector<int> src{ 9, 4, 7, 1, 8, 2, 5 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> dst(4, 0);
    auto r = raze::algorithm::partial_sort_copy(sp, dst);
    RTTS_EXPECT(r.in == sp.end() && r.out == dst.end());
    RTTS_EXPECT((dst == std::vector<int>{ 1, 2, 4, 5 }));

    const std::vector<rtts::test_item> items{ { 5, 1 }, { 1, 2 }, { 9, 3 }, { 2, 4 } };
    std::vector<rtts::test_item> out(2);
    raze::algorithm::partial_sort_copy(items, out, std::greater<>{},
        &rtts::test_item::key, &rtts::test_item::key);
    RTTS_EXPECT(out[0].key == 9 && out[1].key == 5);

    std::vector<int> d2(2);
    auto dr = raze::algorithm::partial_sort_copy(std::vector<int>{ 3, 1, 2 }, d2);
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((d2 == std::vector<int>{ 1, 2 }));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void partial_sort_copy_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 257);
            const size_t dst_n = static_cast<size_t>(rng() % 257);
            std::vector<T> src(src_n);
            if ((it & 1u) == 0u) {
                src = rtts::random::vector<T>(src_n, rng());
            }
            else {
                for (size_t i = 0; i < src_n; ++i)
                    src[i] = static_cast<T>(rng() % 8u);
            }

            const bool desc = (it % 3 == 2);
            auto run_check = [&](auto comp) {
                std::vector<T> exp(dst_n, T(77));
                auto er = std::ranges::partial_sort_copy(src, exp, comp);
                const auto exp_cnt = er.out - exp.begin();

                auto verify = [&](auto fn) {
                    std::vector<T> dst(dst_n, T(77));
                    auto r = fn(src, dst);
                    RTTS_EXPECT(r.in == src.end() && (r.out - dst.begin()) == exp_cnt);
                    RTTS_ALL_EQUAL(dst, exp);
                };

                verify([&](const auto& s, auto& d) { return raze::algorithm::partial_sort_copy(s, d, comp); });
                verify([&](const auto& s, auto& d) {
                    return raze::algorithm::partial_sort_copy(s.begin(), s.end(), d.begin(), d.end(), comp);
                });
                verify([&](const auto& s, auto& d) { return raze::algorithm::partial_sort_copy[opt::fscalar](s, d, comp); });
                verify([&](const auto& s, auto& d) { return raze::algorithm::partial_sort_copy[opt::fstatic](s, d, comp); });

                if (it % 8 == 0 && src_n <= 64 && dst_n <= 64) {
                    rtts::single_pass_input_range<T> sp(src);
                    std::vector<T> dst(dst_n, T(77));
                    auto r = raze::algorithm::partial_sort_copy(sp, dst, comp);
                    RTTS_EXPECT(r.in == sp.end() && (r.out - dst.begin()) == exp_cnt);
                    RTTS_ALL_EQUAL(dst, exp);
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::partial_sort_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        partial_sort_copy_fuzz<T>(seed, 35);
};
