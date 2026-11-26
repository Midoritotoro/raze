#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>

#include <memory>
#include <span>
#include <vector>

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

        // Empty range
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

