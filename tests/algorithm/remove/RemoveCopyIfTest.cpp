#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveCopyIf.h>

RTTS_CASE_TPL("raze::algorithm::remove_copy_if", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(1000)();
    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::fscalar](c.first(), c.last(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::fscalar](c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::unroll<1>](c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::fstatic](c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::fscalar][raze::options::unroll<2>](c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_copy_if[raze::options::fstatic][raze::options::unroll<2>](c.range(), c.out(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if[raze::options::fscalar](c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if[raze::options::unroll<2>](c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out(), [](auto x) constexpr { return x > T(0); }); }));
};

RTTS_CASE_TPL("raze::algorithm::remove_copy_if.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(12345)();
    auto pred = [threshold](auto x) { return x > threshold; };
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_copy_if[raze::options::fscalar](c.range(), c.out(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_copy_if[raze::options::unroll<2>](c.range(), c.out(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy_if(
                c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy_if(
                c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy_if[raze::options::fscalar](
                c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy_if(
                c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        }));
};

RTTS_CASE("raze::algorithm::remove_copy_if.mixed") {
    auto cfg = rtts::algorithm::config::thorough();

    auto pos = [](auto x) { return x > decltype(x)(0); };
    auto even = [](auto x) { return (x & 1) == 0; };

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [pos](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out2(), pos); },
        [pos](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), pos); });

    rtts::algorithm::run<rtts::algorithm::range, uint32_t, float>(cfg,
        [even](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out2(), even); },
        [even](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), even); });

    rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
        [pos](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out2(), pos); },
        [pos](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), pos); });

    rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t>(cfg,
        [even](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out2(), even); },
        [even](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), even); });

    rtts::algorithm::run<rtts::algorithm::range, int32_t, int64_t>(cfg,
        [pos](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out2(), pos); },
        [pos](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), pos); });

    rtts::algorithm::run<rtts::algorithm::range, int16_t, int8_t>(cfg,
        [even](auto c) { return raze::algorithm::remove_copy_if(c.range(), c.out2(), even); },
        [even](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), even); });

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [pos](auto c) { return raze::algorithm::remove_copy_if[raze::options::fscalar](c.first(), c.last(), c.out2(), pos); },
        [pos](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), pos); });

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out2(), [](auto x) constexpr { return x > int32_t(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), [](auto x) constexpr { return x > int32_t(0); }); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int8_t, int16_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); }));
};