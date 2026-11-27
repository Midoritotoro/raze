#include <tests/rts/rts.h>
#include <raze/algorithm/find/LexicographicalCompareThreeWay.h>

#include <algorithm>
#include <array>
#include <compare>
#include <limits>
#include <list>
#include <ranges>
#include <string>
#include <vector>
#include <deque>
#include <random>

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare_three_way", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) {
            return raze::algorithm::lexicographical_compare_three_way(c.first(), c.last(), c.first2(), c.last2());
        },
        [](auto c) {
            return std::lexicographical_compare_three_way(
                c.data.value.begin(), c.data.value.end(),
                c.data.second.begin(), c.data.second.end());
        });

    run<range, T>(cfg,
        [](auto c) {
            return raze::algorithm::lexicographical_compare_three_way[opt::fscalar](c.range(), c.range2());
        },
        [](auto c) {
            return std::lexicographical_compare_three_way(
                c.data.value.begin(), c.data.value.end(),
                c.data.second.begin(), c.data.second.end());
        });

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        std::array<T, 3> b{ T(1), T(2), T(4) };
        std::array<T, 2> p{ T(1), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::lexicographical_compare_three_way(a, b) < 0
            && raze::algorithm::lexicographical_compare_three_way(b, a) > 0
            && raze::algorithm::lexicographical_compare_three_way(a, a) == 0
            && raze::algorithm::lexicographical_compare_three_way(p, a) < 0
            && raze::algorithm::lexicographical_compare_three_way(a, p) > 0
            && raze::algorithm::lexicographical_compare_three_way(e, e) == 0;
    }());
};

RTTS_CASE("raze::algorithm::lexicographical_compare_three_way categories, NaN, and projections") {
    {
        const std::vector<double> a{ 1.0, std::numeric_limits<double>::quiet_NaN() };
        const std::vector<double> b{ 1.0, 2.0 };
        auto cmp = raze::algorithm::lexicographical_compare_three_way(a, b);
        static_assert(std::same_as<decltype(cmp), std::partial_ordering>);
        RTTS_EXPECT(cmp == std::partial_ordering::unordered);
    }

    {
        const std::vector<rtts::test_item> a{ { 1, 100 }, { 2, 200 } };
        const std::vector<rtts::test_item> b{ { 1, 1 }, { 2, 2 } };
        auto cmp = raze::algorithm::lexicographical_compare_three_way(
            a, b, std::compare_three_way{}, &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EXPECT(cmp == std::strong_ordering::equal);

        const std::vector<int> v1{ 1, 2, 3 };
        const std::vector<int> v2{ 1, 2, 4 };
        rtts::single_pass_input_range<int> sp1(v1);
        rtts::single_pass_input_range<int> sp2(v2);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(sp1, sp2) == std::strong_ordering::less);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(
            rtts::make_non_common(v2), rtts::make_non_common(v1)) == std::strong_ordering::greater);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void lexicographical_compare_three_way_fuzz(unsigned seed, size_t iters) {
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

            const auto exp = std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
            RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(a, b) == exp);
            RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end()) == exp);
            RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way[opt::fscalar](a, b) == exp);
            RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way[opt::fstatic](a, b) == exp);

            if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                std::list<T> la(a.begin(), a.end());
                std::deque<T> db(b.begin(), b.end());
                RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(la, db) == exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare_three_way fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        lexicographical_compare_three_way_fuzz<T>(seed, 40);
};
