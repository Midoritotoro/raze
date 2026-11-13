#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyIf.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <vector>


struct tc_struct_4 { int32_t x;           bool operator==(const tc_struct_4&)  const = default; };
struct tc_struct_8 { int64_t x;           bool operator==(const tc_struct_8&)  const = default; };
struct tc_struct_12 { int32_t a, b, c;     bool operator==(const tc_struct_12&) const = default; };

struct tc_nontrivial {
    int32_t x = 0, copies = 0;
    constexpr tc_nontrivial() = default;
    constexpr tc_nontrivial(int32_t v) : x(v) {}
    constexpr tc_nontrivial(const tc_nontrivial&) = default;
    constexpr tc_nontrivial& operator=(const tc_nontrivial& o) { x = o.x; copies = o.copies + 1; return *this; }
    bool operator==(const tc_nontrivial&) const = default;
};

namespace {
    constexpr auto pos = [](auto v) { return v > decltype(v)(0); };
    constexpr auto even = [](auto v) { return (v & 1) == 0; };
    constexpr auto yes = [](auto) { return true; };
    constexpr auto no = [](auto) { return false; };
    constexpr auto x_even = [](const auto& v) { return v.x % 2 == 0; };
    constexpr auto x_pos = [](const auto& v) { return v.x > 0; };
    constexpr auto a_even = [](const auto& v) { return v.a % 2 == 0; };

    constexpr auto mod3 = [](auto v) { return (v & 3) == 0; };     
    constexpr auto mod7 = [](auto v) { return (v & 7) == 0; };  
    constexpr auto eq42 = [](auto v) { return v == decltype(v)(42); };  
    constexpr auto ne42 = [](auto v) { return v != decltype(v)(42); };  

    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };
    inline constexpr size_t fuzz_cases = 200;      
    inline constexpr size_t fuzz_max_size = 512;

    inline constexpr bool run_mixed_fuzz = false;

    template <class T>
    rtts::algorithm::config fuzz_config() {
        rtts::algorithm::config cfg;
        cfg.random_cases = fuzz_cases;
        cfg.random_max_size = fuzz_max_size;

        std::vector<size_t> s = {0,1,2,3,4,5,6,7,8,9,15,17,31,33,63,65,127,129,255,257};
        constexpr size_t es = sizeof(T);
        for (size_t bytes : {16, 32, 64, 128, 256, 512}) {
            const size_t n = bytes / es;
            if (n == 0) continue;
            for (size_t k = 1; k <= 4; ++k) {
                const size_t v = n * k;
                s.push_back(v - 1); s.push_back(v); s.push_back(v + 1);
            }
        }
        s.insert(s.end(), {1000, 1023, 1024, 1025, 4095, 4096, 4097});
        std::sort(s.begin(), s.end());
        s.erase(std::unique(s.begin(), s.end()), s.end());
        cfg.sizes = std::move(s);
        return cfg;
    }

    template <class T, class Pred>
    void tight_output_fuzz(Pred pred, unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 400);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            const auto in = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const size_t kept = static_cast<size_t>(std::ranges::count_if(in, pred));
            std::vector<T> a(kept), b(kept);
            auto ra = raze::algorithm::copy_if(in, a.begin(), pred);
            auto rb = std::ranges::copy_if(in, b.begin(), pred);
            RTTS_EXPECT(ra.out == a.end());
            RTTS_EXPECT(rb.out == b.end());
            RTTS_ALL_EQUAL(a, b);
        }
    }
}

RTTS_CASE("copy_if char distinct") {
    std::vector<char> in(128);
    for (int i = 0; i < 128; ++i)
        in[i] = static_cast<char>(i);

    std::vector<char> a(in.size(), char(-1));
    std::vector<char> b(in.size(), char(-1));

    auto pred = [](char x) { return (x & 3) == 0; };

    auto ra = raze::algorithm::copy_if(in, a.begin(), pred);
    auto rb = std::ranges::copy_if(in, b.begin(), pred);

    std::cout << "kept: " << ra.out - a.begin()
        << " / " << rb.out - b.begin() << '\n';
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            std::cout << "diff[" << i << "]: "
                << int(a[i]) << " / " << int(b[i]) << '\n';
            break;
        }
    }
    RTTS_EQUAL(ra.out - a.begin(), rb.out - b.begin());
    RTTS_ALL_EQUAL(a, b);
};

struct verbose_same {
    template <class A, class B, class RA, class RB>
    bool operator()(A const& a, B const& b, RA const& ra, RB const& rb) const {
        const bool se = rtts::algorithm::state_equal(a, b);
        const bool re = rtts::algorithm::result_equal(ra, rb, a, b);
        if (!se || !re) {
            std::cout << "  state_equal=" << se << " result_equal=" << re << "\n";
            if (!se) {
                std::cout << "    value_eq=" << std::ranges::equal(a.value, b.value)
                    << " second_eq=" << std::ranges::equal(a.second, b.second)
                    << " output_eq=" << std::ranges::equal(a.output, b.output)
                    << " output2_eq=" << std::ranges::equal(a.output2, b.output2) << "\n";
                for (size_t i = 0; i < a.output.size(); ++i)
                    if (a.output[i] != b.output[i]) {
                        std::cout << "    out_diff[" << i << "] raze=" << int(a.output[i])
                            << " std=" << int(b.output[i]) << "\n";
                        break;
                    }
            }
            if (!re) {
                std::cout << "    in_pos  raze=" << rtts::algorithm::position(a.value, ra.in)
                    << " std=" << rtts::algorithm::position(b.value, rb.in) << "\n";
                std::cout << "    out_pos raze=" << rtts::algorithm::position(a.output, ra.out)
                    << " std=" << rtts::algorithm::position(b.output, rb.out) << "\n";
            }
        }
        return se && re;
    }
};

RTTS_CASE("compress tables sanity") {
    auto check = [](auto& tbl, int groups, int stride, int mult) {
        for (int mask = 0; mask < (1 << groups); ++mask) {
            int kept = 0;
            for (int g = 0; g < groups; ++g)
                if (!(mask >> g & 1)) {
                    for (int o = 0; o < stride; ++o)
                        RTTS_EQUAL(int(tbl.shuffle[mask][kept * stride + o]), g * stride + o);
                    ++kept;
                }
            RTTS_EQUAL(int(tbl.size[mask]), kept * mult);
        }
        };
    check(raze::vx::tables_sse<1>, 8, 1, 1);   // u8 half
    check(raze::vx::tables_sse<2>, 8, 2, 2);   // short
    check(raze::vx::tables_avx<4>, 8, 1, 4);
    check(raze::vx::tables_avx<8>, 4, 2, 8);
};
 

RTTS_CASE("copy_if mod7 char repro big") {
    std::vector<char> in(1024, char(-64));  
    std::vector<char> a(in.size(), 0), b(in.size(), 0);
    auto ra = raze::algorithm::copy_if(in, a.begin(), mod7);
    auto rb = std::ranges::copy_if(in, b.begin(), mod7);
    std::cout << "raze kept=" << (ra.out - a.begin())
        << " std kept=" << (rb.out - b.begin()) << "\n";
    RTTS_EQUAL(ra.out - a.begin(), rb.out - b.begin());
};
 

RTTS_CASE_TPL("raze::algorithm::copy_if", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::unroll<2>](c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fstatic](c.range(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), yes); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), yes); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), no); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), no); });

    if constexpr (std::is_integral_v<T>) {
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), even); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), even); });
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), pos); }));
};

RTTS_CASE_TPL("raze::algorithm::copy_if fuzz", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    for (unsigned seed : fuzz_seeds) {
        auto cfg = fuzz_config<T>();
        cfg.seed = seed;

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), pos); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), eq42); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), eq42); });
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), ne42); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), ne42); });
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), yes); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), yes); });
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), no); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), no); });

        if constexpr (std::is_integral_v<T>) {
            run<range, T>(cfg,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), even); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out(), even); });
            run<range, T>(cfg,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), mod3); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out(), mod3); });
            run<range, T>(cfg,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), mod7); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out(), mod7); });
        }

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if[raze::options::unroll<2>](c.range(), c.out(), pos); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos); });
        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::copy_if[raze::options::fstatic](c.range(), c.out(), eq42); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), eq42); });
    }
};

RTTS_CASE_TPL("raze::algorithm::copy_if tight output", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    for (unsigned seed : fuzz_seeds) {
        tight_output_fuzz<T>(pos,  seed, 200);
        tight_output_fuzz<T>(eq42, seed, 200); 
        if constexpr (std::is_integral_v<T>)
            tight_output_fuzz<T>(even, seed, 200);
    }
};

RTTS_CASE("raze::algorithm::copy_if mixed") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); });

    run<range, uint32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), even); });

    run<range, int64_t, double>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

    run<range, int8_t, int16_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });

    run<range, int32_t, int64_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

    run<range, int16_t, int8_t>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });

    run<range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); });

    static_assert(constexpr_run_mixed<int32_t, float, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out2(), pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos); }));

    static_assert(constexpr_run_mixed<int64_t, double, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out2(), even); }));

    static_assert(constexpr_run_mixed<int8_t, int16_t, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out2(), even); }));
};

RTTS_CASE("raze::algorithm::copy_if mixed fuzz") {
    using namespace rtts::algorithm;
    if constexpr (run_mixed_fuzz) {
        for (unsigned seed : fuzz_seeds) {
            auto c32 = fuzz_config<int32_t>(); c32.seed = seed;
            run<range, int32_t, float>(c32,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });
            run<range, int32_t, int64_t>(c32,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

            auto c64 = fuzz_config<int64_t>(); c64.seed = seed;
            run<range, int64_t, double>(c64,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), pos); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

            auto c8 = fuzz_config<int8_t>(); c8.seed = seed;
            run<range, int8_t, int16_t>(c8,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });

            auto c16 = fuzz_config<int16_t>(); c16.seed = seed;
            run<range, int16_t, int8_t>(c16,
                [](auto c) { return raze::algorithm::copy_if(c.range(), c.out2(), even); },
                [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), even); });
        }
    }
};

RTTS_CASE("raze::algorithm::copy_if structs") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });

    run<range, tc_struct_8>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_even); });

    run<range, tc_struct_12>(cfg,
        [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), a_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), a_even); });

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fscalar](c.first(), c.last(), c.out(), x_pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_pos); });

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::unroll<2>](c.range(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });

    static_assert(constexpr_run<tc_struct_4, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), x_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), x_even); }));

    static_assert(constexpr_run<tc_struct_8, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_pos); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_pos); }));

    static_assert(constexpr_run<tc_struct_12, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.range(), c.out(), a_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), a_even); }));

    static_assert(constexpr_run<tc_nontrivial, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if(c.first(), c.last(), c.out(), x_even); },
        [](auto c) constexpr { return std::ranges::copy_if(c.first(), c.last(), c.out(), x_even); }));
};

RTTS_CASE("raze::algorithm::copy_if structs fuzz") {
    using namespace rtts::algorithm;
    for (unsigned seed : fuzz_seeds) {
        auto c4 = fuzz_config<tc_struct_4>(); c4.seed = seed;
        run<range, tc_struct_4>(c4,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), x_even); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });
        run<range, tc_struct_4>(c4,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), even, &tc_struct_4::x); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), even, &tc_struct_4::x); });

        auto c8 = fuzz_config<tc_struct_8>(); c8.seed = seed;
        run<range, tc_struct_8>(c8,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), x_pos); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_pos); });

        auto c12 = fuzz_config<tc_struct_12>(); c12.seed = seed;
        run<range, tc_struct_12>(c12,
            [](auto c) { return raze::algorithm::copy_if(c.range(), c.out(), a_even); },
            [](auto c) { return std::ranges::copy_if(c.range(), c.out(), a_even); });
    }
};