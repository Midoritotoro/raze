#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

#include <memory>
#include <new>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace {
    struct tracked_destroy {
        static inline int alive = 0;
        int val = 0;
        explicit tracked_destroy(int v = 0) noexcept : val(v) { ++alive; }
        ~tracked_destroy() { --alive; }
    };
}

RTTS_CASE("raze::algorithm::destroy_at scalar, array, and constexpr") {
    namespace opt = raze::options;

    {
        tracked_destroy::alive = 0;
        std::allocator<tracked_destroy> alloc;
        auto* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, 42);
        RTTS_EQUAL(tracked_destroy::alive, 1);
        RTTS_EQUAL(p->val, 42);

        raze::algorithm::destroy_at(p);
        RTTS_EQUAL(tracked_destroy::alive, 0);

        raze::algorithm::construct_at(p, 99);
        raze::algorithm::destroy_at[opt::fscalar](p);
        RTTS_EQUAL(tracked_destroy::alive, 0);
        alloc.deallocate(p, 1);
    }

    {
        tracked_destroy::alive = 0;
        alignas(tracked_destroy) std::byte buf[sizeof(tracked_destroy[4])];
        auto* arr = ::new (static_cast<void*>(buf)) tracked_destroy[4]{ tracked_destroy(1), tracked_destroy(2), tracked_destroy(3), tracked_destroy(4) };
        RTTS_EQUAL(tracked_destroy::alive, 4);
        raze::algorithm::destroy_at(reinterpret_cast<tracked_destroy(*)[4]>(arr));
        RTTS_EQUAL(tracked_destroy::alive, 0);
    }

    static_assert([]() constexpr {
        std::allocator<int> alloc;
        int* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, 123);
        const int v = *p;
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
        return v == 123;
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };
}

RTTS_CASE("raze::algorithm::destroy_at fuzz") {
    namespace opt = raze::options;

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            tracked_destroy::alive = 0;
            const size_t n = 1u + static_cast<size_t>(rng() % 128);
            std::allocator<tracked_destroy> alloc;
            auto* p = alloc.allocate(n);

            for (size_t i = 0; i < n; ++i)
                raze::algorithm::construct_at(p + i, static_cast<int>(rng() % 1000u));
            RTTS_EQUAL(tracked_destroy::alive, static_cast<int>(n));

            std::vector<size_t> order(n);
            std::iota(order.begin(), order.end(), size_t(0));
            std::ranges::shuffle(order, rng);

            int remaining = static_cast<int>(n);
            for (size_t k = 0; k < n; ++k) {
                const size_t idx = order[k];
                if ((k % 3) == 0)      raze::algorithm::destroy_at(p + idx);
                else if ((k % 3) == 1) raze::algorithm::destroy_at[opt::fscalar](p + idx);
                else                   raze::algorithm::destroy_at[opt::fstatic](p + idx);
                --remaining;
                RTTS_EQUAL(tracked_destroy::alive, remaining);
            }
            alloc.deallocate(p, n);
        }
    }
};
