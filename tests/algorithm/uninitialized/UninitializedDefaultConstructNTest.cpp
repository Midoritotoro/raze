#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstructN.h>

#include <memory>
#include <stdexcept>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

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

    void check_default_n_rollback(size_t n, int fail_at, bool use_scalar) {
        namespace opt = raze::options;
        default_n_throw_obj::alive = 0;
        default_n_throw_obj::count = 0;
        default_n_throw_obj::fail_at = fail_at;
        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < n);

        std::allocator<default_n_throw_obj> alloc;
        auto* p = n ? alloc.allocate(n) : nullptr;

        bool caught = false;
        try {
            auto* end = use_scalar
                ? raze::algorithm::uninitialized_default_construct_n[opt::fscalar](p, static_cast<std::ptrdiff_t>(n))
                : raze::algorithm::uninitialized_default_construct_n(p, static_cast<std::ptrdiff_t>(n));
            RTTS_EXPECT(!should_throw && end == p + n);
            RTTS_EQUAL(default_n_throw_obj::alive, static_cast<int>(n));
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(p[i].marker, 88);
            raze::algorithm::destroy_n(p, static_cast<std::ptrdiff_t>(n));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(default_n_throw_obj::alive, 0);
        if (p) alloc.deallocate(p, n);
    }

    template <class T>
    void uninitialized_default_construct_n_trivial_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::allocator<T> alloc;
            T* p = n ? alloc.allocate(n) : nullptr;

            const unsigned mode = static_cast<unsigned>(it % 3);
            T* end = (mode == 0u) ? raze::algorithm::uninitialized_default_construct_n(p, static_cast<std::ptrdiff_t>(n))
                   : (mode == 1u) ? raze::algorithm::uninitialized_default_construct_n[opt::fscalar](p, static_cast<std::ptrdiff_t>(n))
                                  : raze::algorithm::uninitialized_default_construct_n[opt::fstatic](p, static_cast<std::ptrdiff_t>(n));
            RTTS_EXPECT(end == p + n);
            raze::algorithm::destroy_n(p, static_cast<std::ptrdiff_t>(n));
            if (p) alloc.deallocate(p, n);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_default_construct_n fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        uninitialized_default_construct_n_trivial_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::uninitialized_default_construct_n lifetime and exception rollback") {
    check_default_n_rollback(8, -1, false);
    check_default_n_rollback(0, -1, true);
    check_default_n_rollback(8, 3, false);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            const size_t n = static_cast<size_t>(rng() % 65);
            const bool should_throw = (n > 0) && ((it % 3) != 2);
            check_default_n_rollback(n, should_throw ? static_cast<int>(rng() % n) : -1, (it & 1u) != 0u);
        }
    }
};
