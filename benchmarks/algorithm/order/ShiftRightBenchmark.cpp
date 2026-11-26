#include <raze/algorithm/order/ShiftRight.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_shift_right(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_iota<T>(size);
    const auto shift = static_cast<std::ptrdiff_t>(size / 3);
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) { return fn(d, shift); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("ShiftRight",
        [](benchmark::State& s) { run_shift_right<T>(s, [](auto& d, auto sh) { return raze::algorithm::shift_right(d, sh); }); },
        [](benchmark::State& s) { run_shift_right<T>(s, [](auto& d, auto sh) { return std::shift_right(d.begin(), d.end(), sh); }); },
        [](benchmark::State& s) { run_shift_right<T>(s, [](auto& d, auto sh) { return raze::algorithm::shift_right[raze::options::fscalar](d, sh); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

