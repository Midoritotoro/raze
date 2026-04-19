#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct bitwise_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    template<class SimdOp, class ScalarOp>
    static void run(const Simd& a, const Simd& b,
                    const _Type_(&A)[N], const _Type_(&B)[N],
                    SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(r[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])), std::bit_cast<U>(_Type_(B[i])))));
    }

    template<class SimdOp, class ScalarOp>
    static void run_unary(const Simd& a,
                          const _Type_(&A)[N],
                          SimdOp sop, ScalarOp sc)
    {
        auto r = sop(a);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(r[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])))));
    }

    template<class SimdOp, class ScalarOp>
    static void run_assign(const Simd& a0, const Simd& b,
                           const _Type_(&A)[N], const _Type_(&B)[N],
                           SimdOp sop, ScalarOp sc)
    {
        Simd a = a0;
        sop(a, b);
        for (size_t i = 0; i < N; ++i)
            raze_assert(std::bit_cast<U>(_Type_(a[i])) ==
                        U(sc(std::bit_cast<U>(_Type_(A[i])), std::bit_cast<U>(_Type_(B[i])))));
    }

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N];

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_((i * 0x5A5A) ^ (i + 3));
            arrB[i] = _Type_((N - i) * 0x33 ^ (i * 7));
        }

        arrA[0]     = -1;
        arrA[N - 1] = 255;

        Simd a; a.copy_from(arrA);
        Simd b; b.copy_from(arrB);

        run(a, b, arrA, arrB, [](auto x, auto y){ return x & y; }, [](auto x, auto y){ return x & y; });
        run(a, b, arrA, arrB, [](auto x, auto y){ return x | y; }, [](auto x, auto y){ return x | y; });
        run(a, b, arrA, arrB, [](auto x, auto y){ return x ^ y; }, [](auto x, auto y){ return x ^ y; });

        run_unary(a, arrA, [](auto x){ return ~x; }, [](auto x){ return ~x; });

        run(a, b, arrA, arrB,
            [](auto x, auto y){ return (x & y) ^ (~x | y); },
            [](auto x, auto y){ return (x & y) ^ (~x | y); });

        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x &= y; }, [](auto x, auto y){ return x & y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x |= y; }, [](auto x, auto y){ return x | y; });
        run_assign(a, b, arrA, arrB, [](auto& x, auto y){ x ^= y; }, [](auto x, auto y){ return x ^ y; });

        if constexpr (std::is_integral_v<_Type_>) {
            for (raze::uint32 sh = 0; sh <= sizeof(_Type_) * 8; ++sh) {
                {
                    Simd v = a;
                    v <<= sh;

                    for (size_t i = 0; i < N; ++i) {
                        _Type_ scalar = 0;
                        if (sh < sizeof(_Type_) * 8)
                            scalar = _Type_(arrA[i]) << sh;

                        raze_assert(v[i] == scalar);
                    }
                }

                {
                    Simd v = a;
                    v >>= sh;

                    for (size_t i = 0; i < N; ++i) {
                        _Type_ scalar = 0;

                        if (sh < sizeof(_Type_) * 8) {
                            scalar = _Type_(arrA[i]) >> sh;
                        }
                        else {
                            if constexpr (std::is_signed_v<_Type_>)
                                scalar = _Type_(arrA[i]) >> (sizeof(_Type_) * 8 - 1);
                        }

                        raze_assert(v[i] == scalar);
                    }
                }
            }
        }

        {
            alignas(64) _Type_ arrSrc[N];
            std::iota(arrSrc, arrSrc + N, 50);

            Simd src; src.copy_from(arrSrc);

            const auto run_tests = [arrA, arrB, arrSrc, a, b, src](auto m) {
                test_where_unary<_Type_, N>(
                    arrA, arrSrc, m, a, src,
                    [m, src](Simd A) {
                        return raze::vx::bit_not[m, src](A);
                    },
                    [m](Simd A) {
                        return raze::vx::bit_not[m](A);
                    },
                    [](_Type_ A, _Type_ Src, bool cond, bool rev) {
                        if constexpr (std::is_floating_point_v<_Type_>)
                            return cond ? std::bit_cast<_Type_>(~std::bit_cast<U>(A)) : Src;
                        else
                            return cond ? (~A) : Src;
                    });

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src,
                    [m, src](Simd A, Simd B) {
                        return raze::vx::bit_and[m, src](A, B);
                    },
                    [m](Simd A, Simd B) {
                        return raze::vx::bit_and[m](A, B);
                    },
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        if constexpr (std::is_floating_point_v<_Type_>)
                            return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
                                & std::bit_cast<U>(A))) : Src;
                        else
                            return cond ? (A & B) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src,
                    [m, src](Simd A, Simd B) {
                        return raze::vx::bit_or[m, src](A, B);
                    },
                    [m](Simd A, Simd B) {
                        return raze::vx::bit_or[m](A, B);
                    },
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        if constexpr (std::is_floating_point_v<_Type_>)
                            return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
                                | std::bit_cast<U>(A))) : Src;
                        else
                            return cond ? (A | B) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src,
                    [m, src](Simd A, Simd B) {
                        return raze::vx::bit_xor[m, src](A, B);
                    },
                    [m](Simd A, Simd B) {
                        return raze::vx::bit_xor[m](A, B);
                    },
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        if constexpr (std::is_floating_point_v<_Type_>)
                            return cond ? std::bit_cast<_Type_>((std::bit_cast<U>(B)
                                ^ std::bit_cast<U>(A))) : Src;
                        else
                            return cond ? (A ^ B) : Src;
                    }
                );

                if constexpr (std::is_integral_v<_Type_>) {
                    for (auto sh = 0; sh < N; ++sh) {
                        test_where_unary<_Type_, N>(
                            arrA, arrSrc, m,
                            a, src,
                            [m, src, sh](Simd A) {
                                return raze::vx::bit_shl[m, src](A, sh);
                            },
                            [m, sh](Simd A) {
                                return raze::vx::bit_shl[m](A, sh);
                            },
                            [sh](_Type_ A, _Type_ Src, bool cond, bool rev) {
                                if (!cond)
                                    return Src;

                                _Type_ scalar = 0;

                                if (sh < sizeof(_Type_) * 8)
                                    scalar = A << sh;

                                return scalar;
                            }
                        );

                        test_where_unary<_Type_, N>(
                            arrA, arrSrc, m,
                            a, src,
                            [m, src, sh](Simd A) {
                                return raze::vx::bit_shr[m, src](A, sh);
                            },
                            [m, sh](Simd A) {
                                return raze::vx::bit_shr[m](A, sh);
                            },
                            [sh](_Type_ A, _Type_ Src, bool cond, bool rev) {
                                if (!cond)
                                    return Src;

                                _Type_ scalar = 0;

                                if (sh < sizeof(_Type_) * 8) {
                                    scalar = A >> sh;
                                }
                                else {
                                    if constexpr (std::is_signed_v<_Type_>)
                                        scalar = A >> (sizeof(_Type_) * 8 - 1);
                                }

                                return scalar;
                            }
                        );
                    }
                }

                /*test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src,
                    [m, src](Simd A, Simd B) {
                        return raze::vx::bit_shl[m, src](A, B);
                    },
                    [m](Simd A, Simd B) {
                        return raze::vx::bit_shl[m](A, B);
                    },
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        constexpr size_t bits = sizeof(_Type_) * 8;
                        U v = std::bit_cast<U>(A);
                        U s = std::bit_cast<U>(B) % bits;
                        return cond ? std::bit_cast<_Type_>(v << s) : Src;
                    }
                );

                test_where_binary<_Type_, N>(
                    arrA, arrB, arrSrc, m,
                    a, b, src,
                    [m, src](Simd A, Simd B) {
                        return raze::vx::bit_shr[m, src](A, B);
                    },
                    [m](Simd A, Simd B) {
                        return raze::vx::bit_shr[m](A, B);
                    },
                    [](_Type_ A, _Type_ B, _Type_ Src, bool cond, bool rev) {
                        constexpr size_t bits = sizeof(_Type_) * 8;
                        U v = std::bit_cast<U>(A);
                        U s = std::bit_cast<U>(B) % bits;
                        return cond ? std::bit_cast<_Type_>(v >> s) : Src;
                    }
                );*/
            };

            for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
                run_tests(make_random_mask<Mask>());
            }
        }
    }
};

int main() {
    test_all<bitwise_tests>();
    return 0;
}
