#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

#include <memory>
#include <new>

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

    // Single object
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

    // Array object
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

