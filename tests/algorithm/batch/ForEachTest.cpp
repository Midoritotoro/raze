#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEach.h>

RTTS_CASE_TPL("raze::algorithm::for_each", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](T& x) { x += add_val; };
    auto op_vector = [add_val](auto& x) { x += add_val; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::for_each(c.first(), c.last(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each(c.range(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::for_each(c.range(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each(c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::unroll<1>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fstatic](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar][raze::options::unroll<2>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fstatic][raze::options::unroll<2>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fscalar](c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::unroll<2>](c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(1000)();
    auto op = [add_val](auto& x) { x += add_val; };
    auto proj = [](auto& x) -> auto& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.first(), c.last(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fscalar](c.range(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); }));
};