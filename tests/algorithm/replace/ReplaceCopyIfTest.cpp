#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopyIf.h>

RTTS_CASE_TPL("raze::algorithm::replace_copy_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    auto check = [&](auto algo) {
        {
            const T threshold = rtts::random::generator<T>(1000)();
            const T new_val = rtts::random::generator<T>(2000)();

            auto pred_scalar = [threshold](const T& x) { return x > threshold; };
            auto pred_vector = [threshold](auto x) { return x > threshold; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_scalar, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred_scalar, new_val); },
                [pred_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_vector, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred_vector, new_val); },
                [pred_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_vector, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_scalar, new_val](auto c) { return algo(c.range(), c.out(), pred_scalar, new_val); },
                [pred_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_vector, new_val](auto c) { return algo(c.range(), c.out(), pred_vector, new_val); },
                [pred_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_vector, new_val); });
        }

        {
            const T target = rtts::random::generator<T>(4000)();
            const T new_val = rtts::random::generator<T>(5000)();

            auto pred_scalar = [target](const T& x) { return x == target; };
            auto pred_vector = [target](auto x) { return x == target; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_scalar, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred_scalar, new_val); },
                [pred_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_vector, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred_vector, new_val); },
                [pred_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_vector, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_scalar, new_val](auto c) { return algo(c.range(), c.out(), pred_scalar, new_val); },
                [pred_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred_vector, new_val](auto c) { return algo(c.range(), c.out(), pred_vector, new_val); },
                [pred_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_vector, new_val); });
        }

        {
            const T new_val = rtts::random::generator<T>(6000)();

            auto false_scalar = [](const T&) { return false; };
            auto false_vector = [](auto) { return false; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, false_scalar, new_val](auto c) { return algo(c.first(), c.last(), c.out(), false_scalar, new_val); },
                [false_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), false_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, false_vector, new_val](auto c) { return algo(c.first(), c.last(), c.out(), false_vector, new_val); },
                [false_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), false_vector, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, false_scalar, new_val](auto c) { return algo(c.range(), c.out(), false_scalar, new_val); },
                [false_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), false_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, false_vector, new_val](auto c) { return algo(c.range(), c.out(), false_vector, new_val); },
                [false_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), false_vector, new_val); });
        }

        {
            const T new_val = rtts::random::generator<T>(7000)();

            auto true_scalar = [](const T&) { return true; };
            auto true_vector = [](auto) { return true; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, true_scalar, new_val](auto c) { return algo(c.first(), c.last(), c.out(), true_scalar, new_val); },
                [true_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), true_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, true_vector, new_val](auto c) { return algo(c.first(), c.last(), c.out(), true_vector, new_val); },
                [true_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), true_vector, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, true_scalar, new_val](auto c) { return algo(c.range(), c.out(), true_scalar, new_val); },
                [true_scalar, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), true_scalar, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, true_vector, new_val](auto c) { return algo(c.range(), c.out(), true_vector, new_val); },
                [true_vector, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), true_vector, new_val); });
        }
    };

    check(replace_copy_if);
    check(replace_copy_if[opt::fstatic]);
    check(replace_copy_if[opt::fscalar]);
    check(replace_copy_if[opt::unroll<1>]);
    check(replace_copy_if[opt::unroll<2>]);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }, T(99));
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy_if[raze::options::fstatic](c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }, T(99));
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy_if[raze::options::fscalar](c.range(), c.out(), [](auto x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(c.range(), c.out(), [](auto x) constexpr { return x > T(0); }, T(99));
        }));
};

RTTS_CASE_TPL("raze::algorithm::replace_copy_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();

    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };
    auto proj = [](const T& x) -> const T& { return x; };

    auto check = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred_scalar, new_val, proj](auto c) { return algo(c.first(), c.last(), c.out(), pred_scalar, new_val, proj); },
            [pred_scalar, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_scalar, new_val, proj); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred_vector, new_val, proj](auto c) { return algo(c.first(), c.last(), c.out(), pred_vector, new_val, proj); },
            [pred_vector, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred_vector, new_val, proj); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred_scalar, new_val, proj](auto c) { return algo(c.range(), c.out(), pred_scalar, new_val, proj); },
            [pred_scalar, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_scalar, new_val, proj); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred_vector, new_val, proj](auto c) { return algo(c.range(), c.out(), pred_vector, new_val, proj); },
            [pred_vector, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_vector, new_val, proj); });
    };

    check(replace_copy_if);
    check(replace_copy_if[opt::fstatic]);
    check(replace_copy_if[opt::fscalar]);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy_if(
                c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(
                c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_copy_if[raze::options::fscalar](
                c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(
                c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));
};