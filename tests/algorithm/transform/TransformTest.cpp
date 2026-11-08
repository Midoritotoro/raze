#include <tests/rts/rts.h>
#include <raze/algorithm/transform/Transform.h>
#include <bit>
#include <cstdint>

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
        } else {
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
}

RTTS_CASE("raze::algorithm::transform.struct_binary") {
    auto cfg = rtts::algorithm::config::thorough();

    auto op = [](auto a, auto b) constexpr {
        if constexpr (requires { a.x; }) {
            return tc_struct_4{ a.x + b.x };
        } else {
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
}

RTTS_CASE("raze::algorithm::transform.struct_unary_projection") {
    auto cfg = rtts::algorithm::config::thorough();

    auto proj = [](auto v) constexpr { return v; };
    auto std_proj = [](const tc_struct_4& v) -> const tc_struct_4& { return v; };

    auto op = [](auto v) constexpr {
        if constexpr (requires { v.x; }) {
            return tc_struct_4{ v.x + 1 };
        } else {
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
}

RTTS_CASE("raze::algorithm::transform.mixed_arithmetic") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        auto op_raze = [](auto v) constexpr {
            if constexpr (std::is_same_v<decltype(v), int32_t>) {
                return static_cast<float>(v) * 2.0f;
            } else {
                return vx::convert<float>(v) * 2.0f;
            }
        };
        auto op_std = [](int32_t v) { return static_cast<float>(v) * 2.0f; };

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [op_raze](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out2(), op_raze); },
            [op_std](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op_std); });
    }

    {
        auto op_raze = [](auto v) constexpr {
            if constexpr (std::is_same_v<decltype(v), int64_t>) {
                return static_cast<double>(v) * 2.0;
            } else {
                return vx::convert<double>(v) * 2.0;
            }
        };
        auto op_std = [](int64_t v) { return static_cast<double>(v) * 2.0; };

        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [op_raze](auto c) { return raze::algorithm::transform(c.first(), c.last(), c.out2(), op_raze); },
            [op_std](auto c) { return std::ranges::transform(c.first(), c.last(), c.out2(), op_std); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.out2(),
                [](auto v) constexpr {
                    if constexpr (std::is_same_v<decltype(v), int32_t>) return static_cast<float>(v) * 2.0f;
                    else return vx::convert<float>(v) * 2.0f;
                });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.out2(),
                [](int32_t v) constexpr { return static_cast<float>(v) * 2.0f; });
        }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::transform(c.first(), c.last(), c.out2(),
                [](auto v) constexpr {
                    if constexpr (std::is_same_v<decltype(v), int64_t>) return static_cast<double>(v) * 2.0;
                    else return vx::convert<double>(v) * 2.0;
                });
        },
        [](auto c) constexpr {
            return std::ranges::transform(c.first(), c.last(), c.out2(),
                [](int64_t v) constexpr { return static_cast<double>(v) * 2.0; });
        }));
}