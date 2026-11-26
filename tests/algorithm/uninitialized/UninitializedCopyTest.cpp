#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedCopy.h>

#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

namespace {
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
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        const auto src = rtts::random::vector<T>(sz, 900u + static_cast<unsigned>(sz));
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto r1 = raze::algorithm::uninitialized_copy(src.begin(), src.end(), dst, dst + sz);
        RTTS_EXPECT(r1.in == src.end() && r1.out == dst + sz);
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], src[i]);
        raze::algorithm::destroy(dst, dst + sz);

        auto sp = std::span(dst, sz);
        auto r2 = raze::algorithm::uninitialized_copy[opt::fscalar](src, sp);
        RTTS_EXPECT(r2.in == src.end() && r2.out == sp.end());
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], src[i]);
        raze::algorithm::destroy(sp);

        if (dst) alloc.deallocate(dst, sz);
    }
};

RTTS_CASE("raze::algorithm::uninitialized_copy shorter output, exception rollback, and dangling") {
    // Shorter output stops early
    {
        const std::vector<int> src{ 1, 2, 3, 4, 5 };
        std::allocator<int> alloc;
        int* dst = alloc.allocate(3);
        auto r = raze::algorithm::uninitialized_copy(src, std::span(dst, 3));
        RTTS_EXPECT(r.in == src.begin() + 3 && r.out == std::span(dst, 3).end());
        RTTS_EXPECT(dst[0] == 1 && dst[1] == 2 && dst[2] == 3);
        alloc.deallocate(dst, 3);
    }

    // Exception rollback
    {
        copy_throw_obj::alive = 0;
        copy_throw_obj::copies = 0;
        copy_throw_obj::fail_at = -1;

        std::vector<copy_throw_obj> src;
        src.reserve(6);
        for (int i = 0; i < 6; ++i) src.emplace_back(i * 10);
        const int base_alive = copy_throw_obj::alive;

        copy_throw_obj::copies = 0;
        copy_throw_obj::fail_at = 3;
        std::allocator<copy_throw_obj> alloc;
        auto* dst = alloc.allocate(6);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_copy(src, std::span(dst, 6));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && copy_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 6);
    }

    // Rvalue dangling
    {
        std::allocator<int> alloc;
        int* dst = alloc.allocate(3);
        auto dr = raze::algorithm::uninitialized_copy(std::vector<int>{ 1, 2, 3 }, std::span(dst, 3));
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        alloc.deallocate(dst, 3);
    }
};

