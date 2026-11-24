#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyIf.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <vector>


struct tc_struct_4 { int32_t x;           bool operator==(const tc_struct_4&)  const = default; };
struct tc_struct_8 { int64_t x;           bool operator==(const tc_struct_8&)  const = default; };
struct tc_struct_12 { int32_t a = 0, b = 0, c = 0; bool operator==(const tc_struct_12&) const = default; };

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
    constexpr auto pos_branching = [](auto v) { if (v > decltype(v)(0)) return true; return false; };
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

    inline constexpr bool run_mixed_fuzz = true;

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

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            break;
        }
    }
    RTTS_EQUAL(ra.out - a.begin(), rb.out - b.begin());
    RTTS_ALL_EQUAL(a, b);
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
    check(raze::vx::tables_sse<1>, 8, 1, 1);  
    check(raze::vx::tables_sse<2>, 8, 2, 2);  
    check(raze::vx::tables_avx<4>, 8, 1, 4);
    check(raze::vx::tables_avx<8>, 4, 2, 8);
};
 

RTTS_CASE("copy_if mod7 char repro big") {
    std::vector<char> in(1024, char(-64));  
    std::vector<char> a(in.size(), 0), b(in.size(), 0);
    auto ra = raze::algorithm::copy_if(in, a.begin(), mod7);
    auto rb = std::ranges::copy_if(in, b.begin(), mod7);
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
        [](auto c) { return raze::algorithm::copy_if[raze::options::fautovec](c.first(), c.last(), c.out(), pos); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pos); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fautovec](c.range(), c.out(), pos_branching); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos_branching); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fautovec][raze::options::fstatic](c.range(), c.out(), pos_branching); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), pos_branching); });

    {
        namespace opt = raze::options;
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo, auto pred) {
            run_aligned<A1, AOut, range, T>(acfg,
                [algo, pred](auto c) { return algo(c.first(), c.last(), c.out(), pred); },
                [pred](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out(), pred); });

            run_aligned<A1, AOut, range, T>(acfg,
                [algo, pred](auto c) { return algo(c.range(), c.out(), pred); },
                [pred](auto c) { return std::ranges::copy_if(c.range(), c.out(), pred); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::copy_if[opt::aligned<16>], pos);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_if[opt::aligned<32>], pos);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_if[opt::aligned<32>][opt::unroll<2>], pos);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_if[opt::aligned<64>], pos);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_if[opt::aligned<64>][opt::fautovec], pos_branching);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_if[opt::aligned<64>][opt::fscalar], pos);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_if[opt::aligned<opt::unaligned>], pos);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_if[opt::aligned<64>][opt::aligned<opt::unaligned>], pos);
        check_aligned.template operator()<64, 1>(raze::algorithm::copy_if[opt::aligned<64, opt::unaligned>], pos);
        check_aligned.template operator()<1, 64>(raze::algorithm::copy_if[opt::aligned<opt::unaligned, 64>], pos);
        check_aligned.template operator()<32, 64>(raze::algorithm::copy_if[opt::aligned<32, 64>][opt::fstatic], pos);
        check_aligned.template operator()<64, 16>(raze::algorithm::copy_if[opt::aligned<64, 16>], pos);
    }

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

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if[raze::options::fautovec](c.range(), c.out(), pos_branching); },
        [](auto c) constexpr { return std::ranges::copy_if(c.range(), c.out(), pos_branching); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_if[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), pos); },
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

    run<range, int32_t, float>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fautovec](c.first(), c.last(), c.out2(), pos_branching); },
        [](auto c) { return std::ranges::copy_if(c.first(), c.last(), c.out2(), pos_branching); });

    run_aligned_mixed<64, 1, range, int32_t, float>(config::compact(),
        [](auto c) { return raze::algorithm::copy_if[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out2(), pos); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out2(), pos); });

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

    run<range, tc_struct_4>(cfg,
        [](auto c) { return raze::algorithm::copy_if[raze::options::fautovec](c.range(), c.out(), x_even); },
        [](auto c) { return std::ranges::copy_if(c.range(), c.out(), x_even); });

    run_aligned<64, 1, range, tc_struct_4>(config::compact(),
        [](auto c) { return raze::algorithm::copy_if[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out(), x_even); },
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


RTTS_CASE("raze::algorithm::copy_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        const std::vector<int> src = { -5, -2, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        auto gt3 = [](auto x) { return x > 3; };
        auto gt3_branch = [](auto x) { if (x > 3) return true; return false; };

        std::vector<int> r1, s1;
        raze::algorithm::copy_if(views::iota(-5, 25), std::back_inserter(r1), gt3);
        std::ranges::copy_if(views::iota(-5, 25), std::back_inserter(s1), gt3);
        RTTS_EXPECT(r1 == s1);

        std::vector<int> r2, s2;
        raze::algorithm::copy_if[opt::fautovec][opt::fstatic](
            src | views::drop(2) | views::take(12), std::back_inserter(r2), gt3_branch);
        std::ranges::copy_if(src | views::drop(2) | views::take(12), std::back_inserter(s2), gt3_branch);
        RTTS_EXPECT(r2 == s2);

        auto chain = src | views::reverse | views::filter([](int x) { return x > 0; }) | views::transform([](int x) { return x * 2; });
        std::vector<int> r3, s3;
        raze::algorithm::copy_if[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(r3), [](int x) { return x % 4 == 0; });
        std::ranges::copy_if(chain, std::back_inserter(s3), [](int x) { return x % 4 == 0; });
        RTTS_EXPECT(r3 == s3);

        std::vector<int> r4, s4;
        raze::algorithm::copy_if(rtts::make_non_common(src), std::back_inserter(r4), gt3);
        std::ranges::copy_if(rtts::make_non_common(src), std::back_inserter(s4), gt3);
        RTTS_EXPECT(r4 == s4);
    }

    // Output iterators: back_inserter, front_inserter, inserter, single-pass, non-destructive string copy
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8 };
        auto is_even = [](auto x) { return (x & 1) == 0; };

        std::deque<int> f1, f2;
        raze::algorithm::copy_if(src, std::front_inserter(f1), is_even);
        std::ranges::copy_if(src, std::front_inserter(f2), is_even);
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 99, 100 }, i2 = { 99, 100 };
        raze::algorithm::copy_if[opt::fscalar](src, std::inserter(i1, i1.begin() + 1), is_even);
        std::ranges::copy_if(src, std::inserter(i2, i2.begin() + 1), is_even);
        RTTS_EXPECT(i1 == i2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::copy_if(spr, std::back_inserter(sp_out), is_even);
        RTTS_EXPECT((sp_out == std::vector<int>{ 2, 4, 6, 8 }) && spr.reads() == src.size());

        // Ensure copy_if copies and does NOT move out of source
        std::vector<std::string> words = { "alpha", "be", "gamma", "delta" };
        std::vector<std::string> kept;
        raze::algorithm::copy_if(words, std::back_inserter(kept), [](const std::string& s) { return s.size() >= 5; });
        RTTS_EXPECT((kept == std::vector<std::string>{ "alpha", "gamma", "delta" }));
        RTTS_EXPECT((words == std::vector<std::string>{ "alpha", "be", "gamma", "delta" }));
    }

    // Volatile, empty, 1-elem, rvalue dangling, pointer-to-member projection
    {
        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) { vsrc[i] = i; vdst[i] = -1; }
        auto vr = raze::algorithm::copy_if(vsrc, vsrc + 16, vdst, [](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(vr.out == vdst + 8);
        for (int i = 0; i < 8; ++i) RTTS_EXPECT(vdst[i] == i * 2);

        std::vector<int> empty_v, empty_out;
        raze::algorithm::copy_if(empty_v, std::back_inserter(empty_out), [](auto x) { return x > 0; });
        RTTS_EXPECT(empty_out.empty());

        std::vector<int> rdst;
        auto rres = raze::algorithm::copy_if(std::vector<int>{ 1, 2, 3, 4 }, std::back_inserter(rdst), [](auto x) { return (x & 1) == 0; });
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 2, 4 }));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 4, 40 }, { 2, 20 }, { 5, 50 } };
        std::vector<rtts::test_item> out_items;
        raze::algorithm::copy_if(items, std::back_inserter(out_items), [](int k) { return k >= 4; }, &rtts::test_item::key);
        RTTS_EXPECT(out_items.size() == 2 && out_items[0].key == 4 && out_items[1].key == 5);
    }
};