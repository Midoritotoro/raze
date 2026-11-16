#include <tests/rts/rts.h>
#include <raze/algorithm/find/NoneOf.h>
#include <src/raze/algorithm/EqualTo.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <forward_list>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct none_of_item {
        int32_t key;
        int32_t value;
        bool operator==(const none_of_item&) const = default;
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
    void check_none_of_variants(const std::vector<T>& input, T target) {
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };
        auto vec_pred = raze::algorithm::equal_to(target);

        const bool exp = std::ranges::none_of(input, pred);

        RTTS_EXPECT(raze::algorithm::none_of(input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of(input.begin(), input.end(), pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of(input, scalar_pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::unroll<2>](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fstatic](input, pred) == exp);

        RTTS_EXPECT(raze::algorithm::none_of(input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of(input.begin(), input.end(), vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::unroll<2>](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fstatic](input, vec_pred) == exp);
    }

    template <class T>
    void none_of_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_none_of_variants(base, T(0));

            if (n > 0) {
                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto input = base;
                    input[pos] = T(0);
                    check_none_of_variants(input, T(0));
                }
            }
        }
    }

    template <class T>
    void none_of_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            const bool exp = std::ranges::none_of(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::none_of(input, vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::none_of[raze::options::fstatic](input, vec_pred) == exp);

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            auto zero_pred = raze::algorithm::equal_to(T(0));
            RTTS_EXPECT(raze::algorithm::none_of(no_zero, zero_pred));
            RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](no_zero, zero_pred));
            RTTS_EXPECT(raze::algorithm::none_of[raze::options::fstatic](no_zero, zero_pred));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const bool sub_exp = std::ranges::none_of(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::none_of(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void none_of_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of(c.first(), c.last(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.first(), c.last(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of(c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.range(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of[raze::options::fscalar](c.first(), c.last(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.first(), c.last(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of[raze::options::unroll<2>](c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.range(), equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of[raze::options::fstatic](c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.range(), equal_pred<T>{ target });
            });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::none_of(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::none_of(c.first(), c.last(), false_pred{}); });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::none_of(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::none_of(c.first(), c.last(), true_pred{}); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::none_of(c.range(), equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::none_of(c.range(), equal_pred<T>{ target });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            return raze::algorithm::none_of(a, true_pred{});
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            return raze::algorithm::none_of(a, equal_pred<T>{ T(9) }) &&
                  !raze::algorithm::none_of(a, equal_pred<T>{ T(3) });
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::none_of[raze::options::fscalar](a, [](auto x) { return x > T(5); }) &&
                  !raze::algorithm::none_of[raze::options::fscalar](a, [](auto x) { return x > T(4); });
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            return raze::algorithm::none_of(a.begin() + 1, a.begin() + 3, equal_pred<T>{ T(4) }) &&
                  !raze::algorithm::none_of(a.begin() + 1, a.begin() + 3, equal_pred<T>{ T(2) });
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::none_of", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    none_of_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::none_of none_match", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t n : { size_t(0), size_t(1), size_t(4), size_t(8), size_t(15), size_t(16), size_t(17),
                      size_t(31), size_t(32), size_t(33), size_t(63), size_t(64), size_t(65),
                      size_t(128), size_t(129), size_t(256), size_t(257), size_t(512), size_t(513) })
    {
        std::vector<T> v(n, T(1));
        auto miss = raze::algorithm::equal_to(T(0));

        RTTS_EXPECT(raze::algorithm::none_of(v, miss));
        RTTS_EXPECT(raze::algorithm::none_of(v.begin(), v.end(), miss));
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](v, miss));
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::unroll<2>](v, miss));
        RTTS_EXPECT(raze::algorithm::none_of[raze::options::fstatic](v, miss));
    }
};

RTTS_CASE_TPL("raze::algorithm::none_of boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    none_of_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::none_of fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        none_of_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::none_of structs and projections") {
    const std::vector<none_of_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    RTTS_EXPECT(raze::algorithm::none_of(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; }));

    RTTS_EXPECT(!raze::algorithm::none_of(
        input,
        [](int32_t k) { return k == 3; },
        [](const auto& item) { return item.key; }));

    RTTS_EXPECT(raze::algorithm::none_of[raze::options::fscalar](
        input,
        [](int32_t v) { return v > 100; },
        [](const auto& item) { return item.value; }));

    RTTS_EXPECT(!raze::algorithm::none_of[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 50; },
        [](const auto& item) { return item.value; }));

    const std::vector<std::string> words{ "alpha", "beta", "gamma", "delta" };
    RTTS_EXPECT(raze::algorithm::none_of(words, [](const std::string& s) { return s == "omega"; }));
    RTTS_EXPECT(!raze::algorithm::none_of(words, [](const std::string& s) { return s.size() == 5 && s[0] == 'g'; }));
};

RTTS_CASE("raze::algorithm::none_of bidirectional and forward") {
    const std::list<int> l{ 1, 2, 3, 4, 5 };
    RTTS_EXPECT(raze::algorithm::none_of(l, [](int x) { return x > 10; }));
    RTTS_EXPECT(!raze::algorithm::none_of[raze::options::fscalar](l.begin(), l.end(), [](int x) { return x == 4; }));

    const std::forward_list<int> fl{ 10, 20, 30, 40, 50 };
    RTTS_EXPECT(raze::algorithm::none_of(fl, [](int x) { return x < 0; }));
    RTTS_EXPECT(!raze::algorithm::none_of[raze::options::fscalar](fl.begin(), fl.end(), [](int x) { return x == 30; }));
};

RTTS_CASE("raze::algorithm::none_of sentinel") {
    {
        std::array<int, 3> empty_arr{ 42, 42, 42 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::none_of(empty_first, std::default_sentinel, raze::algorithm::equal_to(42)));
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::none_of(first, std::default_sentinel, raze::algorithm::equal_to(42)));

        v[0] = 42;
        RTTS_EXPECT(!raze::algorithm::none_of(first, std::default_sentinel, raze::algorithm::equal_to(42)));

        v[0] = 1;
        v[n - 1] = 42;
        RTTS_EXPECT(!raze::algorithm::none_of(first, std::default_sentinel, raze::algorithm::equal_to(42)));
    }
};