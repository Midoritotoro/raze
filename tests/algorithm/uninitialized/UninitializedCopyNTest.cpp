#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedCopyN.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace {
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
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_copy_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        const auto src = rtts::random::vector<T>(sz, 910u + static_cast<unsigned>(sz));
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto r = raze::algorithm::uninitialized_copy_n[opt::fscalar](
            src.begin(), static_cast<std::ptrdiff_t>(sz), dst, dst + sz);
        RTTS_EXPECT(r.in == src.end() && r.out == dst + sz);
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], src[i]);
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(sz));

        if (dst) alloc.deallocate(dst, sz);
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
        copy_n_throw_obj::alive = 0;
        copy_n_throw_obj::copies = 0;
        copy_n_throw_obj::fail_at = -1;

        std::vector<copy_n_throw_obj> src;
        src.reserve(5);
        for (int i = 0; i < 5; ++i) src.emplace_back(i);
        const int base_alive = copy_n_throw_obj::alive;

        copy_n_throw_obj::copies = 0;
        copy_n_throw_obj::fail_at = 2;
        std::allocator<copy_n_throw_obj> alloc;
        auto* dst = alloc.allocate(5);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_copy_n(src.begin(), 5, dst, dst + 5);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && copy_n_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 5);
    }
};

