#pragma once

#include <raze/compatibility/Compatibility.h>
#include <raze/Types.h>
#include <raze/options/Options.h>

#if defined(min)
#  undef min
#endif // defined(min)

#if defined(max)
#  undef max
#endif // defined(max)

#include <version>
#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace raze::bench {

    inline constexpr double default_min_time = 0.5;
    inline constexpr int default_repetitions = 10;

    inline constexpr std::array<std::int64_t, 6> default_sizes{
        16, 64, 256, 1024, 4096, 16384
    };

    inline constexpr std::array<std::array<std::int64_t, 2>, 6> end_pos_cases{{
        { 16,    15    },
        { 64,    63    },
        { 256,   255   },
        { 1024,  1023  },
        { 4096,  4095  },
        { 16384, 16383 },
    }};

    inline constexpr std::array<std::array<std::int64_t, 2>, 6> start_pos_cases{{
        { 16,    0 },
        { 64,    0 },
        { 256,   0 },
        { 1024,  0 },
        { 4096,  0 },
        { 16384, 0 },
    }};

    inline constexpr std::array<std::array<std::int64_t, 3>, 11> search_end_cases{{
        { 16,    12,    4  },
        { 64,    60,    4  },
        { 256,   252,   4  },
        { 1024,  1020,  4  },
        { 4096,  4092,  4  },
        { 16384, 16380, 4  },
        { 64,    32,    32 },
        { 256,   224,   32 },
        { 1024,  992,   32 },
        { 4096,  4064,  32 },
        { 16384, 16352, 32 },
    }};

    inline constexpr std::array<std::array<std::int64_t, 3>, 11> search_start_cases{{
        { 16,    0, 4  },
        { 64,    0, 4  },
        { 256,   0, 4  },
        { 1024,  0, 4  },
        { 4096,  0, 4  },
        { 16384, 0, 4  },
        { 64,    0, 32 },
        { 256,   0, 32 },
        { 1024,  0, 32 },
        { 4096,  0, 32 },
        { 16384, 0, 32 },
    }};

    template <class... Ts>
    struct type_list {};

    using default_types = type_list<i8, i16, i32, i64, f32, f64>;

    template <class T>
    consteval std::string_view type_name() noexcept {
        if constexpr (std::same_as<T, i8>)  return "raze::i8";
        else if constexpr (std::same_as<T, u8>)  return "raze::u8";
        else if constexpr (std::same_as<T, i16>) return "raze::i16";
        else if constexpr (std::same_as<T, u16>) return "raze::u16";
        else if constexpr (std::same_as<T, i32>) return "raze::i32";
        else if constexpr (std::same_as<T, u32>) return "raze::u32";
        else if constexpr (std::same_as<T, i64>) return "raze::i64";
        else if constexpr (std::same_as<T, u64>) return "raze::u64";
        else if constexpr (std::same_as<T, f32>) return "raze::f32";
        else if constexpr (std::same_as<T, f64>) return "raze::f64";
        else return "unknown";
    }

    inline consteval std::string_view compiler_name() noexcept {
#if defined(raze_cpp_nvhpc)
        return "NVHPC";
#elif defined(raze_cpp_icpx)
        return "ICPX";
#elif defined(raze_cpp_clang) && defined(raze_cpp_msvc)
        return "Clang-cl";
#elif defined(raze_cpp_clang)
        if constexpr (compatibility::detail::detect_aocc())
            return "AOCC";
        return "Clang";
#elif defined(raze_cpp_gnu_only)
        return "GCC";
#elif defined(raze_cpp_msvc_only)
        return "MSVC";
#else
        return "Unknown";
#endif
    }

    inline std::string compiler_version() {
#if defined(__NVCOMPILER)
        return std::to_string(__NVCOMPILER_MAJOR__) + "." +
            std::to_string(__NVCOMPILER_MINOR__) + "." +
            std::to_string(__NVCOMPILER_PATCHLEVEL__);
#elif defined(__INTEL_LLVM_COMPILER)
        const int v = __INTEL_LLVM_COMPILER;
        return std::to_string(v / 10000) + "." + std::to_string((v % 10000) / 100);
#elif defined(__clang__)
        return std::to_string(__clang_major__) + "." +
            std::to_string(__clang_minor__) + "." +
            std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
        return std::to_string(__GNUC__) + "." +
            std::to_string(__GNUC_MINOR__) + "." +
            std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_FULL_VER)
        const int major = _MSC_FULL_VER / 10000000;
        const int minor = (_MSC_FULL_VER / 100000) % 100;
        const int build = _MSC_FULL_VER % 100000;
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(build);
#elif defined(_MSC_VER)
        return std::to_string(_MSC_VER / 100) + "." + std::to_string(_MSC_VER % 100);
#else
        return "unknown";
#endif
    }

    inline consteval std::string_view stl_name() noexcept {
#if defined(_MSVC_STL_VERSION)
        return "MSVC STL";
#elif defined(_LIBCPP_VERSION)
        return "libc++";
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
        return "libstdc++";
#else
        return "unknown";
#endif
    }

    inline std::string stl_version() {
#if defined(_MSVC_STL_VERSION)
        std::string res = std::to_string(_MSVC_STL_VERSION);
#  if defined(_MSVC_STL_UPDATE)
        res += " (" + std::to_string(_MSVC_STL_UPDATE) + ")";
#  endif
        return res;
#elif defined(_LIBCPP_VERSION)
        const int v = _LIBCPP_VERSION;
        return std::to_string(v / 10000) + "." +
            std::to_string((v / 100) % 100) + "." +
            std::to_string(v % 100);
#elif defined(__GLIBCXX__)
#  if defined(_GLIBCXX_RELEASE)
        return std::to_string(_GLIBCXX_RELEASE) + " (" + std::to_string(__GLIBCXX__) + ")";
#  else
        return std::to_string(__GLIBCXX__);
#  endif
#else
        return "unknown";
#endif
    }

    inline consteval std::string_view target_isa() noexcept {
#if defined(__AVX512F__)
        return "AVX-512";
#elif defined(__AVX2__)
        return "AVX2";
#elif defined(__AVX__)
        return "AVX";
#elif defined(__SSE4_2__)
        return "SSE4.2";
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
        return "SSE2";
#elif defined(__ARM_NEON) || defined(__aarch64__)
        return "NEON";
#else
        return "Scalar";
#endif
    }

    inline void add_environment_context() {
        ::benchmark::AddCustomContext("raze_compiler", std::string(compiler_name()));
        ::benchmark::AddCustomContext("raze_compiler_version", compiler_version());
        ::benchmark::AddCustomContext("raze_stl", std::string(stl_name()));
        ::benchmark::AddCustomContext("raze_stl_version", stl_version());
        ::benchmark::AddCustomContext("raze_isa", std::string(target_isa()));
    }

    template <class T>
    inline constexpr T needle_value() noexcept {
        return static_cast<T>(127);
    }

    template <class T>
    inline constexpr T alt_value() noexcept {
        return static_cast<T>(126);
    }

    inline sizetype arg_size(const ::benchmark::State& state, std::size_t index = 0) noexcept {
        return static_cast<sizetype>(state.range(index));
    }

    template <class T>
    inline std::vector<T> make_iota(sizetype size, sizetype mod = 100, T offset = T(0)) {
        std::vector<T> data(size);
        for (sizetype i = 0; i < size; ++i)
            data[i] = static_cast<T>(static_cast<T>(i % mod) + offset);
        return data;
    }

    template <class T>
    inline std::vector<T> make_find_data(sizetype size, sizetype pos, T needle = needle_value<T>()) {
        auto data = make_iota<T>(size);
        if (pos < size)
            data[pos] = needle;
        return data;
    }

    template <class T>
    inline std::vector<T> make_half_match(sizetype size, T needle = needle_value<T>()) {
        std::vector<T> data(size);
        for (sizetype i = 0; i < size; ++i)
            data[i] = (i & 1) ? needle : static_cast<T>((i % 100) + 1);
        return data;
    }

    template <class T>
    inline std::vector<T> make_alternating(sizetype size, T even_val, T odd_val) {
        std::vector<T> data(size);
        for (sizetype i = 0; i < size; ++i)
            data[i] = (i & 1) ? odd_val : even_val;
        return data;
    }

    template <class T>
    inline std::vector<T> make_sorted(sizetype size, bool descending = false) {
        std::vector<T> data(size);
        for (sizetype i = 0; i < size; ++i) {
            const auto v = (i * 120) / size;
            data[i] = static_cast<T>(descending ? (120 - v) : v);
        }
        return data;
    }

    template <class T>
    inline std::vector<T> make_sorted_until(sizetype size, bool descending = false) {
        auto data = make_sorted<T>(size, descending);
        if (size >= 2)
            data[size - 1] = descending ? T(127) : T(-1);
        return data;
    }

    template <class T>
    inline std::vector<T> make_adjacent_data(sizetype size) {
        auto data = make_iota<T>(size);
        if (size >= 2) {
            constexpr T dup = needle_value<T>();
            data[size - 2] = dup;
            data[size - 1] = dup;
        }
        return data;
    }

    template <class T>
    inline std::vector<T> make_search_n_data(sizetype size, sizetype pos, sizetype count, T needle = needle_value<T>()) {
        auto data = make_iota<T>(size);
        for (sizetype i = 0; i < count && pos + i < size; ++i)
            data[pos + i] = needle;
        return data;
    }

    template <class T>
    inline std::pair<std::vector<T>, std::vector<T>> make_subrange_data(sizetype size, sizetype pos, sizetype needle_len) {
        auto haystack = make_iota<T>(size);
        std::vector<T> needle(needle_len);

        for (sizetype j = 0; j < needle_len; ++j)
            needle[j] = static_cast<T>(100 + (j % 27));

        for (sizetype j = 0; j < needle_len && pos + j < size; ++j)
            haystack[pos + j] = needle[j];

        return { std::move(haystack), std::move(needle) };
    }

    template <class T>
    inline std::vector<T> make_extreme_data(sizetype size) {
        std::vector<T> data(size);
        for (sizetype i = 0; i < size; ++i)
            data[i] = static_cast<T>(static_cast<std::int64_t>((i * 37 + 13) % 201) - 100);

        if (size >= 4) {
            data[(size * 3) / 4] = static_cast<T>(-120);
            data[(size * 2) / 3] = static_cast<T>(120);
        }
        return data;
    }

    template <class T>
    inline constexpr auto unary_pred() noexcept {
        return [](auto x) { return (x - T(10)) > T(90); };
    }

    template <class T>
    inline constexpr auto unary_pred_not() noexcept {
        return [](auto x) { return (x - T(10)) <= T(90); };
    }

    template <class T>
    inline constexpr auto binary_eq_pred() noexcept {
        return [](auto a, auto b) { return (a - T(1)) == (b - T(1)); };
    }

    inline constexpr auto custom_comp() noexcept {
        return [](auto a, auto b) { return a > b; };
    }

    template <class T, class Fn>
    inline void bench_read(::benchmark::State& state, const std::vector<T>& data, Fn&& fn, sizetype stream_factor = 1) {
        const auto size = data.size();

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(data);
            auto res = fn(data);
            ::benchmark::DoNotOptimize(res);
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_binary_read(
        ::benchmark::State& state,
        const std::vector<T>& a,
        const std::vector<T>& b,
        Fn&& fn,
        sizetype stream_factor = 2)
    {
        const auto size = a.size();

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(a);
            ::benchmark::DoNotOptimize(b);
            auto res = fn(a, b);
            ::benchmark::DoNotOptimize(res);
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_copy(::benchmark::State& state, const std::vector<T>& src, Fn&& fn, sizetype stream_factor = 2) {
        const auto size = src.size();
        std::vector<T> dst(size);

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(src);
            auto res = fn(src, dst);
            ::benchmark::DoNotOptimize(res);
            ::benchmark::DoNotOptimize(dst);
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_binary_copy(
        ::benchmark::State& state,
        const std::vector<T>& a,
        const std::vector<T>& b,
        Fn&& fn,
        sizetype stream_factor = 3)
    {
        const auto size = a.size();
        std::vector<T> dst(size);

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(a);
            ::benchmark::DoNotOptimize(b);
            auto res = fn(a, b, dst);
            ::benchmark::DoNotOptimize(res);
            ::benchmark::DoNotOptimize(dst);
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_inplace(::benchmark::State& state, std::vector<T>& data, Fn&& fn, sizetype stream_factor = 1) {
        const auto size = data.size();

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(data);
            if constexpr (std::is_void_v<std::invoke_result_t<Fn&, std::vector<T>&>>) {
                fn(data);
            }
            else {
                auto res = fn(data);
                ::benchmark::DoNotOptimize(res);
            }
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_inplace_reset(::benchmark::State& state, const std::vector<T>& src, Fn&& fn, sizetype stream_factor = 1) {
        const auto size = src.size();
        auto work = src;

        for (auto _ : state) {
            state.PauseTiming();
            std::memcpy(work.data(), src.data(), size * sizeof(T));
            ::benchmark::DoNotOptimize(work);
            state.ResumeTiming();

            if constexpr (std::is_void_v<std::invoke_result_t<Fn&, std::vector<T>&>>) {
                fn(work);
            }
            else {
                auto res = fn(work);
                ::benchmark::DoNotOptimize(res);
            }
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    template <class T, class Fn>
    inline void bench_swap(::benchmark::State& state, std::vector<T>& a, std::vector<T>& b, Fn&& fn, sizetype stream_factor = 2) {
        const auto size = a.size();

        for (auto _ : state) {
            ::benchmark::DoNotOptimize(a);
            ::benchmark::DoNotOptimize(b);
            auto res = fn(a, b);
            ::benchmark::DoNotOptimize(res);
            ::benchmark::ClobberMemory();
        }

        state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * stream_factor));
    }

    inline void apply_defaults(::benchmark::internal::Benchmark* bm) {
        bm->MinTime(default_min_time)->Repetitions(default_repetitions)->ReportAggregatesOnly(true);
    }

    template <class ArgsContainer>
    inline void apply_args(::benchmark::internal::Benchmark* bm, const ArgsContainer& cases) {
        for (const auto& c : cases) {
            if constexpr (std::integral<std::remove_cvref_t<decltype(c)>>)
                bm->Args({ static_cast<std::int64_t>(c) });
            else
                bm->Args(std::vector<std::int64_t>(c.begin(), c.end()));
        }
    }

    template <class T, class Fn, class ArgsContainer = decltype(default_sizes)>
    inline void register_one(std::string_view fn_name, Fn&& fn, const ArgsContainer& cases = default_sizes) {
        std::string full;
        full.reserve(fn_name.size() + 2 + type_name<T>().size());
        full.append(fn_name);
        full.push_back('<');
        full.append(type_name<T>());
        full.push_back('>');

        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), std::forward<Fn>(fn));
        apply_args(bm, cases);
        apply_defaults(bm);
    }

    template <class T, class RazeFn, class StdFn, class ScalarFn, class ArgsContainer = decltype(default_sizes)>
    inline void register_triplet(
        std::string_view base_name,
        RazeFn&& raze_fn,
        StdFn&& std_fn,
        ScalarFn&& scalar_fn,
        const ArgsContainer& cases = default_sizes)
    {
        const std::string base(base_name);
        register_one<T>("BM_Raze" + base, std::forward<RazeFn>(raze_fn), cases);
        register_one<T>("BM_Std" + base, std::forward<StdFn>(std_fn), cases);
        register_one<T>("BM_Raze" + base + "Scalar", std::forward<ScalarFn>(scalar_fn), cases);
    }

    template <class TypeList = default_types, class RegisterFn>
    inline void register_all_types(RegisterFn&& reg_fn) {
        [&]<class... Ts>(type_list<Ts...>) {
            (reg_fn.template operator()<Ts>(), ...);
        }(TypeList{});
    }

} // namespace raze::bench

#if !defined(RAZE_BENCHMARK_MAIN)
#  define RAZE_BENCHMARK_MAIN()                                                 \
    int main(int argc, char** argv) {                                           \
        ::benchmark::MaybeReenterWithoutASLR(argc, argv);                       \
        char arg0_default[] = "benchmark";                                      \
        char* args_default = arg0_default;                                      \
        if (!argv) {                                                            \
            argc = 1;                                                           \
            argv = &args_default;                                               \
        }                                                                       \
        ::benchmark::Initialize(&argc, argv);                                   \
        if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;     \
        ::raze::bench::add_environment_context();                               \
        RegisterAll();                                                          \
        ::benchmark::RunSpecifiedBenchmarks();                                  \
        ::benchmark::Shutdown();                                                \
        return 0;                                                               \
    }                                                                           \
    int main(int, char**)
#endif // !defined(RAZE_BENCHMARK_MAIN)