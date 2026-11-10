#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyIf.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <vector>


struct tc_struct_4 { int32_t x;           bool operator==(const tc_struct_4&)  const = default; };
struct tc_struct_8 { int64_t x;           bool operator==(const tc_struct_8&)  const = default; };
struct tc_struct_12 { int32_t a, b, c;     bool operator==(const tc_struct_12&) const = default; };

struct tc_nontrivial {
    int32_t x = 0, copies = 0;
    constexpr tc_nontrivial() = default;
    constexpr tc_nontrivial(int32_t v) : x(v) {}
    constexpr tc_nontrivial(const tc_nontrivial&) = default;
    constexpr tc_nontrivial& operator=(const tc_nontrivial& o) { x = o.x; copies = o.copies + 1; return *this; }
    bool operator==(const tc_nontrivial&) const = default;
};
namespace rtts::random {
    template <>
    struct domain<tc_nontrivial> : domain<int32_t> {
        static constexpr domain<tc_nontrivial> standard() noexcept { return {}; }
    };
}
namespace {
    constexpr auto pos = [](auto v) { return v > decltype(v)(0); };
    constexpr auto even = [](auto v) { return v & 1 == 0; };
    constexpr auto yes = [](auto) { return true; };
    constexpr auto no = [](auto) { return false; };
    constexpr auto x_even = [](const auto& v) { return v.x % 2 == 0; };
    constexpr auto x_pos = [](const auto& v) { return v.x > 0; };
    constexpr auto a_even = [](const auto& v) { return v.a % 2 == 0; };
}

RTTS_CASE_TPL("raze::algorithm::copy_if", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::unroll<2>](c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fstatic](c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), yes); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), yes); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), no); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), no); });

    if constexpr (std::is_integral_v<T>) {
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), even); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), even); });
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), pos); }));
};

RTTS_CASE("raze::algorithm::copy_if mixed") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); });

    run<range, uint32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), even); });

    run<range, int64_t, double>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

    run<range, int8_t, int16_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });

    run<range, int32_t, int64_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

    run<range, int16_t, int8_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });

    run<range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); });

    static_assert(constexpr_run_mixed<int32_t, float, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); }));

    static_assert(constexpr_run_mixed<int64_t, double, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out2(), even); }));

    static_assert(constexpr_run_mixed<int8_t, int16_t, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out2(), even); }));
};

RTTS_CASE("raze::algorithm::copy_if structs") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });

    run<range, tc_struct_8>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_even); });

    run<range, tc_struct_12>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), a_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), a_even); });

    run<range, tc_nontrivial>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_even); });

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out(), x_pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_pos); });

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::unroll<2>](c.range(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });

    static_assert(constexpr_run<tc_struct_4, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), x_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), x_even); }));

    static_assert(constexpr_run<tc_struct_8, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_pos); }));

    static_assert(constexpr_run<tc_struct_12, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), a_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), a_even); }));

    static_assert(constexpr_run<tc_nontrivial, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_even); }));
};