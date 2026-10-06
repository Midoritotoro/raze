#include <tests/rts/rts.h>
#include <raze/algorithm/transform/Transform.h>

RTTS_CASE_TPL("raze::algorithm::transform.unary", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](const T& x) { return x + add_val; };
    auto op_vector = [add_val](auto x) { return x + add_val; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::transform(c.range(), c.out(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::transform(c.range(), c.out(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform(c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.first(), c.last(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::unroll<1>](c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fstatic](c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar][raze::options::unroll<2>](c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fstatic][raze::options::unroll<2>](c.range(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.out(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x + T(1); }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x + T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::unroll<2>](c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.unary_projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(1000)();
    auto op = [add_val](auto x) { return x + add_val; };
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out(), op, proj); },
        [op, proj](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::transform(c.range(), c.out(), op, proj); },
        [op, proj](auto c) { return std::ranges::transform(c.range(), c.out(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), op, proj); },
        [op, proj](auto c) { return std::ranges::transform(c.range(), c.out(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.binary", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    auto op_scalar = [](const T& x, const T& y) { return x + y; };
    auto op_vector = [](auto x, auto y) { return x + y; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::transform(c.range(), c.range2(), c.out(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform(c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::unroll<1>](c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fstatic](c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fscalar][raze::options::unroll<2>](c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fstatic][raze::options::unroll<2>](c.range(), c.range2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), [](const T& x, const T& y) constexpr { return x + y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), [](const T& x, const T& y) constexpr { return x + y; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::unroll<2>](c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.binary_projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    auto op = [](auto x, auto y) { return x + y; };
    auto proj1 = [](const T& x) -> const T& { return x; };
    auto proj2 = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj1, proj2](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op, proj1, proj2); },
        [op, proj1, proj2](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op, proj1, proj2); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj1, proj2](auto c) { return raze::algorithm::transform(c.range(), c.range2(), c.out(), op, proj1, proj2); },
        [op, proj1, proj2](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op, proj1, proj2); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj1, proj2](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.range2(), c.out(), op, proj1, proj2); },
        [op, proj1, proj2](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op, proj1, proj2); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj1, proj2](auto c) { return raze::algorithm::transform[raze::options::unroll<2>](c.range(), c.range2(), c.out(), op, proj1, proj2); },
        [op, proj1, proj2](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op, proj1, proj2); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out(),
                [](T x, T y) constexpr { return x + y; },
                [](const T& x) constexpr -> const T& { return x; },
                [](const T& y) constexpr -> const T& { return y; }
            );
        },
        [](auto c) constexpr {
            return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out(),
                [](T x, T y) constexpr { return x + y; },
                [](const T& x) constexpr -> const T& { return x; },
                [](const T& y) constexpr -> const T& { return y; }
            );
        }));
};