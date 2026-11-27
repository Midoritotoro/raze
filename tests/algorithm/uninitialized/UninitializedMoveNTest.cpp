#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedMoveN.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct move_n_throw_obj {
        static inline int alive = 0;
        static inline int moves = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit move_n_throw_obj(int v) noexcept : val(v) { ++alive; }
        move_n_throw_obj(move_n_throw_obj&& o) : val(o.val) {
            if (fail_at >= 0 && moves == fail_at)
                throw std::runtime_error("move_n fail");
            o.val = -1;
            ++moves;
            ++alive;
        }
        ~move_n_throw_obj() { --alive; }
    };

    template <class T>
    void check_uninit_move_n(std::vector<T> src, size_t cnt, size_t dst_n, unsigned mode) {
        namespace opt = raze::options;
        const size_t moved = std::min(cnt, dst_n);
        const auto orig = src;

        std::allocator<T> alloc;
        T* dst = dst_n ? alloc.allocate(dst_n) : nullptr;

        auto r = (mode == 0u) ? raze::algorithm::uninitialized_move_n(src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n)
               : (mode == 1u) ? raze::algorithm::uninitialized_move_n[opt::fscalar](src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n)
                              : raze::algorithm::uninitialized_move_n[opt::fstatic](src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n);

        RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(moved) && r.out == dst + moved);
        for (size_t i = 0; i < moved; ++i)
            RTTS_EQUAL(dst[i], orig[i]);
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(moved));
        if (dst) alloc.deallocate(dst, dst_n);
    }

    void check_move_n_only(size_t src_n, size_t cnt, size_t dst_n) {
        const size_t moved = std::min(cnt, dst_n);
        std::vector<std::unique_ptr<int>> src;
        src.reserve(src_n);
        for (size_t i = 0; i < src_n; ++i)
            src.push_back(std::make_unique<int>(static_cast<int>(i + 100)));

        std::allocator<std::unique_ptr<int>> alloc;
        auto* dst = dst_n ? alloc.allocate(dst_n) : nullptr;
        auto r = raze::algorithm::uninitialized_move_n(src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n);
        RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(moved) && r.out == dst + moved);
        for (size_t i = 0; i < moved; ++i)
            RTTS_EXPECT(src[i] == nullptr && dst[i] != nullptr && *dst[i] == static_cast<int>(i + 100));
        for (size_t i = moved; i < src_n; ++i)
            RTTS_EXPECT(src[i] != nullptr && *src[i] == static_cast<int>(i + 100));
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(moved));
        if (dst) alloc.deallocate(dst, dst_n);
    }

    void check_move_n_rollback(size_t src_n, size_t cnt, size_t dst_n, int fail_at) {
        const size_t moved = std::min(cnt, dst_n);
        move_n_throw_obj::alive = 0;
        move_n_throw_obj::moves = 0;
        move_n_throw_obj::fail_at = -1;

        std::vector<move_n_throw_obj> src;
        src.reserve(src_n);
        for (size_t i = 0; i < src_n; ++i)
            src.emplace_back(static_cast<int>(i + 1));
        const int base_alive = move_n_throw_obj::alive;

        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < moved);
        move_n_throw_obj::moves = 0;
        move_n_throw_obj::fail_at = fail_at;

        std::allocator<move_n_throw_obj> alloc;
        auto* dst = dst_n ? alloc.allocate(dst_n) : nullptr;
        bool caught = false;
        try {
            auto r = raze::algorithm::uninitialized_move_n(src.begin(), static_cast<std::ptrdiff_t>(cnt), dst, dst + dst_n);
            RTTS_EXPECT(!should_throw && r.in == src.begin() + static_cast<std::ptrdiff_t>(moved) && r.out == dst + moved);
            RTTS_EQUAL(move_n_throw_obj::alive, base_alive + static_cast<int>(moved));
            for (size_t i = 0; i < moved; ++i)
                RTTS_EQUAL(dst[i].val, static_cast<int>(i + 1));
            raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(moved));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(move_n_throw_obj::alive, base_alive);
        if (dst) alloc.deallocate(dst, dst_n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_move_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) })
        check_uninit_move_n<T>(rtts::random::vector<T>(sz, 930u + static_cast<unsigned>(sz)), sz, sz, 1u);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 257);
            const size_t cnt = src_n ? static_cast<size_t>(rng() % (src_n + 1)) : 0;
            const size_t dst_n = (it % 3 == 0) ? cnt : static_cast<size_t>(rng() % 257);
            check_uninit_move_n<T>(rtts::random::vector<T>(src_n, rng()), cnt, dst_n, static_cast<unsigned>(it % 3));
        }
    }
};

RTTS_CASE("raze::algorithm::uninitialized_move_n move-only and exception rollback") {
    check_move_n_only(5, 5, 3);
    check_move_n_rollback(5, 5, 5, 2);

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 65);
            const size_t cnt = src_n ? static_cast<size_t>(rng() % (src_n + 1)) : 0;
            const size_t dst_n = static_cast<size_t>(rng() % 65);
            const size_t moved = std::min(cnt, dst_n);
            check_move_n_only(src_n, cnt, dst_n);
            const bool should_throw = (moved > 0) && ((it % 3) != 2);
            check_move_n_rollback(src_n, cnt, dst_n, should_throw ? static_cast<int>(rng() % moved) : -1);
        }
    }
};
