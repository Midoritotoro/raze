#include <tests/rts/rts.h>
#include <raze/algorithm/remove/Remove.h>

RTTS_CASE_TPL("raze::algorithm::remove", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(1000)();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove(c.first(), c.last(), value); },
            [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove(c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.first(), c.last(), value); },
            [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::unroll<1>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fstatic](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar][raze::options::unroll<2>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fstatic][raze::options::unroll<2>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove(c.first(), c.last(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.first(), c.last(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove(c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::fscalar](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::unroll<2>](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));
};

RTTS_CASE_TPL("raze::algorithm::remove.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(12345)();
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove(c.first(), c.last(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.first(), c.last(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove(c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::unroll<2>](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove(
                c.first(), c.last(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove(
                c.first(), c.last(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove[raze::options::fscalar](
                c.range(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove(
                c.range(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));
};