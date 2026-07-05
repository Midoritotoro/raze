#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/utility/Assert.h>

#if defined(min)
#  undef min
#endif // defined(min)

#if defined(max)
#  undef max
#endif // defined(max)

#include <benchmark/benchmark.h>

#include <iostream>
#include <map>

#include <sstream>
#include <array>

#include <iomanip>

template <class T, std::size_t Size>
struct TestData
{
    std::vector<T> data;

    TestData(): data(Size) {
        for (std::size_t i = 0; i < Size; ++i)
            data[i] = static_cast<T>(i + 1);
    }
};


#if !defined(RAZE_BENCHMARK_MAIN)
#define RAZE_BENCHMARK_MAIN()                                                   \
    int main(int argc, char** argv) {                                           \
        benchmark::MaybeReenterWithoutASLR(argc, argv);                         \
        char arg0_default[] = "benchmark";                                      \
        char* args_default = reinterpret_cast<char*>(arg0_default);             \
        if (!argv) {                                                            \
            argc = 1;                                                           \
            argv = &args_default;                                               \
        }                                                                       \
        ::benchmark::Initialize(&argc, argv);                                   \
        if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;     \
        RegisterAll();                                                          \
        BenchmarksCompareReporter reporter;                                     \
        ::benchmark::RunSpecifiedBenchmarks(&reporter);                         \
        ::benchmark::Shutdown();                                                \
        return 0;                                                               \
    }                                                                           \
    int main(int, char**)
#endif // RAZE_BENCHMARK_MAIN


static struct Loc {
    Loc() {
        std::setlocale(LC_ALL, "en_US.UTF-8");
    }
} localeSet;

enum LogColor {
    COLOR_DEFAULT,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};

#ifdef raze_os_win
typedef WORD PlatformColorCode;
#else
typedef const char* PlatformColorCode;
#endif

PlatformColorCode GetPlatformColorCode(LogColor color) {
#ifdef raze_os_win
    switch (color) {
    case COLOR_RED:
        return FOREGROUND_RED;
    case COLOR_GREEN:
        return FOREGROUND_GREEN;
    case COLOR_YELLOW:
        return FOREGROUND_RED | FOREGROUND_GREEN;
    case COLOR_BLUE:
        return FOREGROUND_BLUE;
    case COLOR_MAGENTA:
        return FOREGROUND_BLUE | FOREGROUND_RED;
    case COLOR_CYAN:
        return FOREGROUND_BLUE | FOREGROUND_GREEN;
    case COLOR_WHITE:  // fall through to default
    default:
        return 0;
    }
#else
    switch (color) {
    case COLOR_RED:
        return "1";
    case COLOR_GREEN:
        return "2";
    case COLOR_YELLOW:
        return "3";
    case COLOR_BLUE:
        return "4";
    case COLOR_MAGENTA:
        return "5";
    case COLOR_CYAN:
        return "6";
    case COLOR_WHITE:
        return "7";
    default:
        return nullptr;
    };
#endif
}

std::string FormatString(const char* msg, va_list args) {
    va_list args_cp;
    va_copy(args_cp, args);

    std::size_t size = 256;
    char local_buff[256];
    auto ret = vsnprintf(local_buff, size, msg, args_cp);

    va_end(args_cp);

    raze_assert(ret >= 0);

    if (ret == 0) {
        return {};
    }
    if (static_cast<size_t>(ret) < size) {
        return local_buff;
    }
    size = static_cast<size_t>(ret) + 1;
    std::unique_ptr<char[]> buff(new char[size]);
    ret = vsnprintf(buff.get(), size, msg, args);
    raze_assert(ret > 0 && (static_cast<size_t>(ret)) < size);
    return buff.get();
}

std::string FormatString(const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    auto tmp = FormatString(msg, args);
    va_end(args);
    return tmp;
}


void ColorPrintf(
    std::ostream& out,
    LogColor color,
    const char* fmt,
    ...) 
{
    va_list args;
    va_start(args, fmt);

#ifdef raze_os_win
    const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    const WORD original_color_attrs = buffer_info.wAttributes;

    const WORD original_background_attrs =
        original_color_attrs & (BACKGROUND_RED | BACKGROUND_GREEN |
            BACKGROUND_BLUE | BACKGROUND_INTENSITY);

    SetConsoleTextAttribute(stdout_handle, GetPlatformColorCode(color) |
        FOREGROUND_INTENSITY |
        original_background_attrs);
    out << FormatString(fmt, args);

    out << std::flush;
    SetConsoleTextAttribute(stdout_handle, original_color_attrs);
#else
    const char* color_code = GetPlatformColorCode(color);
    if (color_code != nullptr) {
        out << FormatString("\033[0;3%sm", color_code);
    }
    out << FormatString(fmt, args) << "\033[m";
#endif
    va_end(args);
}


inline std::vector<benchmark::BenchmarkReporter::Run> _benchmarks;

class BenchmarksCompareReporter : public benchmark::ConsoleReporter
{
public:
    BenchmarksCompareReporter() {}

    bool ReportContext(const Context& context) override {
        return ConsoleReporter::ReportContext(context);
    }

    void ReportRuns(const std::vector<Run>& reports) override {
        std::ostream& out = GetOutputStream();
        ConsoleReporter::ReportRuns(reports);

        for (const auto& run : reports) {
            _benchmarks.push_back(run);

            if (_benchmarks.size() % 8 == 0) {
                const auto& first = _benchmarks[0];
                const auto& second = _benchmarks[4];

                const double t1 = first.GetAdjustedRealTime();
                const double t2 = second.GetAdjustedRealTime();

                const double speedup = t2 / t1;

                const auto name1 = first.benchmark_name().substr(0, first.benchmark_name().find('/'));
                const auto name2 = second.benchmark_name().substr(0, second.benchmark_name().find('/'));

                if (speedup > 1.0)
                {
                    ColorPrintf(out, COLOR_BLUE,
                        "%s (%.3f) is faster than %s (%.3f): speedup x%.3f\n",
                        name1.c_str(), t1,
                        name2.c_str(), t2,
                        speedup);
                }
                else if (speedup < 1.0)
                {
                    ColorPrintf(out, COLOR_RED,
                        "%s (%.3f) is slower than %s (%.3f): speedup x%.3f\n",
                        name1.c_str(), t1,
                        name2.c_str(), t2,
                        speedup);
                }
                else
                {
                    ColorPrintf(out, COLOR_WHITE, "Benchmarks are equal\n");
                }

                _benchmarks.clear();

            }
        }
    }
};