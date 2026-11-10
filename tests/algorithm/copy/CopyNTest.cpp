#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyN.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iterator>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::copy_n", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::fscalar](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::unroll<2>](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::fstatic](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        }));
};

RTTS_CASE("raze::algorithm::copy_n mixed arithmetic types") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });

        rtts::algorithm::run<rtts::algorithm::range, uint32_t, float>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, int64_t>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });

        rtts::algorithm::run<rtts::algorithm::range, int16_t, int8_t>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::fscalar](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
            });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
        }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
        }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int8_t, int16_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out2());
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

RTTS_CASE("raze::algorithm::copy_n structs") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_8>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_12>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::fscalar](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) {
                return raze::algorithm::copy_n[raze::options::unroll<2>](
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            },
            [](auto c) {
                return std::ranges::copy_n(
                    c.first(), std::ranges::distance(c.first(), c.last()), c.out());
            });
    }

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_8, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_12, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        }));

    static_assert(rtts::algorithm::constexpr_run<tc_nontrivial, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        },
        [](auto c) constexpr {
            return std::ranges::copy_n(
                c.first(), std::ranges::distance(c.first(), c.last()), c.out());
        }));
};

RTTS_CASE("raze::algorithm::copy_n non trivially copyable") {
    for (const size_t total : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<tc_nontrivial> src(total);
        for (size_t i = 0; i < total; ++i)
            src[i].x = static_cast<int32_t>(i * 7 + 1);

        for (const size_t n : { size_t(0), size_t(1), total / 2, total }) {
            if (n > total) continue;

            std::vector<tc_nontrivial> d1(n), d2(n);

            const auto r1 = raze::algorithm::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d1.begin());
            const auto r2 = std::ranges::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d2.begin());

            RTTS_EXPECT(d1 == d2);
            RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
            RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));
        }
    }
};

RTTS_CASE("raze::algorithm::copy_n partial count (n < distance)") {
    for (const size_t total : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(17),
                                size_t(31), size_t(32), size_t(33), size_t(100), size_t(1024) }) {
        for (const size_t n : { size_t(0), size_t(1), total / 2, total }) {
            if (n > total) continue;

            std::vector<int32_t> src(total);
            for (size_t i = 0; i < total; ++i)
                src[i] = static_cast<int32_t>(i * 7 + 1);

            constexpr size_t guard = 32;
            std::vector<int32_t> d1(n + guard, -1);
            auto d2 = d1;

            const auto r1 = raze::algorithm::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d1.begin());
            const auto r2 = std::ranges::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d2.begin());

            RTTS_EXPECT(d1 == d2);
            RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
            RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

            // Скопировано ровно n элементов, guard-зона не тронута.
            for (size_t i = 0; i < n; ++i) RTTS_EXPECT(d1[i] == src[i]);
            for (size_t i = n; i < n + guard; ++i) RTTS_EXPECT(d1[i] == -1);
        }
    }
};

RTTS_CASE("raze::algorithm::copy_n memcpy size boundaries") {
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
            for (const size_t dst_off : { size_t(0), size_t(1), size_t(7) }) {
                std::vector<uint8_t> src(n + src_off);
                for (size_t i = 0; i < src.size(); ++i)
                    src[i] = static_cast<uint8_t>(i * 131 + 7);

                std::vector<uint8_t> d1(n + dst_off + guard, 0xCC);
                auto d2 = d1;

                const auto first = src.begin() + static_cast<std::ptrdiff_t>(src_off);

                const auto r1 = raze::algorithm::copy_n(first,
                    static_cast<std::ptrdiff_t>(n),
                    d1.begin() + static_cast<std::ptrdiff_t>(dst_off));
                const auto r2 = std::ranges::copy_n(first,
                    static_cast<std::ptrdiff_t>(n),
                    d2.begin() + static_cast<std::ptrdiff_t>(dst_off));

                RTTS_EXPECT(d1 == d2);
                RTTS_EXPECT(r1.in == r2.in);
                RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));
            }
        }
    }
};

namespace {
template <class T, std::size_t N>
constexpr bool copy_n_static_array_matches() {
    std::array<T, N> src{};
    for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

    std::array<T, N> dst1{}, dst2{};
    const auto r1 = raze::algorithm::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(N), dst1.begin());
    const auto r2 = std::ranges::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(N), dst2.begin());

    bool ok = dst1 == dst2
        && (r1.in - src.begin()) == (r2.in - src.begin())
        && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

    if constexpr (std::is_trivially_copyable_v<T>) ok = ok && dst1 == src;
    return ok;
}

template <class T, std::size_t N, std::size_t Count>
constexpr bool copy_n_static_array_partial_matches() {
    static_assert(Count <= N);

    std::array<T, N> src{};
    for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

    std::array<T, N> dst1{}, dst2{};
    const auto r1 = raze::algorithm::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(Count), dst1.begin());
    const auto r2 = std::ranges::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(Count), dst2.begin());

    bool ok = dst1 == dst2
        && (r1.in - src.begin()) == (r2.in - src.begin())
        && (r1.out - dst1.begin()) == (r2.out - dst2.begin())
        && static_cast<std::ptrdiff_t>(Count) == (r1.in - src.begin());

    if constexpr (std::is_trivially_copyable_v<T>) {
        for (std::size_t i = 0; i < Count; ++i) ok = ok && dst1[i] == src[i];
    }

    for (std::size_t i = Count; i < N; ++i) ok = ok && dst1[i] == T{};

    return ok;
}
}

static_assert(copy_n_static_array_matches<int32_t, 1>());
static_assert(copy_n_static_array_matches<int32_t, 7>());
static_assert(copy_n_static_array_matches<int8_t, 15>());
static_assert(copy_n_static_array_matches<int16_t, 33>());
static_assert(copy_n_static_array_matches<int64_t, 17>());
static_assert(copy_n_static_array_matches<uint8_t, 200>());
static_assert(copy_n_static_array_matches<tc_nontrivial, 5>());

static_assert(copy_n_static_array_partial_matches<int32_t, 16, 0>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 1>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 7>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 16>());
static_assert(copy_n_static_array_partial_matches<int64_t, 33, 17>());
static_assert(copy_n_static_array_partial_matches<uint8_t, 200, 129>());
static_assert(copy_n_static_array_partial_matches<tc_nontrivial, 8, 3>());