#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveIf.h>

RTTS_CASE_TPL("raze::algorithm::remove_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T threshold = rtts::random::generator<T>(1000)();
        auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        auto pred_vector = [threshold](auto x) { return x > threshold; };

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
        const T target = rtts::random::generator<T>(4000)();
        auto pred_scalar = [target](const T& x) { return x == target; };
        auto pred_vector = [target](auto x) { return x == target; };

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
        auto false_scalar = [](const T&) { return false; };
        auto false_vector = [](auto) { return false; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [false_scalar](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), false_scalar); },
            [false_scalar](auto c) { return std::ranges::remove_if(c.first(), c.last(), false_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [false_vector](auto c) { return raze::algorithm::remove_if(c.range(), false_vector); },
            [false_vector](auto c) { return std::ranges::remove_if(c.range(), false_vector); },
            rtts::algorithm::prefix_result);
    }

    {
        auto true_scalar = [](const T&) { return true; };
        auto true_vector = [](auto) { return true; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [true_scalar](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), true_scalar); },
            [true_scalar](auto c) { return std::ranges::remove_if(c.first(), c.last(), true_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [true_vector](auto c) { return raze::algorithm::remove_if(c.range(), true_vector); },
            [true_vector](auto c) { return std::ranges::remove_if(c.range(), true_vector); },
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
};

RTTS_CASE_TPL("raze::algorithm::remove_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(12345)();
    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred_scalar, proj](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred_scalar, proj); },
        [pred_scalar, proj](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_scalar, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred_vector, proj](auto c) { return raze::algorithm::remove_if(c.range(), pred_vector, proj); },
        [pred_vector, proj](auto c) { return std::ranges::remove_if(c.range(), pred_vector, proj); },
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
            return raze::algorithm::remove_if(
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