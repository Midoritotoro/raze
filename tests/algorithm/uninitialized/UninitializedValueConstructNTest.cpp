#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstructN.h>

#include <cstring>
#include <memory>
#include <stdexcept>

namespace {
    struct val_n_throw_obj {
        static inline int alive = 0;
        static inline int count = 0;
        static inline int fail_at = -1;
        int x;

        val_n_throw_obj() : x(66) {
            if (fail_at >= 0 && count == fail_at)
                throw std::runtime_error("value_n ctor fail");
            ++count;
            ++alive;
        }
        ~val_n_throw_obj() { --alive; }
    };
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_value_construct_n trivial", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    std::allocator<T> alloc;
    constexpr size_t n = 32;
    T* p = alloc.allocate(n);

    std::memset(p, 0xEF, n * sizeof(T));
    auto end = raze::algorithm::uninitialized_value_construct_n[opt::fscalar](p, static_cast<std::ptrdiff_t>(n));
    RTTS_EXPECT(end == p + n);
    for (size_t i = 0; i < n; ++i) RTTS_EQUAL(p[i], T{});

    alloc.deallocate(p, n);
};

RTTS_CASE("raze::algorithm::uninitialized_value_construct_n non-trivial and exception rollback") {
    val_n_throw_obj::alive = 0;
    val_n_throw_obj::count = 0;
    val_n_throw_obj::fail_at = 3;

    std::allocator<val_n_throw_obj> alloc;
    auto* p = alloc.allocate(8);
    bool caught = false;
    try {
        raze::algorithm::uninitialized_value_construct_n(p, 8);
    }
    catch (const std::runtime_error&) {
        caught = true;
    }
    RTTS_EXPECT(caught && val_n_throw_obj::alive == 0);

    val_n_throw_obj::fail_at = -1;
    auto end = raze::algorithm::uninitialized_value_construct_n(p, 8);
    RTTS_EXPECT(end == p + 8 && val_n_throw_obj::alive == 8);
    raze::algorithm::destroy_n(p, 8);
    RTTS_EQUAL(val_n_throw_obj::alive, 0);
    alloc.deallocate(p, 8);
};

