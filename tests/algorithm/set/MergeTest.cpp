#include <tests/rts/rts.h>
#include <raze/algorithm/set/Merge.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::merge", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t n1 : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        for (size_t n2 : { size_t(0), size_t(1), size_t(9), size_t(35) }) {
            auto a = rtts::random::vector<T>(n1, 600u + static_cast<unsigned>(n1));
            auto b = rtts::random::vector<T>(n2, 700u + static_cast<unsigned>(n2));
            std::ranges::sort(a);
            std::ranges::sort(b);

            std::vector<T> exp(n1 + n2);
            std::ranges::merge(a, b, exp.begin());

            std::vector<T> out1(n1 + n2), out2(n1 + n2);
            auto r1 = raze::algorithm::merge(a, b, out1.begin());
            auto r2 = raze::algorithm::merge[opt::fscalar](a.begin(), a.end(), b.begin(), b.end(), out2.begin());

            RTTS_EXPECT(r1.in1 == a.end() && r1.in2 == b.end() && r1.out == out1.end());
            RTTS_EXPECT(r2.in1 == a.end() && r2.in2 == b.end() && r2.out == out2.end());
            RTTS_ALL_EQUAL(out1, exp);
            RTTS_ALL_EQUAL(out2, exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(3), T(5) };
        std::array<T, 3> b{ T(2), T(3), T(6) };
        std::array<T, 6> out{};
        auto r = raze::algorithm::merge(a, b, out.begin());
        return r.in1 == a.end() && r.in2 == b.end() && r.out == out.end()
            && out == std::array<T, 6>{ T(1), T(2), T(3), T(3), T(5), T(6) };
    }());
};

RTTS_CASE("raze::algorithm::merge stability, projections, single-pass input, and dangling") {
    // Stability: elements from first range precede equivalent elements from second range
    {
        const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 11 }, { 3, 12 } };
        const std::vector<rtts::test_item> b{ { 2, 20 }, { 2, 21 }, { 4, 22 } };
        std::vector<rtts::test_item> out;
        raze::algorithm::merge(a, b, std::back_inserter(out), std::less<>{},
            &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EQUAL(out.size(), size_t(6));
        RTTS_EXPECT(out[1].payload == 11 && out[2].payload == 20 && out[3].payload == 21);
    }

    // Single-pass input ranges & rvalue dangling
    {
        const std::vector<int> a{ 1, 4, 7 };
        const std::vector<int> b{ 2, 3, 8 };
        rtts::single_pass_input_range<int> sp1(a);
        rtts::single_pass_input_range<int> sp2(b);
        std::vector<int> out;
        raze::algorithm::merge(sp1, sp2, std::back_inserter(out));
        RTTS_EXPECT((out == std::vector<int>{ 1, 2, 3, 4, 7, 8 }));

        std::vector<int> o2;
        auto dr = raze::algorithm::merge(std::vector<int>{ 1, 3 }, std::vector<int>{ 2, 4 }, std::back_inserter(o2));
        static_assert(std::same_as<decltype(dr.in1), std::ranges::dangling>);
        static_assert(std::same_as<decltype(dr.in2), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void merge_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % 257);
            const size_t n2 = static_cast<size_t>(rng() % 257);
            std::vector<T> a(n1), b(n2);
            if ((it & 1u) == 0u) {
                a = rtts::random::vector<T>(n1, rng());
                b = rtts::random::vector<T>(n2, rng());
            }
            else {
                const unsigned alpha = (it % 4 == 1) ? 6u : 16u;
                for (size_t i = 0; i < n1; ++i) a[i] = static_cast<T>(rng() % alpha);
                for (size_t i = 0; i < n2; ++i) b[i] = static_cast<T>(rng() % alpha);
            }

            const bool desc = (it % 3 == 2);
            auto run_check = [&](auto comp) {
                std::ranges::sort(a, comp);
                std::ranges::sort(b, comp);

                std::vector<T> exp(n1 + n2);
                std::ranges::merge(a, b, exp.begin(), comp);

                auto verify = [&](auto fn) {
                    std::vector<T> out(n1 + n2);
                    auto r = fn(out.begin());
                    RTTS_EXPECT(r.in1 == a.end() && r.in2 == b.end() && r.out == out.end());
                    RTTS_ALL_EQUAL(out, exp);
                };

                verify([&](auto dst) { return raze::algorithm::merge(a, b, dst, comp); });
                verify([&](auto dst) { return raze::algorithm::merge(a.begin(), a.end(), b.begin(), b.end(), dst, comp); });
                verify([&](auto dst) { return raze::algorithm::merge[opt::fscalar](a, b, dst, comp); });
                verify([&](auto dst) { return raze::algorithm::merge[opt::fstatic](a, b, dst, comp); });

                if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                    std::list<T> la(a.begin(), a.end());
                    std::list<T> lb(b.begin(), b.end());
                    std::vector<T> lout;
                    raze::algorithm::merge(la, lb, std::back_inserter(lout), comp);
                    RTTS_ALL_EQUAL(lout, exp);
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::merge fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        merge_fuzz<T>(seed, 35);
};

RTTS_CASE("raze::algorithm::merge stability fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % 129);
            const size_t n2 = static_cast<size_t>(rng() % 129);
            std::vector<rtts::test_item> a(n1), b(n2);
            for (size_t i = 0; i < n1; ++i) a[i] = { static_cast<int>(rng() % 10u), static_cast<int>(i) };
            for (size_t i = 0; i < n2; ++i) b[i] = { static_cast<int>(rng() % 10u), static_cast<int>(1000 + i) };
            std::ranges::stable_sort(a, std::less<>{}, &rtts::test_item::key);
            std::ranges::stable_sort(b, std::less<>{}, &rtts::test_item::key);

            std::vector<rtts::test_item> got, exp;
            raze::algorithm::merge(a, b, std::back_inserter(got), std::less<>{},
                                   &rtts::test_item::key, &rtts::test_item::key);
            std::ranges::merge(a, b, std::back_inserter(exp), std::less<>{},
                               &rtts::test_item::key, &rtts::test_item::key);
            RTTS_ALL_EQUAL(got, exp);
        }
    }
};
