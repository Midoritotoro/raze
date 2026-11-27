#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftWithIter.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <string>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_left_with_iter", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_left_with_iter(v, std::int64_t{ 10 }, std::plus<>{});
        auto r1 = raze::algorithm::fold_left_with_iter(v, std::int64_t{ 10 }, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_with_iter[opt::fscalar](v.begin(), v.end(), std::int64_t{ 10 }, std::plus<>{});
        auto r3 = raze::algorithm::fold_left_with_iter[opt::fstatic](v, std::int64_t{ 10 }, std::plus<>{});

        RTTS_EXPECT(r1.in == v.end() && r1.value == exp.value);
        RTTS_EXPECT(r2.in == v.end() && r2.value == exp.value);
        RTTS_EXPECT(r3.in == v.end() && r3.value == exp.value);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        auto r = raze::algorithm::fold_left_with_iter(a, 0, std::plus<>{});
        return r.in == a.end() && r.value == 10;
    }());
};

RTTS_CASE("raze::algorithm::fold_left_with_iter order, P2248 braced-init, single-pass, and dangling") {
    {
        const std::vector<int> v{ 1, 2, 3 };
        auto r = raze::algorithm::fold_left_with_iter(v, 10, std::minus<>{});
        RTTS_EXPECT(r.in == v.end() && r.value == 4);
    }

    {
        struct fold_box {
            int v = 0;
        };
        const std::vector<fold_box> v{ { 1 }, { 2 }, { 3 }, { 4 } };
        auto r = raze::algorithm::fold_left_with_iter(v, { 5 },
            [](fold_box acc, fold_box x) { return fold_box{ acc.v + x.v }; });
        RTTS_EXPECT(r.in == v.end() && r.value.v == 15);
    }

    {
        const std::vector<std::string> words{ "a", "b", "c" };
        rtts::single_pass_input_range<std::string> sp(words);
        auto r = raze::algorithm::fold_left_with_iter(sp, std::string("["),
            [](std::string acc, const std::string& s) { return acc + s; });
        RTTS_EXPECT(r.in == sp.end() && r.value == "[abc");

        auto dr = raze::algorithm::fold_left_with_iter(std::vector<int>{ 1, 2, 3 }, 0, std::plus<>{});
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EQUAL(dr.value, 6);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_left_with_iter_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> v(n);
            for (size_t i = 0; i < n; ++i) {
                const int val = static_cast<int>(rng() % 31u) - (std::is_signed_v<T> ? 15 : 0);
                v[i] = static_cast<T>(val);
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));
            const std::int64_t init = static_cast<std::int64_t>(rng() % 101u) - 50;

            const auto exp = std::ranges::fold_left_with_iter(sub, init, std::plus<>{});
            auto r1 = raze::algorithm::fold_left_with_iter(sub, init, std::plus<>{});
            auto r2 = raze::algorithm::fold_left_with_iter(sub.begin(), sub.end(), init, std::plus<>{});
            auto r3 = raze::algorithm::fold_left_with_iter[opt::fscalar](sub, init, std::plus<>{});
            auto r4 = raze::algorithm::fold_left_with_iter[opt::fstatic](sub, init, std::plus<>{});

            RTTS_EXPECT(r1.in == exp.in && r1.value == exp.value);
            RTTS_EXPECT(r2.in == exp.in && r2.value == exp.value);
            RTTS_EXPECT(r3.in == exp.in && r3.value == exp.value);
            RTTS_EXPECT(r4.in == exp.in && r4.value == exp.value);

            auto mix = [](std::int64_t acc, T x) -> std::int64_t {
                return (acc * 3 + static_cast<std::int64_t>(x)) % 1000000007LL;
            };
            const auto exp_m = std::ranges::fold_left_with_iter(sub, init, mix);
            auto rm = raze::algorithm::fold_left_with_iter(sub, init, mix);
            RTTS_EXPECT(rm.in == exp_m.in && rm.value == exp_m.value);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                auto rl = raze::algorithm::fold_left_with_iter(lst, init, mix);
                RTTS_EXPECT(rl.in == lst.end() && rl.value == exp_m.value);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_left_with_iter fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_left_with_iter_fuzz<T>(seed, 40);
};
