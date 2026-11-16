#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindIfNot.h>
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

    struct find_if_not_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_if_not_item&) const = default;
    };

    template <class T>
    struct not_equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x != target; }
    };

    template <class T>
    struct eq_pred {
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
    void check_find_if_not_variants(const std::vector<T>& input, T keep) {
        auto pred = [keep](auto x) { return x != keep; };
        auto scalar_pred = [keep](T x) { return x != keep; };

        auto exp_it = std::ranges::find_if_not(input, pred);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::find_if_not(input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input.begin(), input.end(), pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input, scalar_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::unroll<2>](input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, pred) - input.begin() == exp_off);
    }

    template <class T>
    void check_find_if_not_vec_variants(const std::vector<T>& input, T keep) {
        auto vec_pred = raze::algorithm::equal_to(keep);

        auto exp_it = std::ranges::find_if_not(input, vec_pred);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::find_if_not(input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input.begin(), input.end(), vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::unroll<2>](input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, vec_pred) - input.begin() == exp_off);
    }

    template <class T>
    void find_if_not_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_if_not_variants(base, T(0));

            std::vector<T> all_ones(n, T(1));
            check_find_if_not_vec_variants(all_ones, T(1));

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
                    check_find_if_not_variants(input, T(0));

                    auto vec_input = all_ones;
                    vec_input[pos] = T(0);
                    check_find_if_not_vec_variants(vec_input, T(1));
                }
            }
        }
    }

    template <class T>
    void find_if_not_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T keep = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(keep);
            auto exp_it = std::ranges::find_if_not(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::find_if_not(input, vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, vec_pred) == exp_it);

            // Гарантированный промах (все элементы == keep)
            std::vector<T> all_keep(n, keep);
            RTTS_EXPECT(raze::algorithm::find_if_not(all_keep, vec_pred) == all_keep.end());
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](all_keep, vec_pred) == all_keep.end());
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](all_keep, vec_pred) == all_keep.end());

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::find_if_not(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::find_if_not(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void find_if_not_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not(c.first(), c.last(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.first(), c.last(), not_equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not(c.range(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not[raze::options::fscalar](c.first(), c.last(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.first(), c.last(), not_equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not[raze::options::unroll<2>](c.range(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target });
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not[raze::options::fstatic](c.range(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target });
            });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::find_if_not(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::find_if_not(c.first(), c.last(), true_pred{}); });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_if_not(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::find_if_not(c.first(), c.last(), false_pred{}); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_if_not(c.range(), not_equal_pred<T>{ target });
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            return raze::algorithm::find_if_not(a, false_pred{}) == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(1), T(3), T(1) };
            return raze::algorithm::find_if_not(a, eq_pred<T>{ T(1) }) == a.begin() + 2 &&
                   raze::algorithm::find_if_not(a, true_pred{}) == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::find_if_not[raze::options::fscalar](a, [](auto x) { return x <= T(4); }) == a.begin() + 4;
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(1), T(1), T(4) };
            return raze::algorithm::find_if_not(a.begin() + 1, a.begin() + 3, eq_pred<T>{ T(1) }) == a.begin() + 3;
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_if_not", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_not_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if_not not_found", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t n : { size_t(0), size_t(1), size_t(4), size_t(8), size_t(15), size_t(16), size_t(17),
                      size_t(31), size_t(32), size_t(33), size_t(63), size_t(64), size_t(65),
                      size_t(128), size_t(129), size_t(256), size_t(257), size_t(512), size_t(513) })
    {
        std::vector<T> v(n, T(1));
        auto keep_one = raze::algorithm::equal_to(T(1));

        RTTS_EXPECT(raze::algorithm::find_if_not(v, keep_one) == v.end());
        RTTS_EXPECT(raze::algorithm::find_if_not(v.begin(), v.end(), keep_one) == v.end());
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](v, keep_one) == v.end());
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::unroll<2>](v, keep_one) == v.end());
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](v, keep_one) == v.end());
    }
};

RTTS_CASE_TPL("raze::algorithm::find_if_not boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_not_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if_not fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_if_not_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_if_not structs and projections") {
    const std::vector<find_if_not_item> input{
        { 1, 10 }, { 2, 10 }, { 3, 30 }, { 4, 10 }
    };

    auto it = raze::algorithm::find_if_not(
        input,
        [](int32_t v) { return v == 10; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.begin() + 2);

    it = raze::algorithm::find_if_not[raze::options::fscalar](
        input,
        [](int32_t k) { return k < 4; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(it == input.begin() + 3);

    const std::vector<std::string> words{ "aa", "bb", "gamma", "cc" };
    auto wit = raze::algorithm::find_if_not(words, [](const std::string& s) { return s.size() == 2; });
    RTTS_EXPECT(wit == words.begin() + 2);
};

RTTS_CASE("raze::algorithm::find_if_not bidirectional and forward") {
    const std::list<int> l{ 1, 1, 1, 4, 1 };
    RTTS_EXPECT(raze::algorithm::find_if_not(l, [](int x) { return x == 1; }) == std::ranges::next(l.begin(), 3));
    RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](l.begin(), l.end(), [](int x) { return x > 0; }) == l.end());

    const std::forward_list<int> fl{ 10, 10, 30, 10, 10 };
    RTTS_EXPECT(raze::algorithm::find_if_not(fl, [](int x) { return x == 10; }) == std::ranges::next(fl.begin(), 2));
    RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](fl.begin(), fl.end(), [](int x) { return x > 0; }) == fl.end());
};

RTTS_CASE("raze::algorithm::find_if_not sentinel") {
    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[n - 1] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto res = raze::algorithm::find_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(res.base() == v.begin() + (n - 1));

        std::vector<int> all_ones(n, 1);
        auto miss_first = std::counted_iterator(all_ones.begin(), static_cast<std::ptrdiff_t>(all_ones.size()));
        auto miss = raze::algorithm::find_if_not(miss_first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(miss == std::default_sentinel);
    }
};