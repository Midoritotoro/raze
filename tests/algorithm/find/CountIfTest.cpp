#include <tests/rts/rts.h>
#include <raze/algorithm/find/CountIf.h>
#include <src/raze/algorithm/EqualTo.h>

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

    struct count_if_item {
        int32_t key;
        int32_t value;
        bool operator==(const count_if_item&) const = default;
    };

    template <class T>
    struct equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x == target; }
    };

    struct false_pred {
        constexpr bool operator()(auto) const { return false; }
    };

    struct true_pred {
        constexpr bool operator()(auto) const { return true; }
    };

    template <class T>
    void check_count_if_variants(const std::vector<T>& input, T target) {
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };
        auto vec_pred = raze::algorithm::equal_to(target);

        const auto exp = std::ranges::count_if(input, pred);

        RTTS_EXPECT(raze::algorithm::count_if(input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if(input.begin(), input.end(), pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if(input, scalar_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::unroll<2>](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::fstatic](input, pred) == exp);

        RTTS_EXPECT(raze::algorithm::count_if(input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if(input.begin(), input.end(), vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::unroll<2>](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if[raze::options::fstatic](input, vec_pred) == exp);
    }

    template <class T>
    void count_if_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> ones(n, T(1));
            check_count_if_variants(ones, T(0));
            check_count_if_variants(ones, T(1));

            std::vector<T> alt(n);
            for (size_t i = 0; i < n; ++i)
                alt[i] = static_cast<T>(i & 1 ? 0 : 1);
            check_count_if_variants(alt, T(0));

            if (n > 0) {
                auto sparse = ones;
                sparse[0] = T(0);
                sparse[n - 1] = T(0);
                if (n > 16) sparse[16] = T(0);
                if (n > 31) sparse[31] = T(0);
                if (n > 64) sparse[64] = T(0);
                check_count_if_variants(sparse, T(0));
            }
        }
    }

    template <class T>
    void count_if_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            const auto exp = std::ranges::count_if(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::count_if(input, vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[raze::options::fstatic](input, vec_pred) == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const auto sub_exp = std::ranges::count_if(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::count_if(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void count_if_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if(c.first(), c.last(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.first(), c.last(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if(c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.range(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if[raze::options::fscalar](c.first(), c.last(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.first(), c.last(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if[raze::options::unroll<2>](c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.range(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if[raze::options::fstatic](c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.range(), equal_pred<T>{ target });
            });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), false_pred{}); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::count_if(c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::count_if(c.range(), equal_pred<T>{ target });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            return raze::algorithm::count_if(a, true_pred{}) == 0;
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(1), T(4) };
            return raze::algorithm::count_if(a, equal_pred<T>{ T(1) }) == 2 &&
                   raze::algorithm::count_if(a, false_pred{}) == 0;
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::count_if[raze::options::fscalar](a, [](auto x) { return x > T(2); }) == 3;
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(2), T(4), T(2) };
            return raze::algorithm::count_if(a.begin() + 1, a.begin() + 4, equal_pred<T>{ T(2) }) == 2;
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::count_if", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        count_if_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::count_if structs and projections") {
    const std::vector<count_if_item> input{
        { 1, 10 }, { 2, 20 }, { 1, 30 }, { 4, 20 }, { 1, 50 }
    };

    RTTS_EXPECT(raze::algorithm::count_if(
        input,
        [](int32_t k) { return k == 1; },
        [](const auto& item) { return item.key; }) == 3);

    RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 20; },
        [](const auto& item) { return item.value; }) == 2);

    RTTS_EXPECT(raze::algorithm::count_if(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; }) == 0);

    const std::vector<std::string> words{ "aa", "bbb", "cc", "dddd", "ee" };
    RTTS_EXPECT(raze::algorithm::count_if(words, [](const std::string& s) { return s.size() == 2; }) == 3);
};

RTTS_CASE("raze::algorithm::count_if sentinel") {
    {
        std::array<int, 3> empty_arr{ 1, 1, 1 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::count_if(empty_first, std::default_sentinel, raze::algorithm::equal_to(1)) == 0);
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(42)) == 0);
        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(1)) == static_cast<std::ptrdiff_t>(n));

        v[0] = 42;
        v[n / 2] = 42;
        v[n - 1] = 42;
        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(42)) == 3);
    }
};