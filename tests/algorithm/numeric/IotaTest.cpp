#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Iota.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>
#include <random>

namespace {
    using iota_integral_types = rtts::types<
        char, short, int, long long,
        unsigned char, unsigned short, unsigned int, unsigned long long
    >;
}

RTTS_CASE_TPL("raze::algorithm::iota", iota_integral_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(64) }) {
        std::vector<T> exp(sz), a(sz), b(sz), c(sz);
        auto er = std::ranges::iota(exp, T(1));
        auto ra = raze::algorithm::iota(a, T(1));
        auto rb = raze::algorithm::iota[opt::fscalar](b.begin(), b.end(), T(1));
        auto rc = raze::algorithm::iota[opt::fstatic](c, T(1));

        RTTS_EXPECT(ra.out == a.end() && ra.value == er.value);
        RTTS_EXPECT(rb.out == b.end() && rb.value == er.value);
        RTTS_EXPECT(rc.out == c.end() && rc.value == er.value);
        RTTS_ALL_EQUAL(a, exp);
        RTTS_ALL_EQUAL(b, exp);
        RTTS_ALL_EQUAL(c, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{};
        auto r = raze::algorithm::iota(a, T(10));
        return r.out == a.end() && r.value == T(15)
            && a == std::array<T, 5>{ T(10), T(11), T(12), T(13), T(14) };
    }());
};

RTTS_CASE("raze::algorithm::iota P2248 braced-init, list, non-common, and dangling") {
    struct step_box {
        using difference_type = std::ptrdiff_t;
        int v = 0;
        constexpr step_box& operator++() noexcept { ++v; return *this; }
        constexpr step_box operator++(int) noexcept { auto tmp = *this; ++v; return tmp; }
        constexpr bool operator==(const step_box&) const = default;
    };

    std::vector<step_box> v(5);
    auto r = raze::algorithm::iota(v, { 100 });
    RTTS_EXPECT(r.out == v.end() && r.value.v == 105);
    RTTS_EXPECT(v[0].v == 100 && v[4].v == 104);

    std::list<int> lst(4);
    auto rl = raze::algorithm::iota(rtts::make_non_common(lst), 7);
    RTTS_EXPECT(rl.out == lst.end() && rl.value == 11);
    RTTS_EXPECT((lst == std::list<int>{ 7, 8, 9, 10 }));

    auto dr = raze::algorithm::iota(std::vector<int>(3), 1);
    static_assert(std::same_as<decltype(dr.out), std::ranges::dangling>);
    RTTS_EQUAL(dr.value, 4);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void iota_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 64u : 256u;

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % (max_n + 1));
            std::vector<T> base(n, T(0));
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>(rng() % 50u);

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;

            T start{};
            if constexpr (sizeof(T) == 1 && std::is_signed_v<T>)
                start = static_cast<T>(static_cast<int>(rng() % 111u) - 60);
            else if constexpr (sizeof(T) == 1)
                start = static_cast<T>(rng() % 150u);
            else
                start = static_cast<T>(static_cast<int>(rng() % 1001u) - (std::is_signed_v<T> ? 500 : 0));

            auto exp = base, v1 = base, v2 = base, v3 = base, v4 = base;
            auto re = std::ranges::iota(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                        exp.begin() + static_cast<std::ptrdiff_t>(hi), start);
            auto r1 = raze::algorithm::iota(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v1.begin() + static_cast<std::ptrdiff_t>(hi), start);
            auto r2 = raze::algorithm::iota(std::ranges::subrange(v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                  v2.begin() + static_cast<std::ptrdiff_t>(hi)), start);
            auto r3 = raze::algorithm::iota[opt::fscalar](v3.begin() + static_cast<std::ptrdiff_t>(lo),
                                                          v3.begin() + static_cast<std::ptrdiff_t>(hi), start);
            auto r4 = raze::algorithm::iota[opt::fstatic](v4.begin() + static_cast<std::ptrdiff_t>(lo),
                                                          v4.begin() + static_cast<std::ptrdiff_t>(hi), start);

            RTTS_EXPECT(r1.out - v1.begin() == re.out - exp.begin() && r1.value == re.value);
            RTTS_EXPECT(r2.out - v2.begin() == re.out - exp.begin() && r2.value == re.value);
            RTTS_EXPECT(r3.out - v3.begin() == re.out - exp.begin() && r3.value == re.value);
            RTTS_EXPECT(r4.out - v4.begin() == re.out - exp.begin() && r4.value == re.value);
            RTTS_ALL_EQUAL(v1, exp);
            RTTS_ALL_EQUAL(v2, exp);
            RTTS_ALL_EQUAL(v3, exp);
            RTTS_ALL_EQUAL(v4, exp);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(hi - lo);
                auto rl = raze::algorithm::iota(lst, start);
                RTTS_EXPECT(rl.out == lst.end() && rl.value == re.value);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::iota fuzz", iota_integral_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        iota_fuzz<T>(seed, 40);
};
