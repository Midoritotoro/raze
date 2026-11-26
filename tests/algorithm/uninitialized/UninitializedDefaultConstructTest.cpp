#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstruct.h>

#include <memory>
#include <span>
#include <stdexcept>

namespace {
    struct default_throw_obj {
        static inline int alive = 0;
        static inline int count = 0;
        static inline int fail_at = -1;
        int marker = 77;

        default_throw_obj() {
            if (fail_at >= 0 && count == fail_at)
                throw std::runtime_error("default ctor fail");
            ++count;
            ++alive;
        }
        ~default_throw_obj() { --alive; }
    };
}

RTTS_CASE("raze::algorithm::uninitialized_default_construct lifetime and exception rollback") {
    namespace opt = raze::options;

    // Normal construction (iterator + range + fscalar)
    {
        default_throw_obj::alive = 0;
        default_throw_obj::count = 0;
        default_throw_obj::fail_at = -1;

        std::allocator<default_throw_obj> alloc;
        auto* p = alloc.allocate(8);

        auto end1 = raze::algorithm::uninitialized_default_construct(p, p + 8);
        RTTS_EXPECT(end1 == p + 8 && default_throw_obj::alive == 8);
        for (int i = 0; i < 8; ++i) RTTS_EQUAL(p[i].marker, 77);
        raze::algorithm::destroy(p, p + 8);
        RTTS_EQUAL(default_throw_obj::alive, 0);

        auto sp = std::span(p, 8);
        auto end2 = raze::algorithm::uninitialized_default_construct[opt::fscalar](sp);
        RTTS_EXPECT(end2 == sp.end() && default_throw_obj::alive == 8);
        raze::algorithm::destroy(sp);
        RTTS_EQUAL(default_throw_obj::alive, 0);
        alloc.deallocate(p, 8);
    }

    // Strong exception rollback
    {
        default_throw_obj::alive = 0;
        default_throw_obj::count = 0;
        default_throw_obj::fail_at = 5;

        std::allocator<default_throw_obj> alloc;
        auto* p = alloc.allocate(10);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_default_construct(p, p + 10);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught);
        RTTS_EQUAL(default_throw_obj::alive, 0);
        alloc.deallocate(p, 10);
    }
};

