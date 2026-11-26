#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Iota.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>

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

