#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyBackward.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iterator>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::copy_backward", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return raze::algorithm::copy_backward(c.range(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return std::ranges::copy_backward(c.range(), c.out() + d);
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward[raze::options::fscalar](
                    c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return raze::algorithm::copy_backward[raze::options::unroll<2>](
                    c.range(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return std::ranges::copy_backward(c.range(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return raze::algorithm::copy_backward[raze::options::fstatic](
                    c.range(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return std::ranges::copy_backward(c.range(), c.out() + d);
            });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return raze::algorithm::copy_backward(c.range(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return std::ranges::copy_backward(c.range(), c.out() + d);
        }));
};

RTTS_CASE("raze::algorithm::copy_backward mixed arithmetic types") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, uint32_t, float, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int64_t, double, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, int64_t, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return raze::algorithm::copy_backward(c.range(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return std::ranges::copy_backward(c.range(), c.out2() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, int16_t, int8_t, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward[raze::options::fscalar](
                    c.first(), c.last(), c.out2() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
            });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out2() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int8_t, int16_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return raze::algorithm::copy_backward(c.range(), c.out2() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return std::ranges::copy_backward(c.range(), c.out2() + d);
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
struct tc_struct_12 {
    int32_t a, b, c;
    bool operator==(const tc_struct_12&) const = default;
};

struct tc_nontrivial {
    int32_t x = 0;
    int32_t copies = 0;

    constexpr tc_nontrivial() = default;
    constexpr tc_nontrivial(int32_t v) : x(v) {}
    constexpr tc_nontrivial(const tc_nontrivial&) = default;
    constexpr tc_nontrivial& operator=(const tc_nontrivial& o) {
        x = o.x;
        copies = o.copies + 1;
        return *this;
    }
    bool operator==(const tc_nontrivial&) const = default;
};

RTTS_CASE("raze::algorithm::copy_backward structs") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_8, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_12, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_12, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return raze::algorithm::copy_backward(c.range(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.range());
                return std::ranges::copy_backward(c.range(), c.out() + d);
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward[raze::options::fscalar](
                    c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4, rtts::algorithm::bidirectional_iterator>(cfg,
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return raze::algorithm::copy_backward[raze::options::unroll<2>](
                    c.first(), c.last(), c.out() + d);
            },
            [](auto c) {
                auto d = std::ranges::distance(c.first(), c.last());
                return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
            });
    }

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_8, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_12, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return raze::algorithm::copy_backward(c.range(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.range());
            return std::ranges::copy_backward(c.range(), c.out() + d);
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_nontrivial, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + d);
        },
        [](auto c) constexpr {
            auto d = std::ranges::distance(c.first(), c.last());
            return std::ranges::copy_backward(c.first(), c.last(), c.out() + d);
        }));
};

RTTS_CASE("raze::algorithm::copy_backward non trivially copyable") {
    for (const size_t n : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<tc_nontrivial> src(n);
        for (size_t i = 0; i < n; ++i)
            src[i].x = static_cast<int32_t>(i * 7 + 1);

        std::vector<tc_nontrivial> d1(n), d2(n);

        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), d1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.end(), d2.end());

        RTTS_EXPECT(d1 == d2);
        RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
        RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

        std::vector<tc_nontrivial> d3(n), d4(n);
        const auto r3 = raze::algorithm::copy_backward(src, d3.end());
        const auto r4 = std::ranges::copy_backward(src, d4.end());

        RTTS_EXPECT(d3 == d4);
        RTTS_EXPECT((r3.out - d3.begin()) == (r4.out - d4.begin()));
    }
};

RTTS_CASE("raze::algorithm::copy_backward overlap shift-right") {
    for (const size_t n : { size_t(1), size_t(2), size_t(3), size_t(5),
        size_t(7), size_t(8), size_t(9), size_t(15),
        size_t(16), size_t(17), size_t(31), size_t(32),
        size_t(33), size_t(63), size_t(64), size_t(65),
        size_t(100), size_t(1000) }) {
        for (const size_t shift : { size_t(1), size_t(2), size_t(3) }) {
            if (shift > n) continue;

            std::vector<int32_t> v1(n);
            for (size_t i = 0; i < n; ++i) v1[i] = static_cast<int32_t>(i);
            auto v2 = v1;

            const auto k = static_cast<std::ptrdiff_t>(n - shift);

            raze::algorithm::copy_backward(v1.begin(), v1.begin() + k, v1.end());
            std::ranges::copy_backward(v2.begin(), v2.begin() + k, v2.end());

            RTTS_EXPECT(v1 == v2);

            for (size_t i = 0; i < n; ++i) {
                if (i < shift)
                    RTTS_EXPECT(v1[i] == static_cast<int32_t>(i));
                else
                    RTTS_EXPECT(v1[i] == static_cast<int32_t>(i - shift));
            }
        }
    }
};

RTTS_CASE("raze::algorithm::copy_backward full overlap is no-op") {
    for (const size_t n : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<int32_t> v1(n);
        for (size_t i = 0; i < n; ++i) v1[i] = static_cast<int32_t>(i);
        auto v2 = v1;

        raze::algorithm::copy_backward(v1.begin(), v1.end(), v1.end());
        std::ranges::copy_backward(v2.begin(), v2.end(), v2.end());

        RTTS_EXPECT(v1 == v2);
        for (size_t i = 0; i < n; ++i) RTTS_EXPECT(v1[i] == static_cast<int32_t>(i));
    }
};

RTTS_CASE("raze::algorithm::copy_backward overlap with nontrivial type") {
    for (const size_t n : { size_t(1), size_t(3), size_t(7), size_t(15),
        size_t(16), size_t(17), size_t(64) }) {
        std::vector<tc_nontrivial> v1(n);
        for (size_t i = 0; i < n; ++i) v1[i].x = static_cast<int32_t>(i);
        auto v2 = v1;

        const auto k = static_cast<std::ptrdiff_t>(n - 1);
        if (k <= 0) continue;

        raze::algorithm::copy_backward(v1.begin(), v1.begin() + k, v1.end());
        std::ranges::copy_backward(v2.begin(), v2.begin() + k, v2.end());

        RTTS_EXPECT(v1 == v2);
    }
};

RTTS_CASE("raze::algorithm::copy_backward memcpy size boundaries") {
    const size_t ns[] = {
        0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
        127, 128, 129, 255, 256, 257, 511, 512, 513,
        1023, 1024, 1025, 2047, 2048, 2049, 4095, 4096, 4097, 8191, 8192, 8193,
        65536, 65537,
        3u * 1024 * 1024 - 1, 3u * 1024 * 1024, 3u * 1024 * 1024 + 1, 3u * 1024 * 1024 + 129
    };
    constexpr size_t guard = 64;

    for (const size_t n : ns) {
        for (const size_t src_off : { size_t(0), size_t(3) }) {
            for (const size_t dst_prefix : { size_t(0), size_t(1), size_t(7) }) {
                std::vector<uint8_t> src(n + src_off);
                for (size_t i = 0; i < src.size(); ++i)
                    src[i] = static_cast<uint8_t>(i * 131 + 7);

                std::vector<uint8_t> d1(dst_prefix + n + guard, 0xCC);
                auto d2 = d1;

                const auto first = src.begin() + static_cast<std::ptrdiff_t>(src_off);
                const auto last = src.end();

                const auto out_end_offset = static_cast<std::ptrdiff_t>(dst_prefix + n);

                const auto r1 = raze::algorithm::copy_backward(
                    first, last, d1.begin() + out_end_offset);
                const auto r2 = std::ranges::copy_backward(
                    first, last, d2.begin() + out_end_offset);

                RTTS_EXPECT(d1 == d2);
                RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
                RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

                for (size_t i = 0; i < dst_prefix; ++i)
                    RTTS_EXPECT(d1[i] == 0xCC);

                for (size_t i = dst_prefix + n; i < d1.size(); ++i)
                    RTTS_EXPECT(d1[i] == 0xCC);
            }
        }
    }
};

namespace {
    template <class T, std::size_t N>
    constexpr bool copy_backward_static_array_matches() {
        std::array<T, N> src{};
        for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

        std::array<T, N> dst1{}, dst2{};
        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst2.end());

        bool ok = dst1 == dst2
            && (r1.in - src.begin()) == (r2.in - src.begin())
            && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

        if constexpr (std::is_trivially_copyable_v<T>) ok = ok && dst1 == src;
        return ok;
    }

    template <class T, std::size_t N, std::size_t Shift>
    constexpr bool copy_backward_static_array_shift_right_matches() {
        static_assert(Shift < N);

        std::array<T, N> src{};
        for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

        std::array<T, N> dst1{}, dst2{};

        const auto k = static_cast<std::ptrdiff_t>(N - Shift);
        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.begin() + k, dst1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.begin() + k, dst2.end());

        bool ok = dst1 == dst2
            && (r1.in - src.begin()) == (r2.in - src.begin())
            && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

        for (std::size_t i = 0; i < Shift; ++i) ok = ok && dst1[i] == T{};
        if constexpr (std::is_trivially_copyable_v<T>) {
            for (std::size_t i = 0; i < N - Shift; ++i)
                ok = ok && dst1[i + Shift] == src[i];
        }

        return ok;
    }
}

static_assert(copy_backward_static_array_matches<int32_t, 1>());
static_assert(copy_backward_static_array_matches<int32_t, 7>());
static_assert(copy_backward_static_array_matches<int8_t, 15>());
static_assert(copy_backward_static_array_matches<int16_t, 33>());
static_assert(copy_backward_static_array_matches<int64_t, 17>());
static_assert(copy_backward_static_array_matches<uint8_t, 200>());
static_assert(copy_backward_static_array_matches<tc_nontrivial, 5>());

static_assert(copy_backward_static_array_shift_right_matches<int32_t, 8, 1>());
static_assert(copy_backward_static_array_shift_right_matches<int32_t, 8, 3>());
static_assert(copy_backward_static_array_shift_right_matches<int32_t, 16, 7>());
static_assert(copy_backward_static_array_shift_right_matches<int64_t, 33, 17>());
static_assert(copy_backward_static_array_shift_right_matches<uint8_t, 200, 129>());
static_assert(copy_backward_static_array_shift_right_matches<tc_nontrivial, 8, 3>());