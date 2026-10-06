#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveIf.h>

RTTS_CASE_TPL("raze::algorithm::remove_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(1000)();
    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_if(c.range(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_if(c.range(), pred_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if(c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.first(), c.last(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::unroll<1>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fstatic](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar][raze::options::unroll<2>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fstatic][raze::options::unroll<2>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::unroll<2>](c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));
};

RTTS_CASE_TPL("raze::algorithm::remove_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(12345)();
    auto pred = [threshold](auto x) { return x > threshold; };
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if(c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::unroll<2>](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_if(
                c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_if(
                c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_if[raze::options::fscalar](
                c.range(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_if(
                c.range(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));
};