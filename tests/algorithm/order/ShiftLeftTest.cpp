#include <tests/rts/rts.h>
#include <raze/algorithm/order/ShiftLeft.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::shift_left", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(5), size_t(16), size_t(65) }) {
        std::vector<T> base(sz);
        for (size_t i = 0; i < sz; ++i)
            base[i] = static_cast<T>((i % 97) + 1);

        for (std::ptrdiff_t sh : { std::ptrdiff_t(0), std::ptrdiff_t(1), std::ptrdiff_t(2),
            static_cast<std::ptrdiff_t>(sz / 2), static_cast<std::ptrdiff_t>(sz), static_cast<std::ptrdiff_t>(sz + 3) })
        {
            auto exp = base;
            auto exp_end = std::shift_left(exp.begin(), exp.end(), sh);
            const auto exp_len = exp_end - exp.begin();

            auto check = [&](auto fn) {
                auto v = base;
                auto r = fn(v, sh);
                RTTS_EXPECT(r.begin() == v.begin());
                RTTS_EXPECT(r.end() - v.begin() == exp_len);
                RTTS_EXPECT(std::ranges::equal(r, std::ranges::subrange(exp.begin(), exp_end)));
            };

            check([](auto& v, auto n) { return raze::algorithm::shift_left(v, n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_left(v.begin(), v.end(), n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_left[opt::fscalar](v, n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_left[opt::fstatic](v, n); });
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        auto r = raze::algorithm::shift_left(a, 2);
        return r.begin() == a.begin() && r.end() == a.begin() + 3
            && a[0] == T(3) && a[1] == T(4) && a[2] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::shift_left forward_list, move-only, non-common, and dangling") {
    {
        std::forward_list<int> fl{ 10, 20, 30, 40, 50 };
        auto r = raze::algorithm::shift_left(fl, 2);
        std::vector<int> got(r.begin(), r.end());
        RTTS_EXPECT((got == std::vector<int>{ 30, 40, 50 }));
    }

    {
        std::vector<std::unique_ptr<int>> v;
        for (int i = 1; i <= 5; ++i)
            v.push_back(std::make_unique<int>(i));
        auto r = raze::algorithm::shift_left(v, 2);
        RTTS_EQUAL(r.size(), size_t(3));
        RTTS_EXPECT(*v[0] == 3 && *v[1] == 4 && *v[2] == 5);
    }

    {
        std::vector<int> v{ 1, 2, 3, 4 };
        auto r = raze::algorithm::shift_left(rtts::make_non_common(v), 1);
        RTTS_EXPECT((std::vector<int>(r.begin(), r.end()) == std::vector<int>{ 2, 3, 4 }));

        auto dr = raze::algorithm::shift_left(std::vector<int>{ 1, 2, 3 }, 1);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void shift_left_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            const std::ptrdiff_t sh = static_cast<std::ptrdiff_t>(rng() % (len + 6));

            auto exp = base;
            auto exp_end = std::shift_left(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                           exp.begin() + static_cast<std::ptrdiff_t>(hi), sh);
            const auto exp_len = exp_end - (exp.begin() + static_cast<std::ptrdiff_t>(lo));

            auto check = [&](auto fn) {
                auto v = base;
                auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                 v.begin() + static_cast<std::ptrdiff_t>(hi));
                auto r = fn(sub, sh);
                RTTS_EXPECT(r.begin() == sub.begin());
                RTTS_EXPECT(r.end() - sub.begin() == exp_len);
                RTTS_EXPECT(std::ranges::equal(r, std::ranges::subrange(
                    exp.begin() + static_cast<std::ptrdiff_t>(lo), exp_end)));
                RTTS_EXPECT(std::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
                RTTS_EXPECT(std::equal(v.begin() + static_cast<std::ptrdiff_t>(hi), v.end(),
                                       base.begin() + static_cast<std::ptrdiff_t>(hi)));
            };

            check([](auto sub, auto s) { return raze::algorithm::shift_left(sub, s); });
            check([](auto sub, auto s) { return raze::algorithm::shift_left(sub.begin(), sub.end(), s); });
            check([](auto sub, auto s) { return raze::algorithm::shift_left[opt::fscalar](sub, s); });
            check([](auto sub, auto s) { return raze::algorithm::shift_left[opt::fstatic](sub, s); });

            if (it % 8 == 0 && len <= 64) {
                std::forward_list<T> fl(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                        base.begin() + static_cast<std::ptrdiff_t>(hi));
                auto r = raze::algorithm::shift_left(fl, sh);
                RTTS_EXPECT(std::ranges::equal(r, std::ranges::subrange(
                    exp.begin() + static_cast<std::ptrdiff_t>(lo), exp_end)));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::shift_left fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        shift_left_fuzz<T>(seed, 40);
};
