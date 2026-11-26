#include <raze/algorithm/transform/Transform.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_transform_unary(benchmark::State& state, Fn&& fn) {
    const auto in = rb::make_iota<T>(rb::arg_size(state), 50, T(1));
    rb::bench_copy<T>(state, in, std::forward<Fn>(fn));
}

template <class T, class Fn>
static void run_transform_binary(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto in1 = rb::make_iota<T>(size, 50, T(1));
    const auto in2 = rb::make_iota<T>(size, 50, T(2));
    rb::bench_binary_copy<T>(state, in1, in2, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    auto unary_op = [](auto x) { return x * T(2) + T(1); };
    auto binary_op = [](auto x, auto y) { return x + y; };

    rb::register_triplet<T>("Transform",
        [=](benchmark::State& s) { run_transform_unary<T>(s, [=](const auto& in, auto& out) { return raze::algorithm::transform(in, out.begin(), unary_op); }); },
        [=](benchmark::State& s) { run_transform_unary<T>(s, [=](const auto& in, auto& out) { return std::ranges::transform(in, out.begin(), unary_op); }); },
        [=](benchmark::State& s) { run_transform_unary<T>(s, [=](const auto& in, auto& out) { return raze::algorithm::transform[raze::options::fscalar](in, out.begin(), unary_op); }); });

    rb::register_triplet<T>("TransformBinary",
        [=](benchmark::State& s) { run_transform_binary<T>(s, [=](const auto& a, const auto& b, auto& out) { return raze::algorithm::transform(a, b, out.begin(), binary_op); }); },
        [=](benchmark::State& s) { run_transform_binary<T>(s, [=](const auto& a, const auto& b, auto& out) { return std::ranges::transform(a, b, out.begin(), binary_op); }); },
        [=](benchmark::State& s) { run_transform_binary<T>(s, [=](const auto& a, const auto& b, auto& out) { return raze::algorithm::transform[raze::options::fscalar](a, b, out.begin(), binary_op); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();