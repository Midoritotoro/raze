#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeft.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <string>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_left", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_left(v, std::int64_t{ 5 }, std::plus<>{});
        RTTS_EQUAL(raze::algorithm::fold_left(v, std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_left[opt::fscalar](v.begin(), v.end(), std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_left[opt::fstatic](v, std::int64_t{ 5 }, std::plus<>{}), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::fold_left(a, 0, std::plus<>{}) == 10;
    }());
};

namespace {
    struct fold_box {
        int v = 0;
        constexpr bool operator==(const fold_box&) const = default;
    };
}

RTTS_CASE("raze::algorithm::fold_left order, P2248 braced-init, and single-pass") {
    const std::vector<int> v{ 1, 2, 3 };
    RTTS_EQUAL(raze::algorithm::fold_left(v, 10, std::minus<>{}), 4);

    const std::vector<fold_box> boxes{ { 1 }, { 2 }, { 3 } };
    auto rb = raze::algorithm::fold_left(boxes, { 10 },
        [](fold_box acc, fold_box x) { return fold_box{ acc.v + x.v }; });
    RTTS_EQUAL(rb.v, 16);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::fold_left(sp, 0, std::plus<>{}), 6);
    RTTS_EQUAL(raze::algorithm::fold_left(rtts::make_non_common(v), 1, std::multiplies<>{}), 6);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_left_fuzz(unsigned seed, size_t iters) {
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

            const auto exp_sum = std::ranges::fold_left(sub, init, std::plus<>{});
            RTTS_EQUAL(raze::algorithm::fold_left(sub, init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_left(sub.begin(), sub.end(), init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_left[opt::fscalar](sub, init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_left[opt::fstatic](sub, init, std::plus<>{}), exp_sum);

            auto mix = [](std::int64_t acc, T x) -> std::int64_t {
                return (acc * 3 + static_cast<std::int64_t>(x)) % 1000000007LL;
            };
            const auto exp_mix = std::ranges::fold_left(sub, init, mix);
            RTTS_EQUAL(raze::algorithm::fold_left(sub, init, mix), exp_mix);
            RTTS_EQUAL(raze::algorithm::fold_left[opt::fscalar](sub.begin(), sub.end(), init, mix), exp_mix);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EQUAL(raze::algorithm::fold_left(lst, init, mix), exp_mix);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_left fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_left_fuzz<T>(seed, 40);
};
