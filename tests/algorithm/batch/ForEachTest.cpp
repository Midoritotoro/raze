#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEach.h>

RTTS_CASE_TPL("raze::algorithm::for_each", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T add = rtts::random::generator<T>(42)();
        auto op = [add](T& x) { x += add; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op); },
            [op](auto c) { return std::ranges::for_each(c.first(), c.last(), op); });
    }

    {
        const T mul = rtts::random::generator<T>(40000)();
        auto op = [mul](T& x) { x *= mul; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op); },
            [op](auto c) { return std::ranges::for_each(c.first(), c.last(), op); });
    }

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [](auto c) { return raze::algorithm::for_each(c.first(), c.last(), [](auto& x) { x += T(1); }); },
        [](auto c) { return std::ranges::for_each(c.first(), c.last(), [](auto& x) { x += T(1); }); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(10); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(10); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    const T add = rtts::random::generator<T>(100000)();

    auto op = [add](T& x) { x += add; };
    auto proj = [](T& x) -> T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.first(), c.last(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.first(), c.last(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));
};