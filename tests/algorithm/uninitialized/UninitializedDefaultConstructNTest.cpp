#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstructN.h>

#include <memory>
#include <stdexcept>

namespace {
    struct default_n_throw_obj {
        static inline int alive = 0;
        static inline int count = 0;
        static inline int fail_at = -1;
        int marker = 88;

        default_n_throw_obj() {
            if (fail_at >= 0 && count == fail_at)
                throw std::runtime_error("default_n ctor fail");
            ++count;
            ++alive;
        }
        ~default_n_throw_obj() { --alive; }
    };
}

RTTS_CASE("raze::algorithm::uninitialized_default_construct_n lifetime and exception rollback") {
    namespace opt = raze::options;

    {
        default_n_throw_obj::alive = 0;
        default_n_throw_obj::count = 0;
        default_n_throw_obj::fail_at = -1;

        std::allocator<default_n_throw_obj> alloc;
        auto* p = alloc.allocate(8);

        auto end = raze::algorithm::uninitialized_default_construct_n(p, 8);
        RTTS_EXPECT(end == p + 8 && default_n_throw_obj::alive == 8);
        for (int i = 0; i < 8; ++i) RTTS_EQUAL(p[i].marker, 88);
        raze::algorithm::destroy_n(p, 8);
        RTTS_EQUAL(default_n_throw_obj::alive, 0);

        RTTS_EXPECT(raze::algorithm::uninitialized_default_construct_n[opt::fscalar](p, 0) == p);
        RTTS_EQUAL(default_n_throw_obj::alive, 0);
        alloc.deallocate(p, 8);
    }

    {
        default_n_throw_obj::alive = 0;
        default_n_throw_obj::count = 0;
        default_n_throw_obj::fail_at = 3;

        std::allocator<default_n_throw_obj> alloc;
        auto* p = alloc.allocate(8);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_default_construct_n(p, 8);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught);
        RTTS_EQUAL(default_n_throw_obj::alive, 0);
        alloc.deallocate(p, 8);
    }
};

