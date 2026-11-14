#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveCopy.h>

RTTS_CASE_TPL("raze::algorithm::remove_copy", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(1000)();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy(c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::fscalar](c.first(), c.last(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::fscalar](c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::unroll<1>](c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::fstatic](c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::fscalar][raze::options::unroll<2>](c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove_copy[raze::options::fstatic][raze::options::unroll<2>](c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.first(), c.last(), c.out(), T(0)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.range(), c.out(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out(), T(0)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy[raze::options::fscalar](c.range(), c.out(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out(), T(0)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy[raze::options::unroll<2>](c.range(), c.out(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out(), T(0)); }));
};

RTTS_CASE_TPL("raze::algorithm::remove_copy.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(12345)();
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove_copy(c.range(), c.out(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove_copy[raze::options::fscalar](c.range(), c.out(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove_copy[raze::options::unroll<2>](c.range(), c.out(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy(
                c.first(), c.last(), c.out(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy(
                c.first(), c.last(), c.out(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy[raze::options::fscalar](
                c.range(), c.out(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy(
                c.range(), c.out(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        }));
};

RTTS_CASE("raze::algorithm::remove_copy.mixed") {
    auto cfg = rtts::algorithm::config::thorough();

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, uint32_t, float>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.first(), c.last(), c.out2(), uint32_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), uint32_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.range(), c.out2(), int64_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), int64_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.range(), c.out2(), int8_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), int8_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, int32_t, int64_t>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.range(), c.out2(), int32_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), int32_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, int16_t, int8_t>(cfg,
        [](auto c) { return raze::algorithm::remove_copy(c.range(), c.out2(), int16_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), int16_t(0)); });

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::remove_copy[raze::options::fscalar](c.first(), c.last(), c.out2(), int32_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); });

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.range(), c.out2(), int64_t(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out2(), int64_t(0)); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int8_t, int16_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.range(), c.out2(), int8_t(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out2(), int8_t(0)); }));
};