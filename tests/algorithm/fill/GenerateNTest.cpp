#include <tests/rts/rts.h>
#include <raze/algorithm/fill/GenerateN.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::generate_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto make_gen = []() {
        int counter = 0;
        return [counter]() mutable -> T { return static_cast<T>((++counter) % 97); };
    };

    run<n, T>(cfg,
        [make_gen](auto c) {
            return raze::algorithm::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        },
        [make_gen](auto c) {
            return std::ranges::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        });

    run<n, T>(cfg,
        [make_gen](auto c) {
            return raze::algorithm::generate_n[opt::fscalar](c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        },
        [make_gen](auto c) {
            return std::ranges::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        });

    static_assert([]() constexpr {
        std::array<T, 5> a{};
        int n = 10;
        auto end = raze::algorithm::generate_n(a.begin(), 3, [&n]() constexpr -> T { return static_cast<T>(n++); });
        auto z = raze::algorithm::generate_n(a.begin(), 0, [&n]() constexpr -> T { return static_cast<T>(n++); });
        auto neg = raze::algorithm::generate_n(a.begin(), -5, [&n]() constexpr -> T { return static_cast<T>(n++); });
        return end == a.begin() + 3 && z == a.begin() && neg == a.begin()
            && a[0] == T(10) && a[1] == T(11) && a[2] == T(12) && a[3] == T(0);
    }());
};

RTTS_CASE("raze::algorithm::generate_n output iterators and negative counts") {
    std::vector<int> out;
    int x = 1;
    raze::algorithm::generate_n(std::back_inserter(out), 5, [&x]() {
        const int cur = x++;
        return cur * cur;
    });
    RTTS_EXPECT((out == std::vector<int>{ 1, 4, 9, 16, 25 }));

    int calls = 0;
    raze::algorithm::generate_n(std::back_inserter(out), -3, [&calls]() { return ++calls; });
    RTTS_EQUAL(calls, 0);
    RTTS_EQUAL(out.size(), size_t(5));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void generate_n_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 513);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const std::ptrdiff_t max_cnt = static_cast<std::ptrdiff_t>(n - lo);
            const std::ptrdiff_t cnt = (it % 7 == 0)
                ? -static_cast<std::ptrdiff_t>(rng() % 6)
                : (max_cnt > 0 ? static_cast<std::ptrdiff_t>(rng() % static_cast<unsigned>(max_cnt + 1)) : 0);
            const unsigned gen_seed = rng();

            auto make_gen = [gen_seed]() {
                std::mt19937 g(gen_seed);
                return [g]() mutable -> T { return static_cast<T>(g() % 97u); };
            };

            auto v1 = base, v2 = base, v3 = base, v_exp = base;
            auto re = std::ranges::generate_n(v_exp.begin() + static_cast<std::ptrdiff_t>(lo), cnt, make_gen());
            auto r1 = raze::algorithm::generate_n(v1.begin() + static_cast<std::ptrdiff_t>(lo), cnt, make_gen());
            auto r2 = raze::algorithm::generate_n[opt::fscalar](v2.begin() + static_cast<std::ptrdiff_t>(lo), cnt, make_gen());
            auto r3 = raze::algorithm::generate_n[opt::fstatic](v3.begin() + static_cast<std::ptrdiff_t>(lo), cnt, make_gen());

            RTTS_EXPECT(r1 - v1.begin() == re - v_exp.begin());
            RTTS_EXPECT(r2 - v2.begin() == re - v_exp.begin());
            RTTS_EXPECT(r3 - v3.begin() == re - v_exp.begin());
            RTTS_ALL_EQUAL(v1, v_exp);
            RTTS_ALL_EQUAL(v2, v_exp);
            RTTS_ALL_EQUAL(v3, v_exp);

            if (it % 8 == 0 && n <= 64) {
                std::list<T> lst(base.begin(), base.end());
                std::list<T> lst_exp(base.begin(), base.end());
                const std::ptrdiff_t lcnt = (cnt > 0 && static_cast<size_t>(cnt) <= n) ? cnt : 0;
                raze::algorithm::generate_n(lst.begin(), lcnt, make_gen());
                std::ranges::generate_n(lst_exp.begin(), lcnt, make_gen());
                RTTS_ALL_EQUAL(lst, lst_exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::generate_n fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        generate_n_fuzz<T>(seed, 40);
};
