#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstruct.h>

#include <memory>
#include <span>
#include <stdexcept>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

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

    void check_default_rollback(size_t n, int fail_at, bool use_scalar) {
        namespace opt = raze::options;
        default_throw_obj::alive = 0;
        default_throw_obj::count = 0;
        default_throw_obj::fail_at = fail_at;
        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < n);

        std::allocator<default_throw_obj> alloc;
        auto* p = n ? alloc.allocate(n) : nullptr;
        auto sp = std::span(p, n);

        bool caught = false;
        try {
            if (!use_scalar) RTTS_EXPECT(raze::algorithm::uninitialized_default_construct(p, p + n) == p + n);
            else             RTTS_EXPECT(raze::algorithm::uninitialized_default_construct[opt::fscalar](sp) == sp.end());
            RTTS_EXPECT(!should_throw);
            RTTS_EQUAL(default_throw_obj::alive, static_cast<int>(n));
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(p[i].marker, 77);
            raze::algorithm::destroy(p, p + n);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(default_throw_obj::alive, 0);
        if (p) alloc.deallocate(p, n);
    }

    template <class T>
    void uninitialized_default_construct_trivial_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::allocator<T> alloc;
            T* p = n ? alloc.allocate(n) : nullptr;
            auto sp = std::span(p, n);

            const unsigned mode = static_cast<unsigned>(it % 4);
            if (mode == 0u)      RTTS_EXPECT(raze::algorithm::uninitialized_default_construct(p, p + n) == p + n);
            else if (mode == 1u) RTTS_EXPECT(raze::algorithm::uninitialized_default_construct(sp) == sp.end());
            else if (mode == 2u) RTTS_EXPECT(raze::algorithm::uninitialized_default_construct[opt::fscalar](sp) == sp.end());
            else                 RTTS_EXPECT(raze::algorithm::uninitialized_default_construct[opt::fstatic](p, p + n) == p + n);

            raze::algorithm::destroy(p, p + n);
            if (p) alloc.deallocate(p, n);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_default_construct fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        uninitialized_default_construct_trivial_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::uninitialized_default_construct lifetime and exception rollback") {
    check_default_rollback(8, -1, false);
    check_default_rollback(8, -1, true);
    check_default_rollback(10, 5, false);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            const size_t n = static_cast<size_t>(rng() % 65);
            const bool should_throw = (n > 0) && ((it % 3) != 2);
            check_default_rollback(n, should_throw ? static_cast<int>(rng() % n) : -1, (it & 1u) != 0u);
        }
    }
};
