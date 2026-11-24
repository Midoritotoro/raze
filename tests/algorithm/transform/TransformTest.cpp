#include <tests/rts/rts.h>
#include <raze/algorithm/transform/Transform.h>
#include <bit>
#include <cstdint>

RTTS_CASE_TPL("raze::algorithm::transform.unary", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](const T& x) { return x + add_val; };
    auto op_vector = [add_val](auto x) { return x + add_val; };
    auto op_branching = [add_val](auto x) {
        if (x > add_val) return static_cast<decltype(x)>(x + add_val);
        return x;
    };

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

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.first(), c.last(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.out(), op_branching); },
            [op_branching](auto c) { return std::ranges::transform(c.range(), c.out(), op_branching); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::transform[raze::options::fautovec][raze::options::fstatic](c.range(), c.out(), op_branching); },
            [op_branching](auto c) { return std::ranges::transform(c.range(), c.out(), op_branching); });
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo, auto fn) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, fn](auto c) { return algo(c.first(), c.last(), c.out(), fn); },
                [fn](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), fn); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, fn](auto c) { return algo(c.range(), c.out(), fn); },
                [fn](auto c) { return std::ranges::transform(c.range(), c.out(), fn); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::transform[opt::aligned<16>], op_vector);
        check_aligned.template operator()<32, 32>(raze::algorithm::transform[opt::aligned<32>], op_vector);
        check_aligned.template operator()<32, 32>(raze::algorithm::transform[opt::aligned<32>][opt::unroll<2>], op_vector);
        check_aligned.template operator()<64, 64>(raze::algorithm::transform[opt::aligned<64>], op_vector);
        check_aligned.template operator()<64, 64>(raze::algorithm::transform[opt::aligned<64>], op_scalar);
        check_aligned.template operator()<64, 64>(raze::algorithm::transform[opt::aligned<64>][opt::fautovec], op_branching);
        check_aligned.template operator()<64, 64>(raze::algorithm::transform[opt::aligned<64>][opt::fscalar], op_vector);
        check_aligned.template operator()<1, 1>(raze::algorithm::transform[opt::aligned<opt::unaligned>], op_vector);
        check_aligned.template operator()<1, 1>(raze::algorithm::transform[opt::aligned<64>][opt::aligned<opt::unaligned>], op_vector);
        check_aligned.template operator()<64, 1>(raze::algorithm::transform[opt::aligned<64, opt::unaligned>], op_vector);
        check_aligned.template operator()<1, 64>(raze::algorithm::transform[opt::aligned<opt::unaligned, 64>], op_vector);
        check_aligned.template operator()<32, 64>(raze::algorithm::transform[opt::aligned<32, 64>][opt::fstatic], op_vector);
        check_aligned.template operator()<64, 16>(raze::algorithm::transform[opt::aligned<64, 16>], op_vector);
        check_aligned.template operator()<1, 32>(raze::algorithm::transform[opt::aligned<64>][opt::aligned<opt::unaligned, 32>][opt::fautovec], op_branching);
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

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.out(), [](auto x) constexpr { if (x > T(2)) return static_cast<decltype(x)>(x + T(1)); return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { if (x > T(2)) return static_cast<decltype(x)>(x + T(1)); return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { return x + T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.unary_projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T add_val = rtts::random::generator<T>(1000)();
    auto op = [add_val](auto x) { return x + add_val; };
    auto op_branching = [add_val](auto x) {
        if (x > add_val) return static_cast<decltype(x)>(x + add_val);
        return x;
    };
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

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op_branching, proj](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.out(), op_branching, proj); },
        [op_branching, proj](auto c) { return std::ranges::transform(c.range(), c.out(), op_branching, proj); });

    rtts::algorithm::run_aligned<64, 1, rtts::algorithm::range, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), op, proj); },
        [op, proj](auto c) { return std::ranges::transform(c.range(), c.out(), op, proj); });

    rtts::algorithm::run_aligned<1, 64, rtts::algorithm::range, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::transform[raze::options::aligned<raze::options::unaligned, 64>](c.range(), c.out(), op, proj); },
        [op, proj](auto c) { return std::ranges::transform(c.range(), c.out(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](T x) constexpr { return x + T(5); }, [](const T& x) constexpr -> const T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fautovec][raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), [](auto x) constexpr { if (x > T(2)) return static_cast<decltype(x)>(x + T(5)); return x; }, [](const T& x) constexpr -> const T& { return x; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.out(), [](auto x) constexpr { if (x > T(2)) return static_cast<decltype(x)>(x + T(5)); return x; }, [](const T& x) constexpr -> const T& { return x; }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.binary", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    auto op_scalar = [](const T& x, const T& y) { return x + y; };
    auto op_vector = [](auto x, auto y) { return x + y; };
    auto op_branching = [](auto x, auto y) {
        if (x > y) return static_cast<decltype(x)>(x + y);
        return y;
    };

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

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); },
            [op_vector](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.range2(), c.out(), op_branching); },
            [op_branching](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_branching); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::transform[raze::options::fautovec][raze::options::fstatic](c.range(), c.range2(), c.out(), op_branching); },
            [op_branching](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_branching); });
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t A1, size_t A2, size_t AOut>(auto algo, auto fn) {
            rtts::algorithm::run_aligned<A1, A2, AOut, rtts::algorithm::range, T>(acfg,
                [algo, fn](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2(), c.out(), fn); },
                [fn](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), fn); });

            rtts::algorithm::run_aligned<A1, A2, AOut, rtts::algorithm::range, T>(acfg,
                [algo, fn](auto c) { return algo(c.range(), c.range2(), c.out(), fn); },
                [fn](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), fn); });
        };

        check_aligned.template operator()<16, 16, 16>(raze::algorithm::transform[opt::aligned<16>], op_vector);
        check_aligned.template operator()<32, 32, 32>(raze::algorithm::transform[opt::aligned<32>], op_vector);
        check_aligned.template operator()<64, 64, 64>(raze::algorithm::transform[opt::aligned<64>], op_vector);
        check_aligned.template operator()<64, 64, 64>(raze::algorithm::transform[opt::aligned<64>], op_scalar);
        check_aligned.template operator()<64, 64, 64>(raze::algorithm::transform[opt::aligned<64>][opt::fautovec], op_branching);
        check_aligned.template operator()<64, 64, 64>(raze::algorithm::transform[opt::aligned<64>][opt::fscalar], op_vector);
        check_aligned.template operator()<1, 1, 1>(raze::algorithm::transform[opt::aligned<opt::unaligned>], op_vector);
        check_aligned.template operator()<1, 1, 1>(raze::algorithm::transform[opt::aligned<64>][opt::aligned<opt::unaligned>], op_vector);
        check_aligned.template operator()<64, 1, 32>(raze::algorithm::transform[opt::aligned<64, opt::unaligned, 32>], op_vector);
        check_aligned.template operator()<1, 64, 1>(raze::algorithm::transform[opt::aligned<opt::unaligned, 64, opt::unaligned>], op_vector);
        check_aligned.template operator()<1, 1, 64>(raze::algorithm::transform[opt::aligned<opt::unaligned, opt::unaligned, 64>], op_vector);
        check_aligned.template operator()<32, 64, 16>(raze::algorithm::transform[opt::aligned<32, 64, 16>][opt::unroll<2>], op_vector);
        check_aligned.template operator()<64, 1, 64>(raze::algorithm::transform[opt::aligned<64, opt::unaligned, 64>][opt::fautovec], op_branching);
        check_aligned.template operator()<1, 32, 1>(raze::algorithm::transform[opt::aligned<64>][opt::aligned<opt::unaligned, 32, opt::unaligned>][opt::fstatic], op_vector);
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

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { if (x > y) return static_cast<decltype(x)>(x + y); return y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { if (x > y) return static_cast<decltype(x)>(x + y); return y; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned, 32>](c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); },
        [](auto c) constexpr { return std::ranges::transform(c.range(), c.range2(), c.out(), [](auto x, auto y) constexpr { return x + y; }); }));
};

RTTS_CASE_TPL("raze::algorithm::transform.binary_projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    auto op = [](auto x, auto y) { return x + y; };
    auto op_branching = [](auto x, auto y) {
        if (x > y) return static_cast<decltype(x)>(x + y);
        return y;
    };
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

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op_branching, proj1, proj2](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.range2(), c.out(), op_branching, proj1, proj2); },
        [op_branching, proj1, proj2](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), op_branching, proj1, proj2); });

    rtts::algorithm::run_aligned<64, 1, 32, rtts::algorithm::range, T>(acfg,
        [op, proj1, proj2](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned, 32>](c.range(), c.range2(), c.out(), op, proj1, proj2); },
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

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform[raze::options::fautovec][raze::options::aligned<64, raze::options::unaligned, 32>](
                c.range(), c.range2(), c.out(),
                [](auto x, auto y) constexpr { if (x > y) return static_cast<decltype(x)>(x + y); return y; },
                [](const T& x) constexpr -> const T& { return x; },
                [](const T& y) constexpr -> const T& { return y; }
            );
        },
        [](auto c) constexpr {
            return std::ranges::transform(
                c.range(), c.range2(), c.out(),
                [](auto x, auto y) constexpr { if (x > y) return static_cast<decltype(x)>(x + y); return y; },
                [](const T& x) constexpr -> const T& { return x; },
                [](const T& y) constexpr -> const T& { return y; }
            );
        }));
};

struct tc_struct_4 {
    int32_t x;
    bool operator==(const tc_struct_4&) const = default;
};
struct tc_struct_8 {
    int64_t x;
    bool operator==(const tc_struct_8&) const = default;
};

RTTS_CASE("raze::algorithm::transform.struct_unary") {
    auto cfg = rtts::algorithm::config::thorough();

    auto op = [](auto v) constexpr {
        if constexpr (requires { v.x; }) {
            return tc_struct_4{ v.x * 2 };
        }
        else {
            return v * 2;
        }
        };
    auto std_op = [](const tc_struct_4& v) { return tc_struct_4{ v.x * 2 }; };

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), std_op); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::unroll<2>](c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::fstatic](c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });

        auto acfg = rtts::algorithm::config::compact();
        rtts::algorithm::run_aligned<64, 1, rtts::algorithm::range, tc_struct_4>(acfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), op); },
            [std_op](auto c) { return std::ranges::transform(c.range(), c.out(), std_op); });
    }

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.out(),
                [](auto v) constexpr {
                    if constexpr (requires { v.x; }) return tc_struct_4{ v.x * 2 };
                    else return v * 2;
                });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.out(),
                [](const tc_struct_4& v) constexpr { return tc_struct_4{ v.x * 2 }; });
        }));
};

RTTS_CASE("raze::algorithm::transform.struct_binary") {
    auto cfg = rtts::algorithm::config::thorough();

    auto op = [](auto a, auto b) constexpr {
        if constexpr (requires { a.x; }) {
            return tc_struct_4{ a.x + b.x };
        }
        else {
            return a + b;
        }
        };
    auto std_op = [](const tc_struct_4& a, const tc_struct_4& b) { return tc_struct_4{ a.x + b.x }; };

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), op); },
        [std_op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(), std_op); });

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op](auto c) { return raze::algorithm::transform(c.range(), c.range2(), c.out(), op); },
        [std_op](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), std_op); });

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op](auto c) { return raze::algorithm::transform[raze::options::fscalar](c.range(), c.range2(), c.out(), op); },
        [std_op](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), std_op); });

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.range(), c.range2(), c.out(), op); },
        [std_op](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), std_op); });

    rtts::algorithm::run_aligned<64, 1, 32, rtts::algorithm::range, tc_struct_4>(rtts::algorithm::config::compact(),
        [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned, 32>](c.range(), c.range2(), c.out(), op); },
        [std_op](auto c) { return std::ranges::transform(c.range(), c.range2(), c.out(), std_op); });

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(),
                [](auto a, auto b) constexpr {
                    if constexpr (requires { a.x; }) return tc_struct_4{ a.x + b.x };
                    else return a + b;
                });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out(),
                [](const tc_struct_4& a, const tc_struct_4& b) constexpr { return tc_struct_4{ a.x + b.x }; });
        }));
};

RTTS_CASE("raze::algorithm::transform.struct_unary_projection") {
    auto cfg = rtts::algorithm::config::thorough();

    auto proj = [](auto v) constexpr { return v; };
    auto std_proj = [](const tc_struct_4& v) -> const tc_struct_4& { return v; };

    auto op = [](auto v) constexpr {
        if constexpr (requires { v.x; }) {
            return tc_struct_4{ v.x + 1 };
        }
        else {
            return v + 1;
        }
        };
    auto std_op = [](const tc_struct_4& v) { return tc_struct_4{ v.x + 1 }; };

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op, proj](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out(), op, proj); },
        [std_op, std_proj](auto c) { return std::ranges::transform(c.first(), c.last(), c.out(), std_op, std_proj); });

    rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
        [op, proj](auto c) { return raze::algorithm::transform(c.range(), c.out(), op, proj); },
        [std_op, std_proj](auto c) { return std::ranges::transform(c.range(), c.out(), std_op, std_proj); });

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.out(),
                [](auto v) constexpr {
                    if constexpr (requires { v.x; }) return tc_struct_4{ v.x + 1 };
                    else return v + 1;
                },
                [](auto v) constexpr { return v; });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.out(),
                [](const tc_struct_4& v) constexpr { return tc_struct_4{ v.x + 1 }; },
                [](const tc_struct_4& v) constexpr -> const tc_struct_4& { return v; });
        }));
};

RTTS_CASE("raze::algorithm::transform.mixed_unary") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        auto op = [](auto v) { return v * 2.0f; };
        auto acfg = rtts::algorithm::config::compact();

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op); });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.first(), c.last(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op); });

        rtts::algorithm::run_aligned_mixed<64, 1, rtts::algorithm::range, int32_t, float>(acfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned>](c.first(), c.last(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op); });

        rtts::algorithm::run_aligned_mixed<64, 64, rtts::algorithm::range, int32_t, float>(acfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64>](c.first(), c.last(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op); });
    }

    {
        auto op = [](auto v) { return v * 2.0; };

        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.out2(), [](auto v) constexpr { return v * 2.0f; }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.out2(), [](auto v) constexpr { return v * 2.0f; }); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform(c.first(), c.last(), c.out2(), [](auto v) constexpr { return v * 2.0; }); },
        [](auto c) constexpr { return std::ranges::transform(c.first(), c.last(), c.out2(), [](auto v) constexpr { return v * 2.0; }); }));
};

RTTS_CASE("raze::algorithm::transform.mixed_binary") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    {
        auto op = [](auto a, auto b) { return (a + 42 * b); };
        auto acfg = rtts::algorithm::config::compact();

        rtts::algorithm::run<rtts::algorithm::range, float, char>(cfg,
            [op](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run<rtts::algorithm::range, float, char>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::fautovec](c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run_aligned_mixed<64, 64, 64, rtts::algorithm::range, float, char>(acfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64>](c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run_aligned_mixed<64, 1, 32, rtts::algorithm::range, int32_t, float>(acfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::aligned<64, raze::options::unaligned, 32>](c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(),
                [](auto a, auto b) constexpr { return a + b; });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.first2(), c.last2(), c.out2(),
                [](auto a, auto b) constexpr { return a + b; });
        }));
};

RTTS_CASE("raze::algorithm::transform.strict_types_homogeneous") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); });

        rtts::algorithm::run<rtts::algorithm::range, float>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); });
    }

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t>(cfg,
            [op](auto c) { return raze::algorithm::transform
            [raze::options::strict_types][raze::options::unroll<2>](
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out(), op); });
    }

    static_assert(rtts::algorithm::constexpr_run<int32_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::strict_types](
            c.first(), c.last(), c.first2(), c.last2(), c.out(),
            [](auto a, auto b) constexpr { return a + b; }); },
        [](auto c) constexpr { return std::ranges::transform(
            c.first(), c.last(), c.first2(), c.last2(), c.out(),
            [](auto a, auto b) constexpr { return a + b; }); }));
};

RTTS_CASE("raze::algorithm::transform.strict_types_mixed") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    {
        auto op = [](auto a, auto b) { return a + 42 * b; };

        rtts::algorithm::run<rtts::algorithm::range, float, char>(cfg,
            [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    {
        auto op = [](auto a, auto b) { return a + b; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform
            [raze::options::strict_types][raze::options::fscalar](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op](auto c) { return raze::algorithm::transform
            [raze::options::strict_types][raze::options::fautovec](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

        rtts::algorithm::run_aligned_mixed<64, 1, 32, rtts::algorithm::range, int32_t, float>(rtts::algorithm::config::compact(),
            [op](auto c) { return raze::algorithm::transform
            [raze::options::strict_types][raze::options::aligned<64, raze::options::unaligned, 32>](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
            [op](auto c) { return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::transform[raze::options::strict_types](
            c.first(), c.last(), c.first2(), c.last2(), c.out2(),
            [](auto a, auto b) constexpr { return a + b; }); },
        [](auto c) constexpr { return std::ranges::transform(
            c.first(), c.last(), c.first2(), c.last2(), c.out2(),
            [](auto a, auto b) constexpr { return a + b; }); }));
};

RTTS_CASE("raze::algorithm::transform.strict_types_type_inspecting") {
    auto cfg = rtts::algorithm::config::thorough();

    auto op = [](auto a, auto b) {
        if constexpr (sizeof(a) <= sizeof(int32_t)) {
            return a + b;
        }
        else {
            return decltype(a)(1);
        }
        };

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
        [op](auto c) { return std::ranges::transform(
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

    rtts::algorithm::run<rtts::algorithm::range, float, char>(cfg,
        [op](auto c) { return raze::algorithm::transform[raze::options::strict_types](
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
        [op](auto c) { return std::ranges::transform(
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

    rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
        [op](auto c) { return raze::algorithm::transform
        [raze::options::strict_types][raze::options::fscalar](
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); },
        [op](auto c) { return std::ranges::transform(
            c.first(), c.last(), c.first2(), c.last2(), c.out2(), op); });

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform[raze::options::strict_types](
                c.first(), c.last(), c.first2(), c.last2(), c.out2(),
                [](auto a, auto b) constexpr {
                    if constexpr (sizeof(a) <= sizeof(int32_t)) return a + b;
                    else return decltype(a)(1);
                });
        },
        [](auto c) constexpr {
            return std::ranges::transform(
                c.first(), c.last(), c.first2(), c.last2(), c.out2(),
                [](auto a, auto b) constexpr {
                    if constexpr (sizeof(a) <= sizeof(int32_t)) return a + b;
                    else return decltype(a)(1);
                });
        }));
};


RTTS_CASE("raze::algorithm::transform views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Unary & binary views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        const std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        const std::vector<int> b = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };

        std::vector<int> u1, u2;
        raze::algorithm::transform[opt::fautovec][opt::fstatic](
            views::iota(1, 25), std::back_inserter(u1), [](auto x) { if (x > 10) return x * 2; return x + 1; });
        std::ranges::transform(
            views::iota(1, 25), std::back_inserter(u2), [](auto x) { if (x > 10) return x * 2; return x + 1; });
        RTTS_EXPECT(u1 == u2);

        auto chain = a | views::filter([](int x) { return (x % 2) == 0; }) | views::transform([](int x) { return x + 5; }) | views::reverse;
        std::vector<int> uc1, uc2;
        raze::algorithm::transform[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(uc1), [](int x) { return x * 3; });
        std::ranges::transform(chain, std::back_inserter(uc2), [](int x) { return x * 3; });
        RTTS_EXPECT(uc1 == uc2);

        // Binary views with mismatched lengths
        std::vector<int> bv1, bv2;
        raze::algorithm::transform[opt::fautovec][opt::fstatic](
            views::iota(1, 15), b | views::reverse, std::back_inserter(bv1),
            [](auto x, auto y) { if (x > 5) return x + y; return y - x; });
        std::ranges::transform(
            views::iota(1, 15), b | views::reverse, std::back_inserter(bv2),
            [](auto x, auto y) { if (x > 5) return x + y; return y - x; });
        RTTS_EXPECT(bv1 == bv2);

        auto fv1 = a | views::filter([](int x) { return x > 4; });
        auto tv2 = b | views::transform([](int x) { return x / 10; });
        std::vector<int> bv3, bv4;
        raze::algorithm::transform[opt::aligned<64, opt::unaligned, opt::unaligned>](
            fv1, tv2, std::back_inserter(bv3), [](int x, int y) { return x * y; });
        std::ranges::transform(fv1, tv2, std::back_inserter(bv4), [](int x, int y) { return x * y; });
        RTTS_EXPECT(bv3 == bv4);

        std::vector<int> nc1(a.size()), nc2(a.size());
        raze::algorithm::transform(rtts::make_non_common(a), nc1.begin(), [](auto x) { return x + 1; });
        std::ranges::transform(rtts::make_non_common(a), nc2.begin(), [](auto x) { return x + 1; });
        RTTS_EXPECT(nc1 == nc2);
    }

    // back_inserter, front_inserter, inserter, single-pass input ranges (unary & binary)
    {
        const std::vector<int> src1 = { 1, 2, 3, 4, 5 };
        const std::vector<int> src2 = { 10, 20, 30, 40, 50, 60, 70 };

        std::deque<int> f1, f2;
        raze::algorithm::transform(src1, std::front_inserter(f1), [](auto x) { return x * 10; });
        std::ranges::transform(src1, std::front_inserter(f2), [](auto x) { return x * 10; });
        RTTS_EXPECT(f1 == f2);

        std::vector<int> ins1 = { 0, 0 }, ins2 = { 0, 0 };
        raze::algorithm::transform[opt::fscalar](src1, src2, std::inserter(ins1, ins1.begin() + 1), std::plus<>{});
        std::ranges::transform(src1, src2, std::inserter(ins2, ins2.begin() + 1), std::plus<>{});
        RTTS_EXPECT(ins1 == ins2);

        rtts::single_pass_input_range<int> sp1(src1);
        std::vector<int> sp_u;
        raze::algorithm::transform(sp1, std::back_inserter(sp_u), [](int x) { return x + 1; });
        RTTS_EXPECT((sp_u == std::vector<int>{ 2, 3, 4, 5, 6 }) && sp1.reads() == src1.size());

        rtts::single_pass_input_range<int> sp2a(src1);
        rtts::single_pass_input_range<int> sp2b(src2);
        std::vector<int> sp_b;
        raze::algorithm::transform(sp2a, sp2b, std::back_inserter(sp_b), std::plus<>{});
        RTTS_EXPECT((sp_b == std::vector<int>{ 11, 22, 33, 44, 55 }) && sp2a.reads() == src1.size());
    }

    // Mismatched lengths (contiguous), self-aliasing, volatile, rvalue dangling, member projections
    {
        const std::vector<int> short_v(19, 3);
        const std::vector<int> long_v(53, 7);
        std::vector<int> out1(19), out2(19);
        auto rb1 = raze::algorithm::transform(short_v, long_v, out1.begin(), std::plus<>{});
        auto sb1 = std::ranges::transform(short_v, long_v, out2.begin(), std::plus<>{});
        RTTS_EXPECT(out1 == out2 && rb1.in1 == sb1.in1 && rb1.in2 == sb1.in2 && (rb1.out - out1.begin()) == (sb1.out - out2.begin()));

        auto rb2 = raze::algorithm::transform(long_v, short_v, out1.begin(), std::plus<>{});
        auto sb2 = std::ranges::transform(long_v, short_v, out2.begin(), std::plus<>{});
        RTTS_EXPECT(out1 == out2 && rb2.in1 == sb2.in1 && rb2.in2 == sb2.in2);

        std::vector<int> empty_v;
        auto rb_empty = raze::algorithm::transform(empty_v, long_v, out1.begin(), std::plus<>{});
        RTTS_EXPECT(rb_empty.in1 == empty_v.end() && rb_empty.in2 == long_v.begin() && rb_empty.out == out1.begin());

        // Self-aliasing unary & binary
        std::vector<int> self_v(35);
        std::iota(self_v.begin(), self_v.end(), 1);
        auto self_exp = self_v;
        raze::algorithm::transform(self_v, self_v.begin(), [](auto x) { return x * 2 + 1; });
        std::ranges::transform(self_exp, self_exp.begin(), [](int x) { return x * 2 + 1; });
        RTTS_EXPECT(self_v == self_exp);

        raze::algorithm::transform(self_v, self_v, self_v.begin(), [](auto x, auto y) { return x + y; });
        std::ranges::transform(self_exp, self_exp, self_exp.begin(), [](int x, int y) { return x + y; });
        RTTS_EXPECT(self_v == self_exp);

        // Volatile buffers
        volatile int vs1[16], vs2[16], vd[16];
        for (int i = 0; i < 16; ++i) { vs1[i] = i; vs2[i] = i * 2; }
        raze::algorithm::transform(vs1, vs1 + 16, vd, [](int x) { return x + 10; });
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vd[i] == i + 10);
        raze::algorithm::transform(vs1, vs1 + 16, vs2, vs2 + 16, vd, [](int x, int y) { return x + y; });
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vd[i] == i * 3);

        // Non-borrowed rvalue containers -> dangling
        std::vector<int> rdst;
        auto ru = raze::algorithm::transform(std::vector<int>{ 1, 2, 3 }, std::back_inserter(rdst), [](auto x) { return x * 10; });
        static_assert(std::same_as<decltype(ru.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 10, 20, 30 }));

        std::vector<int> rbdst;
        auto rb = raze::algorithm::transform(std::vector<int>{ 1, 2 }, std::vector<int>{ 3, 4 }, std::back_inserter(rbdst), std::plus<>{});
        static_assert(std::same_as<decltype(rb.in1), std::ranges::dangling>);
        static_assert(std::same_as<decltype(rb.in2), std::ranges::dangling>);
        RTTS_EXPECT((rbdst == std::vector<int>{ 4, 6 }));

        // Pointer-to-member projections
        const std::vector<rtts::test_item> it1 = { { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<rtts::test_item> it2 = { { 10, 1 }, { 20, 2 }, { 30, 3 } };
        std::vector<int> proj_out;
        raze::algorithm::transform(it1, it2, std::back_inserter(proj_out), std::plus<>{}, &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EXPECT((proj_out == std::vector<int>{ 11, 22, 33 }));
    }
};