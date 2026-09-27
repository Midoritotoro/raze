#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopy.h>

RTTS_CASE_TPL("raze::algorithm::replace_copy", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T old_val = rtts::random::generator<T>(42)();
        const T new_val = rtts::random::generator<T>(1000)();

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [old_val, new_val](auto c) { return raze::algorithm::replace_copy(c.first(), c.last(), c.out(), old_val, new_val); },
            [old_val, new_val](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), old_val, new_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [old_val, new_val](auto c) { return raze::algorithm::replace_copy(c.range(), c.out(), old_val, new_val); },
            [old_val, new_val](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val); });
    }

    {
        const T val = rtts::random::generator<T>(777)();

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [val](auto c) { return raze::algorithm::replace_copy(c.first(), c.last(), c.out(), val, val); },
            [val](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), val, val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [val](auto c) { return raze::algorithm::replace_copy(c.range(), c.out(), val, val); },
            [val](auto c) { return std::ranges::replace_copy(c.range(), c.out(), val, val); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::replace_copy(c.first(), c.last(), c.out(), T(2), T(99)); },
        [](auto c) constexpr { return std::ranges::replace_copy(c.first(), c.last(), c.out(), T(2), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::replace_copy(c.range(), c.out(), T(2), T(99)); },
        [](auto c) constexpr { return std::ranges::replace_copy(c.range(), c.out(), T(2), T(99)); }));
};

RTTS_CASE_TPL("raze::algorithm::replace_copy.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T old_val = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [old_val, new_val, proj](auto c) { return raze::algorithm::replace_copy(c.first(), c.last(), c.out(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), old_val, new_val, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [old_val, new_val, proj](auto c) { return raze::algorithm::replace_copy(c.range(), c.out(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy(
                c.first(), c.last(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy(
                c.first(), c.last(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy(
                c.range(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy(
                c.range(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        }));
};