#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/UninitializedMove.h>

#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

namespace {
    struct move_throw_obj {
        static inline int alive = 0;
        static inline int moves = 0;
        static inline int fail_at = -1;
        int val = 0;

        explicit move_throw_obj(int v) noexcept : val(v) { ++alive; }
        move_throw_obj(move_throw_obj&& o) : val(o.val) {
            if (fail_at >= 0 && moves == fail_at)
                throw std::runtime_error("move fail");
            o.val = -1;
            ++moves;
            ++alive;
        }
        ~move_throw_obj() { --alive; }
    };
}

RTTS_CASE_TPL("raze::algorithm::uninitialized_move", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        auto src = rtts::random::vector<T>(sz, 920u + static_cast<unsigned>(sz));
        const auto orig = src;
        std::allocator<T> alloc;
        T* dst = sz ? alloc.allocate(sz) : nullptr;

        auto sp = std::span(dst, sz);
        auto r = raze::algorithm::uninitialized_move[opt::fscalar](src, sp);
        RTTS_EXPECT(r.in == src.end() && r.out == sp.end());
        for (size_t i = 0; i < sz; ++i) RTTS_EQUAL(dst[i], orig[i]);
        raze::algorithm::destroy(sp);

        if (dst) alloc.deallocate(dst, sz);
    }
};

RTTS_CASE("raze::algorithm::uninitialized_move move-only, exception rollback, and dangling") {
    // Move-only unique_ptr
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 6; ++i) src.push_back(std::make_unique<int>(i * 7));

        std::allocator<std::unique_ptr<int>> alloc;
        auto* dst = alloc.allocate(6);
        auto sp = std::span(dst, 6);
        auto r = raze::algorithm::uninitialized_move(src, sp);
        RTTS_EXPECT(r.in == src.end() && r.out == sp.end());
        for (int i = 0; i < 6; ++i) {
            RTTS_EXPECT(src[i] == nullptr);
            RTTS_EXPECT(dst[i] != nullptr && *dst[i] == i * 7);
        }
        raze::algorithm::destroy(sp);
        alloc.deallocate(dst, 6);
    }

    // Exception rollback
    {
        move_throw_obj::alive = 0;
        move_throw_obj::moves = 0;
        move_throw_obj::fail_at = -1;

        std::vector<move_throw_obj> src;
        src.reserve(5);
        for (int i = 0; i < 5; ++i) src.emplace_back(i + 1);
        const int base_alive = move_throw_obj::alive;

        move_throw_obj::moves = 0;
        move_throw_obj::fail_at = 3;
        std::allocator<move_throw_obj> alloc;
        auto* dst = alloc.allocate(5);
        bool caught = false;
        try {
            raze::algorithm::uninitialized_move(src, std::span(dst, 5));
        }
        catch (const std::runtime_error&) {
            caught = true;
        }
        RTTS_EXPECT(caught && move_throw_obj::alive == base_alive);
        alloc.deallocate(dst, 5);
    }

    // Rvalue dangling
    {
        std::allocator<int> alloc;
        int* dst = alloc.allocate(3);
        auto dr = raze::algorithm::uninitialized_move(std::vector<int>{ 1, 2, 3 }, std::span(dst, 3));
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        alloc.deallocate(dst, 3);
    }
};

