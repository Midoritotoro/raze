#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEachN.h>

template <class It, class Size, class Fn, class Proj = std::identity>
constexpr auto std_for_each_n(It first, Size count, Fn fn, Proj proj = {}) {
#if defined(raze_cpp_msvc_only)
    using Diff = std::iter_difference_t<It>;
    auto n = static_cast<Diff>(count);
    for (; n > 0; --n, (void)++first) {
        std::invoke(fn, std::invoke(proj, *first));
    }
    return std::ranges::for_each_n_result<It, Fn>{std::move(first), std::move(fn)};
#else
    return std::ranges::for_each_n(std::move(first), static_cast<std::iter_difference_t<It>>(count), std::move(fn), std::move(proj));
#endif
}

RTTS_CASE_TPL("raze::algorithm::for_each_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T add = rtts::random::generator<T>(42)();
        auto op = [add](T& x) { x += add; };

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op); },
            [op](auto c) { return std_for_each_n(c.first(), c.count(), op); });
    }

    {
        const T mul = rtts::random::generator<T>(40000)();
        auto op = [mul](T& x) { x *= mul; };

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op); },
            [op](auto c) { return std_for_each_n(c.first(), c.count(), op); });
    }

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [](auto& x) { x += T(1); }); },
        [](auto c) { return std_for_each_n(c.first(), c.count(), [](auto& x) { x += T(1); }); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(10); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(10); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    const T add = rtts::random::generator<T>(100000)();

    auto op = [add](T& x) { x += add; };
    auto proj = [](T& x) -> T& { return x; };

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));
};