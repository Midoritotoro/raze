#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedFill.h>

#include <memory>
#include <span>
#include <stdexcept>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

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

    template <class T>
    void check_uninit_fill(size_t n, T val, unsigned mode) {
        namespace opt = raze::options;
        std::allocator<T> alloc;
        T* dst = n ? alloc.allocate(n) : nullptr;
        auto sp = std::span(dst, n);

        if (mode == 0u)      RTTS_EXPECT(raze::algorithm::uninitialized_fill(dst, dst + n, val) == dst + n);
        else if (mode == 1u) RTTS_EXPECT(raze::algorithm::uninitialized_fill(sp, val) == sp.end());
        else if (mode == 2u) RTTS_EXPECT(raze::algorithm::uninitialized_fill[opt::fscalar](sp, val) == sp.end());
        else                 RTTS_EXPECT(raze::algorithm::uninitialized_fill[opt::fstatic](dst, dst + n, val) == dst + n);

        for (size_t i = 0; i < n; ++i)
            RTTS_EQUAL(dst[i], val);
        raze::algorithm::destroy(dst, dst + n);
        if (dst) alloc.deallocate(dst, n);
    }

    void check_fill_rollback(size_t n, int marker, int fail_at) {
        fill_throw_obj::alive = 0;
        fill_throw_obj::copies = 0;
        fill_throw_obj::fail_at = -1;

        fill_throw_obj proto(marker);
        const int base_alive = fill_throw_obj::alive;

        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < n);
        fill_throw_obj::copies = 0;
        fill_throw_obj::fail_at = fail_at;

        std::allocator<fill_throw_obj> alloc;
        auto* dst = n ? alloc.allocate(n) : nullptr;
        bool caught = false;
        try {
            auto end = raze::algorithm::uninitialized_fill(dst, dst + n, proto);
            RTTS_EXPECT(!should_throw && end == dst + n);
            RTTS_EQUAL(fill_throw_obj::alive, base_alive + static_cast<int>(n));
            for (size_t i = 0; i < n; ++i)
                RTTS_EQUAL(dst[i].val, marker);
            raze::algorithm::destroy(dst, dst + n);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(fill_throw_obj::alive, base_alive);
        if (dst) alloc.deallocate(dst, n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_fill", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        check_uninit_fill<T>(sz, T(42), 0u);
        check_uninit_fill<T>(sz, T(77), 2u);
    }

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);
        for (size_t it = 0; it < 40; ++it)
            check_uninit_fill<T>(static_cast<size_t>(rng() % 257), val_gen(), static_cast<unsigned>(it % 4));
    }
};

RTTS_CASE("raze::algorithm::uninitialized_fill P2248 braced-init and exception rollback") {
    {
        std::allocator<pair_box> alloc;
        auto* p = alloc.allocate(4);
        raze::algorithm::uninitialized_fill(std::span(p, 4), { 10, 20 });
        for (int i = 0; i < 4; ++i) RTTS_EXPECT(p[i].a == 10 && p[i].b == 20);
        raze::algorithm::destroy(p, p + 4);
        alloc.deallocate(p, 4);
    }

    check_fill_rollback(6, 99, 3);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 50; ++it) {
            const int marker = static_cast<int>(rng() % 1000u);
            const size_t n = static_cast<size_t>(rng() % 65);
            const bool should_throw = (n > 0) && ((it % 3) != 2);
            check_fill_rollback(n, marker, should_throw ? static_cast<int>(rng() % n) : -1);
        }
    }
};
