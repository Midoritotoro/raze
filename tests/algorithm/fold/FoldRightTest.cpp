#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldRight.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <string>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_right", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_right(v, std::int64_t{ 5 }, std::plus<>{});
        RTTS_EQUAL(raze::algorithm::fold_right(v, std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_right[opt::fscalar](v.begin(), v.end(), std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_right[opt::fstatic](v, std::int64_t{ 5 }, std::plus<>{}), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        auto sub = [](T x, std::int64_t acc) constexpr -> std::int64_t {
            return static_cast<std::int64_t>(x) - acc;
        };
        return raze::algorithm::fold_right(a, std::int64_t{ 10 }, sub) == -8;
    }());
};

RTTS_CASE("raze::algorithm::fold_right right-to-left order, P2248 braced-init, list, and non-common") {
    struct fold_box {
        int v = 0;
    };
    const std::list<int> lst{ 1, 2, 3 };
    RTTS_EQUAL(raze::algorithm::fold_right(lst, 10, std::minus<>{}), -8);
    const std::list<fold_box> blst{ { 1 }, { 2 }, { 3 } };
    auto rb = raze::algorithm::fold_right(blst, { 10 },
        [](fold_box x, fold_box acc) { return fold_box{ x.v + acc.v }; });
    RTTS_EQUAL(rb.v, 16);
    RTTS_EQUAL(raze::algorithm::fold_right(rtts::make_non_common(lst), 10, std::minus<>{}), -8);

    const std::vector<std::string> words{ "a", "b", "c" };
    auto s = raze::algorithm::fold_right(words, std::string("]"),
        [](const std::string& elem, std::string acc) { return elem + acc; });
    RTTS_EQUAL(s, std::string("abc]"));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_right_fuzz(unsigned seed, size_t iters) {
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

            const auto exp_sum = std::ranges::fold_right(sub, init, std::plus<>{});
            RTTS_EQUAL(raze::algorithm::fold_right(sub, init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_right(sub.begin(), sub.end(), init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_right[opt::fscalar](sub, init, std::plus<>{}), exp_sum);
            RTTS_EQUAL(raze::algorithm::fold_right[opt::fstatic](sub, init, std::plus<>{}), exp_sum);

            auto mix = [](T x, std::int64_t acc) -> std::int64_t {
                return (acc * 3 + static_cast<std::int64_t>(x)) % 1000000007LL;
            };
            const auto exp_mix = std::ranges::fold_right(sub, init, mix);
            RTTS_EQUAL(raze::algorithm::fold_right(sub, init, mix), exp_mix);
            RTTS_EQUAL(raze::algorithm::fold_right[opt::fscalar](sub.begin(), sub.end(), init, mix), exp_mix);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EQUAL(raze::algorithm::fold_right(lst, init, mix), exp_mix);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_right fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_right_fuzz<T>(seed, 40);
};
