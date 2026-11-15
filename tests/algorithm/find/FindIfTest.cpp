#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindIf.h>

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

    struct find_if_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_if_item&) const = default;
    };

    template <class T>
    void check_find_if_variants(const std::vector<T>& input, T target) {
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };

        auto exp_it = std::ranges::find_if(input, pred);
        const auto exp_off = exp_it - input.begin();

        auto it = raze::algorithm::find_if(input, pred);
        RTTS_EXPECT(it - input.begin() == exp_off);

        it = raze::algorithm::find_if(input.begin(), input.end(), pred);
        RTTS_EXPECT(it - input.begin() == exp_off);

        it = raze::algorithm::find_if(input, scalar_pred);
        RTTS_EXPECT(it - input.begin() == exp_off);

        it = raze::algorithm::find_if[raze::options::fscalar](input, pred);
        RTTS_EXPECT(it - input.begin() == exp_off);

        it = raze::algorithm::find_if[raze::options::unroll<2>](input, pred);
        RTTS_EXPECT(it - input.begin() == exp_off);

        it = raze::algorithm::find_if[raze::options::fstatic](input, pred);
        RTTS_EXPECT(it - input.begin() == exp_off);
    }

    template <class T>
    void find_if_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_if_variants(base, T(0));

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
                    check_find_if_variants(input, T(0));
                }
            }
        }
    }

    template <class T>
    void find_if_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto pred = [target](auto x) { return x == target; };
            auto exp_it = std::ranges::find_if(input, pred);

            RTTS_EXPECT(raze::algorithm::find_if(input, pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fscalar](input.begin(), input.end(), pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fstatic](input, pred) == exp_it);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::find_if(input.begin() + first, input.begin() + last, pred);
            RTTS_EXPECT(raze::algorithm::find_if(input.begin() + first, input.begin() + last, pred) == sub_exp);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::find_if", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    auto make_pred = [](auto c) {
        T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        return [target](auto x) { return x == target; };
    };

    run<n, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if(c.first(), c.last(), make_pred(c)); },
        [&](auto c) { return std::ranges::find_if(c.first(), c.last(), make_pred(c)); });

    run<n, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if(c.range(), make_pred(c)); },
        [&](auto c) { return std::ranges::find_if(c.range(), make_pred(c)); });

    run<n, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if[raze::options::fscalar](c.first(), c.last(), make_pred(c)); },
        [&](auto c) { return std::ranges::find_if(c.first(), c.last(), make_pred(c)); });

    run<n, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if[raze::options::unroll<2>](c.range(), make_pred(c)); },
        [&](auto c) { return std::ranges::find_if(c.range(), make_pred(c)); });

    run<n, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if[raze::options::fstatic](c.range(), make_pred(c)); },
        [&](auto c) { return std::ranges::find_if(c.range(), make_pred(c)); });

    auto never = [](auto) { return false; };
    run<range, T>(cfg,
        [&](auto c) { return raze::algorithm::find_if(c.first(), c.last(), never); },
        [&](auto c) { return std::ranges::find_if(c.first(), c.last(), never); });

    static_assert(constexpr_run<T, range, 0>(
        [](auto c) { return raze::algorithm::find_if(c.first(), c.last(), [](auto) { return true; }); },
        [](auto c) { return std::ranges::find_if(c.first(), c.last(), [](auto) { return true; }); }));

    static_assert(constexpr_run<T, n, 5, 3>(
        [](auto c) {
            T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::find_if(c.range(), [target](auto x) { return x == target; });
        },
        [](auto c) {
            T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return std::ranges::find_if(c.range(), [target](auto x) { return x == target; });
        }));

    static_assert([]() constexpr {
        std::array<T, 0> a{};
        return raze::algorithm::find_if(a, [](auto) { return true; }) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::find_if(a, [](auto x) { return x == T(3); }) == a.begin() + 2;
        }());

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        return raze::algorithm::find_if[raze::options::fscalar](a, [](auto x) { return x > T(4); }) == a.begin() + 4;
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::find_if(a.begin() + 1, a.begin() + 3, [](auto x) { return x == T(4); }) == a.begin() + 3;
        }());
};

RTTS_CASE_TPL("raze::algorithm::find_if boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_if_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_if structs and projections") {
    const std::vector<find_if_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    auto it = raze::algorithm::find_if(
        input,
        [](int32_t k) { return k == 3; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(it == input.begin() + 2);

    it = raze::algorithm::find_if[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 50; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.begin() + 4);

    it = raze::algorithm::find_if(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.end());

    const std::vector<std::string> words{ "alpha", "beta", "gamma", "delta" };
    auto wit = raze::algorithm::find_if(words, [](const std::string& s) { return s.size() == 5 && s[0] == 'g'; });
    RTTS_EXPECT(wit == words.begin() + 2);
};

RTTS_CASE("raze::algorithm::find_if bidirectional and forward") {
    const std::list<int> l{ 1, 2, 3, 4, 5 };
    RTTS_EXPECT(raze::algorithm::find_if(l, [](int x) { return x == 4; }) == std::ranges::next(l.begin(), 3));
    RTTS_EXPECT(raze::algorithm::find_if[raze::options::fscalar](l.begin(), l.end(), [](int x) { return x == 99; }) == l.end());

    const std::forward_list<int> fl{ 10, 20, 30, 40, 50 };
    RTTS_EXPECT(raze::algorithm::find_if(fl, [](int x) { return x == 30; }) == std::ranges::next(fl.begin(), 2));
    RTTS_EXPECT(raze::algorithm::find_if[raze::options::fscalar](fl.begin(), fl.end(), [](int x) { return x == 99; }) == fl.end());
};

RTTS_CASE("raze::algorithm::find_if sentinel") {
    std::vector<int> v(300, 1);
    v[299] = 42;

    auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
    auto res = raze::algorithm::find_if(first, std::default_sentinel, [](auto x) { return x == 42; });
    RTTS_EXPECT(res.base() == v.begin() + 299);

    auto miss = raze::algorithm::find_if(first, std::default_sentinel, [](auto x) { return x == 777; });
    RTTS_EXPECT(miss == std::default_sentinel);
};