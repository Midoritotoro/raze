#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedFill.h>

#include <memory>
#include <span>
#include <stdexcept>

namespace {
    struct fill_throw_obj {
        static inline int alive = 0;
        static inline int copies = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit fill_throw_obj(int v) noexcept : val(v) { ++alive; }
        fill_throw_obj(const fill_throw_obj& o) : val(o.val) {
            if (fail_at >= 0 && copies == fail_at)
                throw std::runtime_error("fill fail");
            ++copies;
            ++alive;
        }
        ~fill_throw_obj() { --alive; }
    };

    struct pair_box {
        int a;
        int b;
    };
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_fill", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto end1 = raze::algorithm::uninitialized_fill(dst, dst + sz, T(42));
        RTTS_EXPECT(end1 == dst + sz);
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], T(42));
        raze::algorithm::destroy(dst, dst + sz);

        auto sp = std::span(dst, sz);
        auto end2 = raze::algorithm::uninitialized_fill[opt::fscalar](sp, T(77));
        RTTS_EXPECT(end2 == sp.end());
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], T(77));
        raze::algorithm::destroy(sp);

        if (dst) alloc.deallocate(dst, sz);
    }
};

RTTS_CASE("raze::algorithm::uninitialized_fill P2248 braced-init and exception rollback") {
    // C++26 P2248: braced-init-list value
    {
        std::allocator<pair_box> alloc;
        auto* p = alloc.allocate(4);
        raze::algorithm::uninitialized_fill(std::span(p, 4), { 10, 20 });
        for (int i = 0; i < 4; ++i) RTTS_EXPECT(p[i].a == 10 && p[i].b == 20);
        raze::algorithm::destroy(p, p + 4);
        alloc.deallocate(p, 4);
    }

    // Exception rollback
    {
        fill_throw_obj::alive = 0;
        fill_throw_obj::copies = 0;
        fill_throw_obj::fail_at = -1;

        fill_throw_obj proto(99);
        const int base_alive = fill_throw_obj::alive;

        fill_throw_obj::copies = 0;
        fill_throw_obj::fail_at = 3;
        std::allocator<fill_throw_obj> alloc;
        auto* dst = alloc.allocate(6);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_fill(dst, dst + 6, proto);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && fill_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 6);
    }
};

