#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>

#include <memory>
#include <span>
#include <vector>
#include <random>

namespace {
    struct tracked_obj {
        static inline int alive = 0;
        int v = 0;
        tracked_obj() noexcept { ++alive; }
        ~tracked_obj() { --alive; }
    };
}

RTTS_CASE("raze::algorithm::destroy iterator, range, fscalar, and constexpr") {
    namespace opt = raze::options;

    {
        tracked_obj::alive = 0;
        std::allocator<tracked_obj> alloc;
        auto* p = alloc.allocate(16);

        raze::algorithm::uninitialized_value_construct(p, p + 16);
        RTTS_EQUAL(tracked_obj::alive, 16);
        auto end1 = raze::algorithm::destroy(p, p + 16);
        RTTS_EXPECT(end1 == p + 16 && tracked_obj::alive == 0);

        raze::algorithm::uninitialized_value_construct(std::span(p, 16));
        RTTS_EQUAL(tracked_obj::alive, 16);
        auto end2 = raze::algorithm::destroy[opt::fscalar](std::span(p, 16));
        RTTS_EXPECT(end2 == std::span(p, 16).end() && tracked_obj::alive == 0);

        auto end3 = raze::algorithm::destroy(p, p);
        RTTS_EXPECT(end3 == p);
        alloc.deallocate(p, 16);
    }

    static_assert([]() constexpr {
        std::allocator<int> alloc;
        int* p = alloc.allocate(4);
        for (int i = 0; i < 4; ++i) std::construct_at(p + i, i);
        auto end = raze::algorithm::destroy(p, p + 4);
        alloc.deallocate(p, 4);
        return end == p + 4;
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };
}

RTTS_CASE("raze::algorithm::destroy fuzz") {
    namespace opt = raze::options;

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            tracked_obj::alive = 0;
            const size_t n = static_cast<size_t>(rng() % 257);
            std::allocator<tracked_obj> alloc;
            auto* p = n ? alloc.allocate(n) : nullptr;

            raze::algorithm::uninitialized_value_construct(p, p + n);
            RTTS_EQUAL(tracked_obj::alive, static_cast<int>(n));

            const size_t c1 = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t c2 = n ? (c1 + static_cast<size_t>(rng() % (n - c1 + 1))) : 0;

            auto e1 = raze::algorithm::destroy(p, p + c1);
            RTTS_EXPECT(e1 == p + c1 && tracked_obj::alive == static_cast<int>(n - c1));

            auto sp2 = std::span(p + c1, c2 - c1);
            auto e2 = raze::algorithm::destroy[opt::fscalar](sp2);
            RTTS_EXPECT(e2 == sp2.end() && tracked_obj::alive == static_cast<int>(n - c2));

            auto e3 = raze::algorithm::destroy[opt::fstatic](p + c2, p + n);
            RTTS_EXPECT(e3 == p + n && tracked_obj::alive == 0);

            if (p) alloc.deallocate(p, n);
        }
    }
};
