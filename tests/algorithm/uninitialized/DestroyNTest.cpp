#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstructN.h>

#include <memory>

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

