#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>

#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

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

    template <class T>
    void check_uninit_val(size_t n, int fill_byte, unsigned mode) {
        namespace opt = raze::options;
        std::allocator<T> alloc;
        T* p = n ? alloc.allocate(n) : nullptr;
        if (n) std::memset(p, fill_byte, n * sizeof(T));
        auto sp = std::span(p, n);

        if (mode == 0u)      RTTS_EXPECT(raze::algorithm::uninitialized_value_construct(p, p + n) == p + n);
        else if (mode == 1u) RTTS_EXPECT(raze::algorithm::uninitialized_value_construct(sp) == sp.end());
        else if (mode == 2u) RTTS_EXPECT(raze::algorithm::uninitialized_value_construct[opt::fscalar](sp) == sp.end());
        else                 RTTS_EXPECT(raze::algorithm::uninitialized_value_construct[opt::fstatic](p, p + n) == p + n);

        for (size_t i = 0; i < n; ++i)
            RTTS_EQUAL(p[i], T{});
        raze::algorithm::destroy(p, p + n);
        if (p) alloc.deallocate(p, n);
    }

    void check_val_rollback(size_t n, int fail_at, bool use_scalar) {
        namespace opt = raze::options;
        val_throw_obj::alive = 0;
        val_throw_obj::count = 0;
        val_throw_obj::fail_at = fail_at;
        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < n);

        std::allocator<val_throw_obj> alloc;
        auto* p = n ? alloc.allocate(n) : nullptr;
        auto sp = std::span(p, n);

        bool caught = false;
        try {
            if (!use_scalar) RTTS_EXPECT(raze::algorithm::uninitialized_value_construct(p, p + n) == p + n);
            else             RTTS_EXPECT(raze::algorithm::uninitialized_value_construct[opt::fscalar](sp) == sp.end());
            RTTS_EXPECT(!should_throw);
            RTTS_EQUAL(val_throw_obj::alive, static_cast<int>(n));
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(p[i].x, 55);
            raze::algorithm::destroy(p, p + n);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(val_throw_obj::alive, 0);
        if (p) alloc.deallocate(p, n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_value_construct", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    check_uninit_val<T>(32, 0xAB, 0u);
    check_uninit_val<T>(32, 0xCD, 2u);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it)
            check_uninit_val<T>(static_cast<size_t>(rng() % 257), static_cast<int>(0x5Au ^ (it & 0xFFu)), static_cast<unsigned>(it % 4));
    }
};

RTTS_CASE("raze::algorithm::uninitialized_value_construct non-trivial and exception rollback") {
    check_val_rollback(8, 4, false);
    check_val_rollback(8, -1, true);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            const size_t n = static_cast<size_t>(rng() % 65);
            const bool should_throw = (n > 0) && ((it % 3) != 2);
            check_val_rollback(n, should_throw ? static_cast<int>(rng() % n) : -1, (it & 1u) != 0u);
        }
    }
};
