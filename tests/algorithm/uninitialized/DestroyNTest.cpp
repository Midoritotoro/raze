#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstructN.h>

#include <memory>
#include <random>

namespace {
    struct tracked_n_obj {
        static inline int alive = 0;
        tracked_n_obj() noexcept { ++alive; }
        ~tracked_n_obj() { --alive; }
    };
}

RTTS_CASE("raze::algorithm::destroy_n counts, fscalar, and constexpr") {
    namespace opt = raze::options;

    {
        tracked_n_obj::alive = 0;
        std::allocator<tracked_n_obj> alloc;
        auto* p = alloc.allocate(10);

        raze::algorithm::uninitialized_value_construct_n(p, 10);
        RTTS_EQUAL(tracked_n_obj::alive, 10);

        auto mid = raze::algorithm::destroy_n(p, 4);
        RTTS_EXPECT(mid == p + 4 && tracked_n_obj::alive == 6);

        auto end = raze::algorithm::destroy_n[opt::fscalar](mid, 6);
        RTTS_EXPECT(end == p + 10 && tracked_n_obj::alive == 0);

        RTTS_EXPECT(raze::algorithm::destroy_n(p, 0) == p);
        alloc.deallocate(p, 10);
    }

    static_assert([]() constexpr {
        std::allocator<int> alloc;
        int* p = alloc.allocate(3);
        for (int i = 0; i < 3; ++i) std::construct_at(p + i, i);
        auto end = raze::algorithm::destroy_n(p, 3);
        alloc.deallocate(p, 3);
        return end == p + 3;
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };
}

RTTS_CASE("raze::algorithm::destroy_n fuzz") {
    namespace opt = raze::options;

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            tracked_n_obj::alive = 0;
            const size_t n = static_cast<size_t>(rng() % 257);
            std::allocator<tracked_n_obj> alloc;
            auto* p = n ? alloc.allocate(n) : nullptr;

            raze::algorithm::uninitialized_value_construct_n(p, static_cast<std::ptrdiff_t>(n));
            RTTS_EQUAL(tracked_n_obj::alive, static_cast<int>(n));

            size_t pos = 0;
            size_t step = 0;
            while (pos < n) {
                const size_t rem = n - pos;
                const size_t chunk = (step % 5 == 0) ? 0u : static_cast<size_t>(rng() % (rem + 1));
                auto* res = ((step % 3) == 0) ? raze::algorithm::destroy_n(p + pos, static_cast<std::ptrdiff_t>(chunk))
                          : ((step % 3) == 1) ? raze::algorithm::destroy_n[opt::fscalar](p + pos, static_cast<std::ptrdiff_t>(chunk))
                                              : raze::algorithm::destroy_n[opt::fstatic](p + pos, static_cast<std::ptrdiff_t>(chunk));
                RTTS_EXPECT(res == p + pos + chunk);
                pos += chunk;
                RTTS_EQUAL(tracked_n_obj::alive, static_cast<int>(n - pos));
                ++step;
            }
            RTTS_EXPECT(raze::algorithm::destroy_n(p + n, 0) == p + n);
            RTTS_EQUAL(tracked_n_obj::alive, 0);

            if (p) alloc.deallocate(p, n);
        }
    }
};
