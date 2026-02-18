#include <cassert>
#include <functional>
#include <iostream>
#include <src/raze/type_traits/Invoke.h>

int add(int a, int b) { return a + b; }

struct Functor {
    int operator()(int x) const { return x * 2; }
};

struct Obj {
    int value = 42;
    int method(int x) const { return value + x; }
};

int main() {
    {
        int r = raze::type_traits::invoke(add, 2, 3);
        assert(r == 5);
    }

    {
        auto lam = [](int x, int y) { return x * y; };
        int r = raze::type_traits::invoke(lam, 4, 5);
        assert(r == 20);
    }

    {
        Functor f;
        int r = raze::type_traits::invoke(f, 10);
        assert(r == 20);
    }

    {
        Obj o;

        int r = raze::type_traits::invoke(&Obj::method, o, 8);
        assert(r == 50); // 42 + 8
    }

    {
        Obj o;
        Obj* p = &o;
        int r = raze::type_traits::invoke(&Obj::method, p, 10);
        assert(r == 52);
    }

    {
        Obj o;
        std::reference_wrapper<Obj> ref(o);
        int r = raze::type_traits::invoke(&Obj::method, ref, 7);
        assert(r == 49);
    }

    {
        Obj o;
        int& r = raze::type_traits::invoke(&Obj::value, o);
        assert(r == 42);
        r = 100;
        assert(o.value == 100);
    }

    {
        Obj o;
        Obj* p = &o;
        int& r = raze::type_traits::invoke(&Obj::value, p);
        assert(r == 42);
    }

    {
        Obj o;
        std::reference_wrapper<Obj> ref(o);
        int& r = raze::type_traits::invoke(&Obj::value, ref);
        assert(r == 42);
    }
}
