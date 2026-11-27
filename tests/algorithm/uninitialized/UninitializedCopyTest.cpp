#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedCopy.h>

#include <memory>
#include <span>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <random>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct copy_throw_obj {
        static inline int alive = 0;
        static inline int copies = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit copy_throw_obj(int v) noexcept : val(v) { ++alive; }
        copy_throw_obj(const copy_throw_obj& o) : val(o.val) {
            if (fail_at >= 0 && copies == fail_at)
                throw std::runtime_error("copy fail");
            ++copies;
            ++alive;
        }
        ~copy_throw_obj() { --alive; }
    };

    template <class T>
    void check_uninit_copy(const std::vector<T>& src, size_t dst_n, unsigned mode) {
        namespace opt = raze::options;
        const size_t copied = std::min(src.size(), dst_n);

        std::allocator<T> alloc;
        T* dst = dst_n ? alloc.allocate(dst_n) : nullptr;
        auto sp = std::span(dst, dst_n);

        if (mode == 0u) {
            auto r = raze::algorithm::uninitialized_copy(src.begin(), src.end(), dst, dst + dst_n);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == dst + copied);
        }
        else if (mode == 1u) {
            auto r = raze::algorithm::uninitialized_copy(src, sp);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == sp.begin() + static_cast<std::ptrdiff_t>(copied));
        }
        else if (mode == 2u) {
            auto r = raze::algorithm::uninitialized_copy[opt::fscalar](src, sp);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == sp.begin() + static_cast<std::ptrdiff_t>(copied));
        }
        else {
            auto r = raze::algorithm::uninitialized_copy[opt::fstatic](src.begin(), src.end(), dst, dst + dst_n);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(copied) && r.out == dst + copied);
        }

        for (size_t i = 0; i < copied; ++i)
            RTTS_EQUAL(dst[i], src[i]);
        raze::algorithm::destroy(dst, dst + copied);
        if (dst) alloc.deallocate(dst, dst_n);
    }

    void check_copy_rollback(size_t src_n, size_t dst_n, int fail_at, std::mt19937& rng) {
        const size_t count = std::min(src_n, dst_n);
        copy_throw_obj::alive = 0;
        copy_throw_obj::copies = 0;
        copy_throw_obj::fail_at = -1;

        std::vector<copy_throw_obj> src;
        src.reserve(src_n);
        for (size_t i = 0; i < src_n; ++i)
            src.emplace_back(static_cast<int>(rng() % 1000u));
        const int base_alive = copy_throw_obj::alive;

        const bool should_throw = (fail_at >= 0) && (static_cast<size_t>(fail_at) < count);
        copy_throw_obj::copies = 0;
        copy_throw_obj::fail_at = fail_at;

        std::allocator<copy_throw_obj> alloc;
        auto* dst = dst_n ? alloc.allocate(dst_n) : nullptr;
        bool caught = false;
        try {
            auto r = raze::algorithm::uninitialized_copy(src, std::span(dst, dst_n));
            RTTS_EXPECT(!should_throw);
            RTTS_EXPECT(r.in == src.begin() + static_cast<std::ptrdiff_t>(count));
            RTTS_EQUAL(copy_throw_obj::alive, base_alive + static_cast<int>(count));
            for (size_t i = 0; i < count; ++i)
                RTTS_EQUAL(dst[i].val, src[i].val);
            raze::algorithm::destroy(dst, dst + count);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EQUAL(caught, should_throw);
        RTTS_EQUAL(copy_throw_obj::alive, base_alive);
        if (dst) alloc.deallocate(dst, dst_n);
    }
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        const auto src = rtts::random::vector<T>(sz, 900u + static_cast<unsigned>(sz));
        check_uninit_copy<T>(src, sz, 0u);
        check_uninit_copy<T>(src, sz, 2u);
    }

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 257);
            const size_t dst_n = (it % 3 == 0) ? src_n : static_cast<size_t>(rng() % 257);
            check_uninit_copy<T>(rtts::random::vector<T>(src_n, rng()), dst_n, static_cast<unsigned>(it % 4));
        }
    }
};

RTTS_CASE("raze::algorithm::uninitialized_copy shorter output, exception rollback, and dangling") {
    {
        const std::vector<int> src{ 1, 2, 3, 4, 5 };
        std::allocator<int> alloc;
        int* dst = alloc.allocate(3);
        auto r = raze::algorithm::uninitialized_copy(src, std::span(dst, 3));
        RTTS_EXPECT(r.in == src.begin() + 3 && r.out == std::span(dst, 3).end());
        RTTS_EXPECT(dst[0] == 1 && dst[1] == 2 && dst[2] == 3);
        alloc.deallocate(dst, 3);
    }

    {
        std::mt19937 rng(1u);
        check_copy_rollback(6, 6, 3, rng);
    }

    {
        std::allocator<int> alloc;
        int* dst = alloc.allocate(3);
        auto dr = raze::algorithm::uninitialized_copy(std::vector<int>{ 1, 2, 3 }, std::span(dst, 3));
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        alloc.deallocate(dst, 3);
    }

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t src_n = static_cast<size_t>(rng() % 65);
            const size_t dst_n = static_cast<size_t>(rng() % 65);
            const size_t count = std::min(src_n, dst_n);
            const bool should_throw = (count > 0) && ((it % 3) != 2);
            check_copy_rollback(src_n, dst_n, should_throw ? static_cast<int>(rng() % count) : -1, rng);
        }
    }
};
