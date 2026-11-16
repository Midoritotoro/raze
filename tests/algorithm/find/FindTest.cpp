#include <tests/rts/rts.h>
#include <raze/algorithm/find/Find.h>

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

    struct find_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_item&) const = default;
    };

    template <class T>
    void check_find_variants(const std::vector<T>& input, T target) {
        auto exp_it = std::ranges::find(input, target);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::find(input, target) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find(input.begin(), input.end(), target) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find[raze::options::fscalar](input, target) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find[raze::options::unroll<2>](input, target) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find[raze::options::fstatic](input, target) - input.begin() == exp_off);
    }

    template <class T>
    void find_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_variants(base, T(0));

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
                    check_find_variants(input, T(0));
                }
            }
        }
    }

    template <class T>
    void find_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto exp_it = std::ranges::find(input, target);

            RTTS_EXPECT(raze::algorithm::find(input, target) == exp_it);
            RTTS_EXPECT(raze::algorithm::find[raze::options::fscalar](input.begin(), input.end(), target) == exp_it);
            RTTS_EXPECT(raze::algorithm::find[raze::options::fstatic](input, target) == exp_it);

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            RTTS_EXPECT(raze::algorithm::find(no_zero, T(0)) == no_zero.end());
            RTTS_EXPECT(raze::algorithm::find[raze::options::fscalar](no_zero, T(0)) == no_zero.end());
            RTTS_EXPECT(raze::algorithm::find[raze::options::fstatic](no_zero, T(0)) == no_zero.end());

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::find(input.begin() + first, input.begin() + last, target);
            RTTS_EXPECT(raze::algorithm::find(input.begin() + first, input.begin() + last, target) == sub_exp);
        }
    }

    template <class T>
    void find_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find(c.first(), c.last(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.first(), c.last(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find(c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.range(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find[raze::options::fscalar](c.first(), c.last(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.first(), c.last(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find[raze::options::unroll<2>](c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.range(), target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find[raze::options::fstatic](c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.range(), target);
            });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find(c.first(), c.last(), T(1)); },
            [](auto c) { return std::ranges::find(c.first(), c.last(), T(1)); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find(c.range(), target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::find(c.range(), target);
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            return raze::algorithm::find(a, T(1)) == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            return raze::algorithm::find(a, T(3)) == a.begin() + 2 &&
                   raze::algorithm::find(a, T(9)) == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::find[raze::options::fscalar](a, T(5)) == a.begin() + 4 &&
                   raze::algorithm::find[raze::options::fscalar](a, T(9)) == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            return raze::algorithm::find(a.begin() + 1, a.begin() + 3, T(4)) == a.begin() + 3;
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find not_found", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t n : { size_t(0), size_t(1), size_t(4), size_t(8), size_t(15), size_t(16), size_t(17),
                      size_t(31), size_t(32), size_t(33), size_t(63), size_t(64), size_t(65),
                      size_t(128), size_t(129), size_t(256), size_t(257), size_t(512), size_t(513) })
    {
        std::vector<T> v(n, T(1));

        RTTS_EXPECT(raze::algorithm::find(v, T(0)) == v.end());
        RTTS_EXPECT(raze::algorithm::find(v.begin(), v.end(), T(0)) == v.end());
        RTTS_EXPECT(raze::algorithm::find[raze::options::fscalar](v, T(0)) == v.end());
        RTTS_EXPECT(raze::algorithm::find[raze::options::unroll<2>](v, T(0)) == v.end());
        RTTS_EXPECT(raze::algorithm::find[raze::options::fstatic](v, T(0)) == v.end());
    }
};

RTTS_CASE_TPL("raze::algorithm::find boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find structs and projections") {
    const std::vector<find_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    RTTS_EXPECT((raze::algorithm::find(input, find_item{ 3, 30 }) == input.begin() + 2));
    RTTS_EXPECT((raze::algorithm::find(input, find_item{ 9, 90 }) == input.end()));

    auto it = raze::algorithm::find(input, 40, [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.begin() + 3);

    it = raze::algorithm::find[raze::options::fscalar](input, 2, [](const auto& item) { return item.key; });
    RTTS_EXPECT(it == input.begin() + 1);

    const std::vector<std::string> words{ "alpha", "beta", "gamma", "delta" };
    RTTS_EXPECT(raze::algorithm::find(words, std::string("gamma")) == words.begin() + 2);
    RTTS_EXPECT(raze::algorithm::find(words, "omega") == words.end());
};

RTTS_CASE("raze::algorithm::find sentinel") {
    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[n - 1] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto res = raze::algorithm::find(first, std::default_sentinel, 42);
        RTTS_EXPECT(res.base() == v.begin() + (n - 1));

        auto miss = raze::algorithm::find(first, std::default_sentinel, 777);
        RTTS_EXPECT(miss == std::default_sentinel);
    }
};