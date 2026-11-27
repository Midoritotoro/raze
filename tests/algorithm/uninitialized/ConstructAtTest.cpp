#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace {
    struct multi_arg_obj {
        int a;
        double b;
        std::string s;
        multi_arg_obj(int x, double y, std::string z) : a(x), b(y), s(std::move(z)) {}
    };
}

RTTS_CASE("raze::algorithm::construct_at multi-arg, move-only, and constexpr") {
    namespace opt = raze::options;

    {
        std::allocator<multi_arg_obj> alloc;
        auto* p = alloc.allocate(1);
        auto* r = raze::algorithm::construct_at(p, 7, 3.5, std::string("raze"));
        RTTS_EXPECT(r == p);
        RTTS_EQUAL(p->a, 7);
        RTTS_EQUAL(p->b, 3.5);
        RTTS_EQUAL(p->s, std::string("raze"));
        raze::algorithm::destroy_at(p);

        auto* r2 = raze::algorithm::construct_at[opt::fscalar](p, 9, 1.5, std::string("scalar"));
        RTTS_EXPECT(r2 == p && p->a == 9 && p->s == "scalar");
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
    }

    {
        std::allocator<std::unique_ptr<int>> alloc;
        auto* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, std::make_unique<int>(55));
        RTTS_EQUAL(**p, 55);
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
    }

    static_assert([]() constexpr {
        std::allocator<std::pair<int, int>> alloc;
        auto* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, 10, 20);
        const int sum = p->first + p->second;
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
        return sum == 30;
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void construct_at_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = 1u + static_cast<size_t>(rng() % 128);
            const auto vals = rtts::random::vector<T>(n, rng());
            std::vector<size_t> order(n);
            std::iota(order.begin(), order.end(), size_t(0));
            std::ranges::shuffle(order, rng);

            std::allocator<T> alloc;
            T* buf = alloc.allocate(n);

            for (size_t k = 0; k < n; ++k) {
                const size_t idx = order[k];
                T* r = ((k % 3) == 0) ? raze::algorithm::construct_at(buf + idx, vals[idx])
                     : ((k % 3) == 1) ? raze::algorithm::construct_at[opt::fscalar](buf + idx, vals[idx])
                                      : raze::algorithm::construct_at[opt::fstatic](buf + idx, vals[idx]);
                RTTS_EXPECT(r == buf + idx);
            }
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(buf[i], vals[i]);

            std::ranges::shuffle(order, rng);
            for (size_t idx : order)
                raze::algorithm::destroy_at(buf + idx);
            alloc.deallocate(buf, n);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::construct_at fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        construct_at_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::construct_at multi-arg fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 30; ++it) {
            const size_t n = 1u + static_cast<size_t>(rng() % 64);
            std::allocator<multi_arg_obj> alloc;
            auto* p = alloc.allocate(n);
            std::vector<int> xs(n);
            for (size_t i = 0; i < n; ++i) {
                xs[i] = static_cast<int>(rng() % 10000u) - 5000;
                auto* r = raze::algorithm::construct_at(p + i, xs[i], static_cast<double>(xs[i]) * 0.5,
                                                        std::string("s") + std::to_string(xs[i]));
                RTTS_EXPECT(r == p + i);
            }
            for (size_t i = 0; i < n; ++i) {
                RTTS_EQUAL(p[i].a, xs[i]);
                RTTS_EQUAL(p[i].b, static_cast<double>(xs[i]) * 0.5);
                RTTS_EQUAL(p[i].s, std::string("s") + std::to_string(xs[i]));
                raze::algorithm::destroy_at(p + i);
            }
            alloc.deallocate(p, n);
        }
    }
};
