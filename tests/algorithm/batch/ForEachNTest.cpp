#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEachN.h>

RTTS_CASE_TPL("raze::algorithm::for_each_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T add = rtts::random::generator<T>(42)();
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [=](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [=](T& x) { x += add; }); },
            [=](auto c) { return std::ranges::for_each_n(c.first(), c.count(), [=](T& x) { x += add; }); });
    }

    //{
    //    const T mul = rtts::random::generator<T>(40000)();
    //    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
    //        [=](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [=](T& x) { x *= mul; }); },
    //        [=](auto c) { return std::ranges::for_each_n(c.first(), c.count(), [=](T& x) { x *= mul; }); });
    //}

    //rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
    //    [](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [](auto& x) { x += T(1); }); },
    //    [](auto c) { return std::ranges::for_each_n(c.first(), c.count(), [](auto& x) { x += T(1); }); });

    //static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
    //    [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(10); }); },
    //    [](auto c) constexpr { return std::ranges::for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(10); }); }));
};

//RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
//<class T> (rtts::type<T>) {
//    auto cfg = rtts::algorithm::config::thorough();
//    const T add = rtts::random::generator<T>(100000)();
//
//    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
//        [=](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [=](T& x) { x += add; }, [](T& x) -> T& { return x; }); },
//        [=](auto c) { return std::ranges::for_each_n(c.first(), c.count(), [=](T& x) { x += add; }, [](T& x) -> T& { return x; }); });
//
//    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
//        [=](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), [=](auto& x) { x += add; }, [](auto& x) -> auto& { return x; }); },
//        [=](auto c) { return std::ranges::for_each_n(c.first(), c.count(), [=](auto& x) { x += add; }, [](auto& x) -> auto& { return x; }); });
//
//    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
//        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), 
//            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
//        [](auto c) constexpr { return std::ranges::for_each_n(c.first(), c.count(),
//            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));
//};