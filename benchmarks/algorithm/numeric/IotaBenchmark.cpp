#include <raze/algorithm/numeric/Iota.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_iota(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    std::vector<T> data(size);

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        if constexpr (sizeof(T) == 1 && std::is_signed_v<T>) {
            constexpr std::size_t chunk = 128;
            auto it = d.begin();
            for (std::size_t off = 0; off < d.size(); off += chunk) {
                const std::size_t len = (d.size() - off < chunk) ? (d.size() - off) : chunk;
                it = fn(d.begin() + static_cast<std::ptrdiff_t>(off),
                    d.begin() + static_cast<std::ptrdiff_t>(off + len), static_cast<T>(-64));
            }
            return it;
        }
        else {
            return fn(d.begin(), d.end(), T(0));
        }
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Iota",
        [](benchmark::State& s) { run_iota<T>(s, [](auto f, auto l, T v) { return raze::algorithm::iota(f, l, v).out; }); },
        [](benchmark::State& s) { run_iota<T>(s, [](auto f, auto l, T v) { return std::ranges::iota(f, l, v).out; }); },
        [](benchmark::State& s) { run_iota<T>(s, [](auto f, auto l, T v) { return raze::algorithm::iota[raze::options::fscalar](f, l, v).out; }); });
}

void RegisterAll() {
    using iota_types = rb::type_list<raze::i8, raze::i16, raze::i32, raze::i64>;
    rb::register_all_types<iota_types>([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

