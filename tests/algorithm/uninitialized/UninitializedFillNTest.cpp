#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedFillN.h>

#include <memory>
#include <stdexcept>

namespace {
    struct fill_n_throw_obj {
        static inline int alive = 0;
        static inline int copies = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit fill_n_throw_obj(int v) noexcept : val(v) { ++alive; }
        fill_n_throw_obj(const fill_n_throw_obj& o) : val(o.val) {
            if (fail_at >= 0 && copies == fail_at)
                throw std::runtime_error("fill_n fail");
            ++copies;
            ++alive;
        }
        ~fill_n_throw_obj() { --alive; }
    };

    struct pair_box {
        int a;
        int b;
    };
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_fill_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto end = raze::algorithm::uninitialized_fill_n[opt::fscalar](dst, static_cast<std::ptrdiff_t>(sz), T(33));
        RTTS_EXPECT(end == dst + sz);
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], T(33));
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(sz));

        if (dst) alloc.deallocate(dst, sz);
    }
};

RTTS_CASE("raze::algorithm::uninitialized_fill_n P2248 braced-init and exception rollback") {
    {
        std::allocator<pair_box> alloc;
        auto* p = alloc.allocate(4);
        auto end = raze::algorithm::uninitialized_fill_n(p, 4, { 3, 7 });
        RTTS_EXPECT(end == p + 4);
        for (int i = 0; i < 4; ++i) RTTS_EXPECT(p[i].a == 3 && p[i].b == 7);
        raze::algorithm::destroy_n(p, 4);
        alloc.deallocate(p, 4);
    }

    {
        fill_n_throw_obj::alive = 0;
        fill_n_throw_obj::copies = 0;
        fill_n_throw_obj::fail_at = -1;

        fill_n_throw_obj proto(11);
        const int base_alive = fill_n_throw_obj::alive;

        fill_n_throw_obj::copies = 0;
        fill_n_throw_obj::fail_at = 2;
        std::allocator<fill_n_throw_obj> alloc;
        auto* dst = alloc.allocate(6);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_fill_n(dst, 6, proto);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && fill_n_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 6);
    }
};

