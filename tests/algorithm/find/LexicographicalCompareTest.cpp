#include <tests/rts/rts.h>
#include <raze/algorithm/find/LexicographicalCompare.h>

#include <algorithm>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <ranges>
#include <string>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.first(), c.last(), c.first2(), c.last2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare(c.range(), c.range2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare[opt::fscalar](c.range(), c.range2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare[opt::fstatic](c.range(), c.range2(), std::greater<>{}); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2(), std::greater<>{}); });

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        std::array<T, 3> b{ T(1), T(2), T(4) };
        std::array<T, 2> p{ T(1), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::lexicographical_compare(a, b)
            && !raze::algorithm::lexicographical_compare(b, a)
            && !raze::algorithm::lexicographical_compare(a, a)
            && raze::algorithm::lexicographical_compare(p, a)
            && !raze::algorithm::lexicographical_compare(a, p)
            && raze::algorithm::lexicographical_compare(e, p)
            && !raze::algorithm::lexicographical_compare(e, e);
    }());
};

RTTS_CASE("raze::algorithm::lexicographical_compare projections, views, and edge cases") {
    namespace views = std::views;

    {
        const std::vector<rtts::test_item> a{ { 1, 99 }, { 2, 88 } };
        const std::vector<rtts::test_item> b{ { 1, 1 }, { 3, 2 } };
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            a, b, std::less<>{}, &rtts::test_item::key, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::lexicographical_compare(
            a, b, std::less<>{}, &rtts::test_item::payload, &rtts::test_item::payload));
    }

    {
        const std::vector<int> v1{ 1, 2, 3 };
        const std::vector<int> v2{ 1, 2, 3, 4 };
        rtts::single_pass_input_range<int> sp1(v1);
        rtts::single_pass_input_range<int> sp2(v2);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(sp1, sp2));

        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            rtts::make_non_common(v1), rtts::make_non_common(v2)));
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            views::iota(0, 5), views::iota(0, 6)));
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            std::string("apple"), std::string("apricot")));
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void lexicographical_compare_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % 257);
            const size_t n2 = static_cast<size_t>(rng() % 257);
            std::vector<T> a, b;

            if ((it & 1u) == 0u) {
                const size_t common = (n1 < n2) ? n1 : n2;
                a = rtts::random::vector<T>(n1, rng());
                b.resize(n2);
                for (size_t i = 0; i < common; ++i)
                    b[i] = a[i];
                for (size_t i = common; i < n2; ++i)
                    b[i] = val_gen();
                if (common > 0 && (rng() % 3u) != 0u) {
                    const size_t pos = static_cast<size_t>(rng() % common);
                    b[pos] = val_gen();
                }
            }
            else {
                a.resize(n1);
                b.resize(n2);
                for (size_t i = 0; i < n1; ++i) a[i] = static_cast<T>(rng() % 4u);
                for (size_t i = 0; i < n2; ++i) b[i] = static_cast<T>(rng() % 4u);
            }

            const bool exp_lt = std::ranges::lexicographical_compare(a, b);
            RTTS_EQUAL(raze::algorithm::lexicographical_compare(a, b), exp_lt);
            RTTS_EQUAL(raze::algorithm::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end()), exp_lt);
            RTTS_EQUAL(raze::algorithm::lexicographical_compare[opt::fscalar](a, b), exp_lt);
            RTTS_EQUAL(raze::algorithm::lexicographical_compare[opt::fstatic](a, b), exp_lt);

            const bool exp_gt = std::ranges::lexicographical_compare(a, b, std::greater<>{});
            RTTS_EQUAL(raze::algorithm::lexicographical_compare(a, b, std::greater<>{}), exp_gt);
            RTTS_EQUAL(raze::algorithm::lexicographical_compare[opt::fscalar](a, b, std::greater<>{}), exp_gt);

            if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                std::list<T> la(a.begin(), a.end());
                std::deque<T> db(b.begin(), b.end());
                RTTS_EQUAL(raze::algorithm::lexicographical_compare(la, db), exp_lt);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        lexicographical_compare_fuzz<T>(seed, 40);
};
