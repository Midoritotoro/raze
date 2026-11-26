#include <raze/algorithm/copy/CopyBackward.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_copy_backward(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T, class Fn>
static void run_copy_backward_overlap(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_inplace<T>(state, data, std::forward<Fn>(fn), 2);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("CopyBackward",
        [](benchmark::State& s) { run_copy_backward<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy_backward(in, out.end()); }); },
        [](benchmark::State& s) { run_copy_backward<T>(s, [](const auto& in, auto& out) { return std::ranges::copy_backward(in, out.end()); }); },
        [](benchmark::State& s) { run_copy_backward<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy_backward[raze::options::fscalar](in, out.end()); }); });

    rb::register_triplet<T>("CopyBackwardOverlap",
        [](benchmark::State& s) { run_copy_backward_overlap<T>(s, [](auto& d) { return raze::algorithm::copy_backward(d.begin(), d.end() - 1, d.end()); }); },
        [](benchmark::State& s) { run_copy_backward_overlap<T>(s, [](auto& d) { return std::ranges::copy_backward(d.begin(), d.end() - 1, d.end()); }); },
        [](benchmark::State& s) { run_copy_backward_overlap<T>(s, [](auto& d) { return raze::algorithm::copy_backward[raze::options::fscalar](d.begin(), d.end() - 1, d.end()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

