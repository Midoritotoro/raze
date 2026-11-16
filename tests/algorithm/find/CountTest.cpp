#include <tests/rts/rts.h>
#include <raze/algorithm/find/Count.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct count_item {
        int32_t key;
        int32_t value;
        bool operator==(const count_item&) const = default;
    };

    template <class T>
    void check_count_variants(const std::vector<T>& input, T target) {
        const auto exp = std::ranges::count(input, target);

        RTTS_EXPECT(raze::algorithm::count(input, target) == exp);
        RTTS_EXPECT(raze::algorithm::count(input.begin(), input.end(), target) == exp);
        RTTS_EXPECT(raze::algorithm::count[raze::options::fscalar](input, target) == exp);
        RTTS_EXPECT(raze::algorithm::count[raze::options::unroll<2>](input, target) == exp);
        RTTS_EXPECT(raze::algorithm::count[raze::options::fstatic](input, target) == exp);
    }

    template <class T>
    void count_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> ones(n, T(1));
            check_count_variants(ones, T(0));
            check_count_variants(ones, T(1));

            std::vector<T> alt(n);
            for (size_t i = 0; i < n; ++i)
                alt[i] = static_cast<T>(i & 1 ? 0 : 1);
            check_count_variants(alt, T(0));

            if (n > 0) {
                auto sparse = ones;
                sparse[0] = T(0);
                sparse[n - 1] = T(0);
                if (n > 16) sparse[16] = T(0);
                if (n > 31) sparse[31] = T(0);
                if (n > 64) sparse[64] = T(0);
                check_count_variants(sparse, T(0));
            }
        }
    }

    template <class T>
    void count_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            const auto exp = std::ranges::count(input, target);

            RTTS_EXPECT(raze::algorithm::count(input, target) == exp);
            RTTS_EXPECT(raze::algorithm::count[raze::options::fscalar](input.begin(), input.end(), target) == exp);
            RTTS_EXPECT(raze::algorithm::count[raze::options::fstatic](input, target) == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const auto sub_exp = std::ranges::count(input.begin() + first, input.begin() + last, target);
            RTTS_EXPECT(raze::algorithm::count(input.begin() + first, input.begin() + last, target) == sub_exp);
        }
    }

    template <class T>
    void count_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count(c.first(), c.last(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.first(), c.last(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count(c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.range(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count[raze::options::fscalar](c.first(), c.last(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.first(), c.last(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count[raze::options::unroll<2>](c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.range(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count[raze::options::fstatic](c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.range(), target);
            });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::count(c.first(), c.last(), T(1)); },
            [](auto c) { return std::ranges::count(c.first(), c.last(), T(1)); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count(c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count(c.range(), target);
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            return raze::algorithm::count(a, T(1)) == 0;
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(1), T(4) };
            return raze::algorithm::count(a, T(1)) == 2 &&
                   raze::algorithm::count(a, T(9)) == 0;
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(5), T(2), T(5), T(4), T(5) };
            return raze::algorithm::count[raze::options::fscalar](a, T(5)) == 3;
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(2), T(4), T(2) };
            return raze::algorithm::count(a.begin() + 1, a.begin() + 4, T(2)) == 2;
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::count", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::count boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::count fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        count_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::count structs and projections") {
    const std::vector<count_item> input{
        { 1, 10 }, { 2, 20 }, { 1, 10 }, { 4, 20 }, { 1, 50 }
    };

    RTTS_EXPECT((raze::algorithm::count(input, count_item{ 1, 10 }) == 2));
    RTTS_EXPECT((raze::algorithm::count(input, count_item{ 9, 90 }) == 0));

    RTTS_EXPECT(raze::algorithm::count(input, 1, [](const auto& item) { return item.key; }) == 3);
    RTTS_EXPECT(raze::algorithm::count[raze::options::fscalar](input, 20, [](const auto& item) { return item.value; }) == 2);
    RTTS_EXPECT(raze::algorithm::count(input, 999, [](const auto& item) { return item.value; }) == 0);

    const std::vector<std::string> words{ "aa", "bb", "aa", "cc", "aa" };
    RTTS_EXPECT(raze::algorithm::count(words, std::string("aa")) == 3);
    RTTS_EXPECT(raze::algorithm::count(words, "omega") == 0);
};

RTTS_CASE("raze::algorithm::count sentinel") {
    {
        std::array<int, 3> empty_arr{ 1, 1, 1 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::count(empty_first, std::default_sentinel, 1) == 0);
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::count(first, std::default_sentinel, 42) == 0);
        RTTS_EXPECT(raze::algorithm::count(first, std::default_sentinel, 1) == static_cast<std::ptrdiff_t>(n));

        v[0] = 42;
        v[n / 2] = 42;
        v[n - 1] = 42;
        RTTS_EXPECT(raze::algorithm::count(first, std::default_sentinel, 42) == 3);
    }
};