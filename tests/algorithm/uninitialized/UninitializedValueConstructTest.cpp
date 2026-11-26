#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>

#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>

namespace {
    struct val_throw_obj {
        static inline int alive = 0;
        static inline int count = 0;
        static inline int fail_at = -1;
        int x;

        val_throw_obj() : x(55) {
            if (fail_at >= 0 && count == fail_at)
                throw std::runtime_error("value ctor fail");
            ++count;
            ++alive;
        }
        ~val_throw_obj() { --alive; }
    };
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_value_construct trivial zero-init", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    std::allocator<T> alloc;
    constexpr size_t n = 32;
    T* p = alloc.allocate(n);

    std::memset(p, 0xAB, n * sizeof(T));
    auto end = raze::algorithm::uninitialized_value_construct(p, p + n);
    RTTS_EXPECT(end == p + n);
    for (size_t i = 0; i < n; ++i) RTTS_EQUAL(p[i], T{});

    std::memset(p, 0xCD, n * sizeof(T));
    auto sp = std::span(p, n);
    auto end2 = raze::algorithm::uninitialized_value_construct[opt::fscalar](sp);
    RTTS_EXPECT(end2 == sp.end());
    for (size_t i = 0; i < n; ++i) RTTS_EQUAL(p[i], T{});

    alloc.deallocate(p, n);
};

RTTS_CASE("raze::algorithm::uninitialized_value_construct non-trivial and exception rollback") {
    val_throw_obj::alive = 0;
    val_throw_obj::count = 0;
    val_throw_obj::fail_at = 4;

    std::allocator<val_throw_obj> alloc;
    auto* p = alloc.allocate(8);
    bool caught = false;
    try {
        raze::algorithm::uninitialized_value_construct(p, p + 8);
    }
    catch (const std::runtime_error&) {
        caught = true;
    }
    RTTS_EXPECT(caught && val_throw_obj::alive == 0);

    val_throw_obj::fail_at = -1;
    raze::algorithm::uninitialized_value_construct(std::span(p, 8));
    RTTS_EQUAL(val_throw_obj::alive, 8);
    for (int i = 0; i < 8; ++i) RTTS_EQUAL(p[i].x, 55);
    raze::algorithm::destroy(p, p + 8);
    RTTS_EQUAL(val_throw_obj::alive, 0);
    alloc.deallocate(p, 8);
};

