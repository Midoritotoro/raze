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

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](T& x) { x += add_val; };
    auto op_vector = [add_val](auto& x) { x += add_val; };

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op_scalar); },
            [op_scalar](auto c) { return std_for_each_n(c.first(), c.count(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::unroll<1>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fstatic](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar][raze::options::unroll<2>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fstatic][raze::options::unroll<2>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::unroll<2>](c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(100000)();
    auto op = [add_val](auto& x) { x += add_val; };
    auto proj = [](auto& x) -> auto& { return x; };

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::unroll<2>](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));
};