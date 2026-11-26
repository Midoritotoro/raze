#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedMoveN.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace {
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
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_move_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        auto src = rtts::random::vector<T>(sz, 930u + static_cast<unsigned>(sz));
        const auto orig = src;
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto r = raze::algorithm::uninitialized_move_n[opt::fscalar](
            src.begin(), static_cast<std::ptrdiff_t>(sz), dst, dst + sz);
        RTTS_EXPECT(r.in == src.end() && r.out == dst + sz);
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], orig[i]);
        raze::algorithm::destroy_n(dst, static_cast<std::ptrdiff_t>(sz));

        if (dst) alloc.deallocate(dst, sz);
    }
};

RTTS_CASE("raze::algorithm::uninitialized_move_n move-only and exception rollback") {
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 5; ++i) src.push_back(std::make_unique<int>(i + 10));

        std::allocator<std::unique_ptr<int>> alloc;
        auto* dst = alloc.allocate(3);
        auto r = raze::algorithm::uninitialized_move_n(src.begin(), 5, dst, dst + 3);
        RTTS_EXPECT(r.in == src.begin() + 3 && r.out == dst + 3);
        for (int i = 0; i < 3; ++i) {
            RTTS_EXPECT(src[i] == nullptr && *dst[i] == i + 10);
        }
        RTTS_EXPECT(src[3] != nullptr && *src[3] == 13);
        raze::algorithm::destroy_n(dst, 3);
        alloc.deallocate(dst, 3);
    }

    {
        move_n_throw_obj::alive = 0;
        move_n_throw_obj::moves = 0;
        move_n_throw_obj::fail_at = -1;

        std::vector<move_n_throw_obj> src;
        src.reserve(5);
        for (int i = 0; i < 5; ++i) src.emplace_back(i);
        const int base_alive = move_n_throw_obj::alive;

        move_n_throw_obj::moves = 0;
        move_n_throw_obj::fail_at = 2;
        std::allocator<move_n_throw_obj> alloc;
        auto* dst = alloc.allocate(5);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_move_n(src.begin(), 5, dst, dst + 5);
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && move_n_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 5);
    }
};

