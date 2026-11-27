#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedFillN.h>

#include <memory>
#include <stdexcept>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

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

    template <class T>
    void check_uninit_fill_n(size_t n, T val, unsigned mode) {
        namespace opt = raze::options;
        std::allocator<T> alloc;
        T* dst = n ? alloc.allocate(n) : nullptr;

        T* end = (mode == 0u) ? raze::algorithm::uninitialized_fill_n(dst, static_cast<std::ptrdiff_t>(n), val)
               : (mode == 1u) ? raze::algorithm::uninitialized_fill_n[opt::fscalar](dst, static_cast<std::ptrdiff_t>(n), val)
                              : raze::algorithm::uninitialized_fill_n[opt::fstatic](dst, static_cast<std::ptrdiff_t>(n), val);
        RTTS_EXPECT(end == dst + n);
        for (size_t i = 0; i < n; ++i)
            RTTS_EQUAL(dst[i], val);
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(n));
        if (dst) alloc.deallocate(dst, n);
    }

    void check_fill_n_rollback(size_t n, int marker, int fail_at) {
        fill_n_throw_obj::alive = 0;
        fill_n_throw_obj::copies = 0;
        fill_n_throw_obj::fail_at = -1;

        fill_n_throw_obj proto(marker);
        const int base_alive = fill_n_throw_obj::alive;

        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < n);
        fill_n_throw_obj::copies = 0;
        fill_n_throw_obj::fail_at = fail_at;

        std::allocator<fill_n_throw_obj> alloc;
        auto* dst = n ? alloc.allocate(n) : nullptr;
        bool caught = false;
        try {
            auto* end = raze::algorithm::uninitialized_fill_n(dst, static_cast<std::ptrdiff_t>(n), proto);
            RTTS_EXPECT(!should_throw && end == dst + n);
            RTTS_EQUAL(fill_n_throw_obj::alive, base_alive + static_cast<int>(n));
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(dst[i].val, marker);
            raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(n));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(fill_n_throw_obj::alive, base_alive);
        if (dst) alloc.deallocate(dst, n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_fill_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) })
        check_uninit_fill_n<T>(sz, T(33), 1u);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);
        for (size_t it = 0; it < 40; ++it)
            check_uninit_fill_n<T>(static_cast<size_t>(rng() % 257), val_gen(), static_cast<unsigned>(it % 3));
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

    check_fill_n_rollback(6, 11, 2);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            const int marker = static_cast<int>(rng() % 1000u);
            const size_t n = static_cast<size_t>(rng() % 65);
            const bool should_throw = (n > 0) && ((it % 3) != 2);
            check_fill_n_rollback(n, marker, should_throw ? static_cast<int>(rng() % n) : -1);
        }
    }
};
