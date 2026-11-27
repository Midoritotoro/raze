#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedCopyN.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct copy_n_throw_obj {
        static inline int alive = 0;
        static inline int copies = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit copy_n_throw_obj(int v) noexcept : val(v) { ++alive; }
        copy_n_throw_obj(const copy_n_throw_obj& o) : val(o.val) {
            if (fail_at >= 0 && copies == fail_at)
                throw std::runtime_error("copy_n fail");
            ++copies;
            ++alive;
        }
        ~copy_n_throw_obj() { --alive; }
    };

    template <class T>
    void check_uninit_copy_n(const std::vector<T>& src, size_t cnt, size_t dst_n, unsigned mode) {
        namespace opt = raze::options;
        const size_t copied = std::min(cnt, dst_n);

        std::allocator<T> alloc;
        T* dst = dst_n ? alloc.allocate(dst_n) : nullptr;

        auto r = (mode == 0u) ? raze::algorithm::uninitialized_copy_n(src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n)
               : (mode == 1u) ? raze::algorithm::uninitialized_copy_n[opt::fscalar](src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n)
                              : raze::algorithm::uninitialized_copy_n[opt::fstatic](src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n);

        RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == dst + copied);
        for (size_t i = 0; i < copied; ++i)
            RTTS_EQUAL(dst[i], src[i]);
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(copied));
        if (dst) alloc.deallocate(dst, dst_n);
    }

    void check_copy_n_rollback(size_t src_n, size_t cnt, size_t dst_n, int fail_at, std::mt19937& rng) {
        const size_t copied = std::min(cnt, dst_n);
        copy_n_throw_obj::alive = 0;
        copy_n_throw_obj::copies = 0;
        copy_n_throw_obj::fail_at = -1;

        std::vector<copy_n_throw_obj> src;
        src.reserve(src_n);
        for (size_t i = 0; i < src_n; ++i)
            src.emplace_back(static_cast<int>(rng() % 1000u));
        const int base_alive = copy_n_throw_obj::alive;

        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < copied);
        copy_n_throw_obj::copies = 0;
        copy_n_throw_obj::fail_at = fail_at;

        std::allocator<copy_n_throw_obj> alloc;
        auto* dst = dst_n ? alloc.allocate(dst_n) : nullptr;
        bool caught = false;
        try {
            auto r = raze::algorithm::uninitialized_copy_n(src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n);
            RTTS_EXPECT(!should_throw);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == dst + copied);
            RTTS_EQUAL(copy_n_throw_obj::alive, base_alive + static_cast<int>(copied));
            for (size_t i = 0; i < copied; ++i)
                RTTS_EQUAL(dst[i].val, src[i].val);
            raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(copied));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(copy_n_throw_obj::alive, base_alive);
        if (dst) alloc.deallocate(dst, dst_n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_copy_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) })
        check_uninit_copy_n<T>(rtts::random::vector<T>(sz, 910u + static_cast<unsigned>(sz)), sz, sz, 1u);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 257);
            const size_t cnt = src_n ? static_cast<size_t>(rng() % (src_n + 1)) : 0;
            const size_t dst_n = (it % 3 == 0) ? cnt : static_cast<size_t>(rng() % 257);
            check_uninit_copy_n<T>(rtts::random::vector<T>(src_n, rng()), cnt, dst_n, static_cast<unsigned>(it % 3));
        }
    }
};

RTTS_CASE("raze::algorithm::uninitialized_copy_n shorter output and exception rollback") {
    {
        const std::vector<int> src{ 10, 20, 30, 40 };
        std::allocator<int> alloc;
        int* dst = alloc.allocate(2);
        auto r = raze::algorithm::uninitialized_copy_n(src.begin(), 4, dst, dst + 2);
        RTTS_EXPECT(r.in == src.begin() + 2 && r.out == dst + 2);
        RTTS_EXPECT(dst[0] == 10 && dst[1] == 20);
        alloc.deallocate(dst, 2);
    }

    {
        std::mt19937 rng(1u);
        check_copy_n_rollback(5, 5, 5, 2, rng);
    }

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 65);
            const size_t cnt = src_n ? static_cast<size_t>(rng() % (src_n + 1)) : 0;
            const size_t dst_n = static_cast<size_t>(rng() % 65);
            const size_t copied = std::min(cnt, dst_n);
            const bool should_throw = (copied > 0) && ((it % 3) != 2);
            check_copy_n_rollback(src_n, cnt, dst_n, should_throw ? static_cast<int>(rng() % copied) : -1, rng);
        }
    }
};
