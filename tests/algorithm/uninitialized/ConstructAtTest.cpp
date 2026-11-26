#include <tests/rts/rts.h>
#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

#include <memory>
#include <string>
#include <utility>

namespace {
    struct multi_arg_obj {
        int a;
        double b;
        std::string s;
        multi_arg_obj(int x, double y, std::string z) : a(x), b(y), s(std::move(z)) {}
    };
}

RTTS_CASE("raze::algorithm::construct_at multi-arg, move-only, and constexpr") {
    namespace opt = raze::options;

    {
        std::allocator<multi_arg_obj> alloc;
        auto* p = alloc.allocate(1);
        auto* r = raze::algorithm::construct_at(p, 7, 3.5, std::string("raze"));
        RTTS_EXPECT(r == p);
        RTTS_EQUAL(p->a, 7);
        RTTS_EQUAL(p->b, 3.5);
        RTTS_EQUAL(p->s, std::string("raze"));
        raze::algorithm::destroy_at(p);

        auto* r2 = raze::algorithm::construct_at[opt::fscalar](p, 9, 1.5, std::string("scalar"));
        RTTS_EXPECT(r2 == p && p->a == 9 && p->s == "scalar");
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
    }

    {
        std::allocator<std::unique_ptr<int>> alloc;
        auto* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, std::make_unique<int>(55));
        RTTS_EQUAL(**p, 55);
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
    }

    static_assert([]() constexpr {
        std::allocator<std::pair<int, int>> alloc;
        auto* p = alloc.allocate(1);
        raze::algorithm::construct_at(p, 10, 20);
        const int sum = p->first + p->second;
        raze::algorithm::destroy_at(p);
        alloc.deallocate(p, 1);
        return sum == 30;
    }());
};

