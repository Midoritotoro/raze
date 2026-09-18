set(RAZE_TEST_ARCH_CONFIGS
    "SSE2|-msse2| |RAZE_HAS_SSE2_SUPPORT=1"
    "SSE3|-msse3| |RAZE_HAS_SSE3_SUPPORT=1"
    "SSSE3|-mssse3| |RAZE_HAS_SSSE3_SUPPORT=1"
    "SSE41|-msse4.1| |RAZE_HAS_SSE41_SUPPORT=1"
    "SSE42|-msse4.2| |RAZE_HAS_SSE42_SUPPORT=1"
    "AVX|-mavx|/arch:AVX|RAZE_HAS_AVX_SUPPORT=1"
    "FMA3|-mfma|/arch:AVX|RAZE_HAS_FMA3_SUPPORT=1"
    "AVX2|-mavx2|/arch:AVX2|RAZE_HAS_AVX2_SUPPORT=1"
    "AVX512F|-mavx512f|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512BW|-mavx512f -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512DQ|-mavx512f -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512BWDQ|-mavx512f -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1"
    "AVX512VLBWDQ|-mavx512f -mavx512vl -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLDQ|-mavx512f -mavx512vl -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLBW|-mavx512f -mavx512vl -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLF|-mavx512f -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VBMI|-mavx512f -mavx512vbmi -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1"
    "AVX512VBMI2|-mavx512f -mavx512vbmi2 -mavx512vbmi -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512VBMI2_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512VBMI2_SUPPORT=1"
    "AVX512VBMIVL|-mavx512f -mavx512vbmi -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1"
    "AVX512VBMI2VL|-mavx512f -mavx512vbmi2 -mavx512vbmi -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512VBMI2_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1"
    "AVX512VBMIDQ|-mavx512f -mavx512vbmi -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1"
    "AVX512VBMI2DQ|-mavx512f -mavx512vbmi2 -mavx512vbmi -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512VBMI2_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1"
    "AVX512VBMIVLDQ|-mavx512f -mavx512vbmi -mavx512bw -mavx512dq -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1"
    "AVX512VBMI2VLDQ|-mavx512f -mavx512vbmi2 -mavx512vbmi -mavx512bw -mavx512dq -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512VBMI2_SUPPORT=1"
)

function(raze_add_parent_target target)
    string(REGEX REPLACE "[^.]+\\.([^.]+)$" "\\1" parent_target ${target})
    string(REGEX REPLACE "^.*\\.([^.]+)$" "\\1" suffix ${parent_target})

    if(NOT TARGET ${target})
        add_custom_target(${target})
        set_property(TARGET ${target} PROPERTY FOLDER "${suffix}")
    endif()

    if(NOT ${parent_target} STREQUAL ${target})
        raze_add_parent_target(${parent_target})
        add_dependencies(${parent_target} ${target})
    endif()
endfunction()

function(raze_generate_test_runner runner_source arch_names)

    set(isa_names_cpp "")

    foreach(arch_name IN LISTS arch_names)
        string(APPEND isa_names_cpp
            "        \"${arch_name}\",\n"
        )
    endforeach()

    file(WRITE "${runner_source}" [=[
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <cstdio>
#    include <sys/wait.h>
#endif

namespace {

std::string format_number(std::string value)
{
    if (value.size() <= 3) {
        return value;
    }

    for (std::ptrdiff_t i =
             static_cast<std::ptrdiff_t>(value.size()) - 3;
         i > 0;
         i -= 3)
    {
        value.insert(
            static_cast<std::size_t>(i),
            ","
        );
    }

    return value;
}

std::string extract_test_count(std::string_view output)
{
    constexpr std::string_view prefix = "Results: ";

    const auto begin = output.find(prefix);

    if (begin == std::string_view::npos) {
        return {};
    }

    const auto tests_begin =
        begin + prefix.size();

    const auto tests_end =
        output.find(" tests", tests_begin);

    if (tests_end == std::string_view::npos) {
        return {};
    }

    return format_number(
        std::string(
            output.substr(
                tests_begin,
                tests_end - tests_begin
            )
        )
    );
}

std::string extract_isa_name(std::string_view executable_name)
{
    const auto pos =
        executable_name.rfind('.');

    if (pos == std::string_view::npos) {
        return std::string(executable_name);
    }

    return std::string(
        executable_name.substr(pos + 1)
    );
}

void print_indented_output(std::string_view output)
{
    std::size_t begin = 0;

    while (begin < output.size()) {
        auto end =
            output.find('\n', begin);

        if (end == std::string_view::npos) {
            end = output.size();
        }

        std::string_view line =
            output.substr(
                begin,
                end - begin
            );

        if (!line.empty() &&
            line.back() == '\r')
        {
            line.remove_suffix(1);
        }

        std::cout
            << "    "
            << line
            << '\n';

        begin = end;

        if (begin < output.size()) {
            ++begin;
        }
    }
}

struct process_result
{
    bool started = false;
    int exit_code = -1;
    std::string output;
};

#ifdef _WIN32

std::filesystem::path current_executable_path()
{
    std::wstring buffer(32768, L'\0');

    const DWORD length =
        GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size())
        );

    if (length == 0) {
        return {};
    }

    if (length >= buffer.size()) {
        std::cerr
            << "ERROR: GetModuleFileNameW buffer is too small\n";

        return {};
    }

    buffer.resize(length);

    return std::filesystem::absolute(
        std::filesystem::path(buffer)
    );
}

#else

std::filesystem::path current_executable_path(
    const char* argv0
)
{
    if (argv0 == nullptr || *argv0 == '\0') {
        return {};
    }

    return std::filesystem::absolute(
        std::filesystem::path(argv0)
    );
}

#endif

#ifdef _WIN32

process_result run_process(
    const std::filesystem::path& executable
)
{
    process_result result;

    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength =
        sizeof(SECURITY_ATTRIBUTES);

    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = nullptr;

    HANDLE read_pipe = nullptr;
    HANDLE write_pipe = nullptr;

    if (!CreatePipe(
            &read_pipe,
            &write_pipe,
            &security_attributes,
            0))
    {
        return result;
    }

    SetHandleInformation(
        read_pipe,
        HANDLE_FLAG_INHERIT,
        0
    );

    STARTUPINFOW startup_info{};
    startup_info.cb =
        sizeof(STARTUPINFOW);

    startup_info.dwFlags =
        STARTF_USESTDHANDLES;

    startup_info.hStdInput =
        GetStdHandle(STD_INPUT_HANDLE);

    startup_info.hStdOutput =
        write_pipe;

    startup_info.hStdError =
        write_pipe;

    PROCESS_INFORMATION process_info{};

    std::wstring command_line =
        L"\"" +
        executable.wstring() +
        L"\"";

    std::vector<wchar_t> command_buffer(
        command_line.begin(),
        command_line.end()
    );

    command_buffer.push_back(L'\0');

    const BOOL created =
        CreateProcessW(
            nullptr,
            command_buffer.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &startup_info,
            &process_info
        );

    CloseHandle(write_pipe);

    if (!created) {
        CloseHandle(read_pipe);
        return result;
    }

    result.started = true;

    std::string output;
    char buffer[4096];

    for (;;) {
        DWORD bytes_read = 0;

        const BOOL ok =
            ReadFile(
                read_pipe,
                buffer,
                sizeof(buffer),
                &bytes_read,
                nullptr
            );

        if (!ok || bytes_read == 0) {
            break;
        }

        output.append(
            buffer,
            buffer + bytes_read
        );
    }

    CloseHandle(read_pipe);

    WaitForSingleObject(
        process_info.hProcess,
        INFINITE
    );

    DWORD exit_code = 1;

    if (!GetExitCodeProcess(
            process_info.hProcess,
            &exit_code))
    {
        exit_code = 1;
    }

    CloseHandle(
        process_info.hThread
    );

    CloseHandle(
        process_info.hProcess
    );

    result.exit_code =
        static_cast<int>(exit_code);

    result.output =
        std::move(output);

    return result;
}

#else

process_result run_process(
    const std::filesystem::path& executable
)
{
    process_result result;

    const std::string command =
        "\"" +
        executable.string() +
        "\" 2>&1";

    FILE* pipe =
        popen(
            command.c_str(),
            "r"
        );

    if (!pipe) {
        return result;
    }

    result.started = true;

    char buffer[4096];

    while (fgets(
        buffer,
        sizeof(buffer),
        pipe))
    {
        result.output += buffer;
    }

    const int status =
        pclose(pipe);

    if (WIFEXITED(status)) {
        result.exit_code =
            WEXITSTATUS(status);
    }
    else {
        result.exit_code = 1;
    }

    return result;
}

#endif

}

int main(int argc, char** argv)
{
    namespace fs = std::filesystem;
    using clock = std::chrono::steady_clock;

#ifdef _WIN32
    const fs::path self =
        current_executable_path();
#else
    const fs::path self =
        current_executable_path(
            argc > 0 ? argv[0] : nullptr
        );
#endif

    if (self.empty()) {
        std::cerr
            << "ERROR: unable to determine runner executable path\n";

        return 1;
    }

    const fs::path test_directory =
        self.parent_path();

    const char* isa_names[] = {
]=])

    file(APPEND "${runner_source}" "${isa_names_cpp}")

    file(APPEND "${runner_source}" [=[
        nullptr
    };

    std::size_t total = 0;

    for (const char* executable_name : isa_names) {
        if (executable_name == nullptr) {
            break;
        }

        ++total;
    }

    std::size_t passed = 0;
    std::size_t failed = 0;
    std::size_t index = 0;

    const auto suite_start =
        clock::now();

    std::cout
        << '\n'
        << "RAZE TEST: "
        << total
        << " ISA configurations\n"
        << '\n';

    for (const char* executable_name : isa_names) {
        if (executable_name == nullptr) {
            break;
        }

        ++index;

        const std::string isa =
            extract_isa_name(
                executable_name
            );

        fs::path executable =
            test_directory / executable_name;

#ifdef _WIN32
        executable += ".exe";
#endif

        std::cout
            << "["
            << std::right
            << std::setw(2)
            << index
            << "/"
            << std::setw(2)
            << total
            << "] "
            << std::left
            << std::setw(18)
            << isa
            << std::flush;

        if (!fs::exists(executable)) {
            std::cout
                << std::left
                << std::setw(7)
                << "FAIL"
                << "executable not found\n";

            ++failed;
            continue;
        }

        const auto start =
            clock::now();

        const process_result result =
            run_process(executable);

        const auto end =
            clock::now();

        const auto elapsed_ms =
            std::chrono::duration_cast<
                std::chrono::milliseconds
            >(end - start).count();

        if (!result.started) {
            std::cout
                << std::left
                << std::setw(7)
                << "FAIL"
                << "could not start process\n";

            ++failed;
            continue;
        }

        if (result.exit_code == 0) {
            ++passed;

            const std::string test_count =
                extract_test_count(
                    result.output
                );

            std::cout
                << std::left
                << std::setw(7)
                << "PASS";

            if (!test_count.empty()) {
                std::cout
                    << std::right
                    << std::setw(10)
                    << test_count
                    << " tests";
            }
            else {
                std::cout
                    << std::right
                    << std::setw(17)
                    << "completed";
            }

            std::cout
                << std::right
                << std::setw(9)
                << elapsed_ms
                << " ms\n";
        }
        else {
            ++failed;

            const std::string test_count =
                extract_test_count(
                    result.output
                );

            std::cout
                << std::left
                << std::setw(7)
                << "FAIL";

            if (!test_count.empty()) {
                std::cout
                    << std::right
                    << std::setw(10)
                    << test_count
                    << " tests";
            }
            else {
                std::cout
                    << std::right
                    << std::setw(17)
                    << ("exit " +
                        std::to_string(
                            result.exit_code
                        ));
            }

            std::cout
                << std::right
                << std::setw(9)
                << elapsed_ms
                << " ms\n";

            if (!result.output.empty()) {
                std::cout
                    << '\n'
                    << "    ----------------------------------------------------------\n"
                    << "    Output from "
                    << isa
                    << '\n'
                    << "    ----------------------------------------------------------\n";

                print_indented_output(
                    result.output
                );

                std::cout
                    << "    ----------------------------------------------------------\n";
            }
        }
    }

    const auto suite_end =
        clock::now();

    const auto total_ms =
        std::chrono::duration_cast<
            std::chrono::milliseconds
        >(suite_end - suite_start).count();

    std::cout
        << '\n'
        << "============================================================\n"
        << "RAZE TEST SUMMARY\n"
        << "============================================================\n"
        << "Passed: "
        << passed
        << '\n'
        << "Failed: "
        << failed
        << '\n'
        << "Total:  "
        << total
        << '\n'
        << "Time:   "
        << total_ms
        << " ms\n"
        << "============================================================\n"
        << std::flush;

    return failed == 0 ? 0 : 1;
}
]=])

endfunction()


function(raze_generate_test root main_source rootpath file)
    if(ARGC GREATER 4)
        set(parent_target ${ARGV4})
    else()
        set(parent_target "unit")
    endif()

    string(REPLACE ".cpp" "" base ${file})
    string(REPLACE "/" "." base ${base})
    string(REPLACE "\\" "." base ${base})

    if(NOT root STREQUAL "")
        set(test "${root}.${base}")
    else()
        set(test "${base}")
    endif()

    if(NOT EXISTS "${main_source}")
        message(FATAL_ERROR
            "raze_generate_test: main.cpp not found: ${main_source}"
        )
    endif()

    set(main_target "${test}.main")

    add_library(${main_target} OBJECT "${main_source}")

    set_target_properties(${main_target} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_SCAN_FOR_MODULES OFF

        EXCLUDE_FROM_DEFAULT_BUILD TRUE
        EXCLUDE_FROM_ALL TRUE
    )

    target_compile_features(${main_target} PRIVATE cxx_std_23)

    target_include_directories(${main_target} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${main_target} PRIVATE
            /bigobj
            /permissive-
            /Od
            /MP
        )
    else()
        target_compile_options(${main_target} PRIVATE
            -g
        )
    endif()

    set(isa_targets)
    set(isa_names)

    foreach(arch_config IN LISTS RAZE_TEST_ARCH_CONFIGS)

        string(REPLACE "|" ";" config_list "${arch_config}")

        list(GET config_list 0 arch_name)
        list(GET config_list 1 gcc_and_llvm_flags)
        list(GET config_list 2 msvc_arch_flag)
        list(GET config_list 3 msvc_defs)

        set(isa_target "${test}.${arch_name}")

        add_executable(
            ${isa_target}
            "${rootpath}${file}"
        )

        target_sources(
            ${isa_target}
            PRIVATE
                $<TARGET_OBJECTS:${main_target}>
        )

        set_target_properties(${isa_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
            EXCLUDE_FROM_ALL TRUE

            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            CXX_SCAN_FOR_MODULES OFF

            OUTPUT_NAME "${test}.${arch_name}"
        )

        target_compile_features(
            ${isa_target}
            PRIVATE
                cxx_std_23
        )

        target_include_directories(
            ${isa_target}
            PRIVATE
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_SOURCE_DIR}/include
        )

        target_compile_definitions(
            ${isa_target}
            PRIVATE
                RAZE_TEST_ARCH_NAMESPACE=raze_test_${arch_name}
        )

        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

            string(REPLACE "," ";" msvc_defs_list "${msvc_defs}")

            target_compile_definitions(
                ${isa_target}
                PRIVATE
                    ${msvc_defs_list}
            )

            target_compile_options(
                ${isa_target}
                PRIVATE
                    /bigobj
                    /permissive-
                    /Od
                    /MP
                    ${msvc_arch_flag}
            )

        else()

            separate_arguments(
                flags_list
                UNIX_COMMAND
                "${gcc_and_llvm_flags}"
            )

            target_compile_options(
                ${isa_target}
                PRIVATE
                    ${flags_list}
                    -g
            )

        endif()

        target_link_libraries(
            ${isa_target}
            PRIVATE
                raze::raze
        )

        list(APPEND isa_targets ${isa_target})
        list(APPEND isa_names "${test}.${arch_name}")

    endforeach()

    set(runner_source
        "${CMAKE_CURRENT_BINARY_DIR}/${test}.all_isa.cpp"
    )

    raze_generate_test_runner(
        "${runner_source}"
        "${isa_names}"
    )

    add_executable(
        ${test}
        "${runner_source}"
    )

    set_target_properties(${test} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/unit"

        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_SCAN_FOR_MODULES OFF

        OUTPUT_NAME "${test}"

        FOLDER "Tests"
    )

    target_compile_features(
        ${test}
        PRIVATE
            cxx_std_23
    )

    add_dependencies(${test} ${isa_targets})

    add_test(
        NAME ${test}
        COMMAND ${test}
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${test}>"
    )

    if(NOT TARGET ${parent_target})
        add_custom_target(${parent_target})
    endif()

    add_dependencies(${parent_target} ${test})

    raze_add_parent_target(${test})

endfunction()
function(raze_glob_unit root relative pattern)
    if(ARGC GREATER 3)
        set(parent_target ${ARGV3})
    else()
        set(parent_target "unit")
    endif()

    set(main_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

    file(GLOB files
        RELATIVE ${relative}
        ${pattern}
    )

    foreach(file ${files})
        raze_generate_test(
            "${root}"
            "${main_source}"
            "${relative}/"
            ${file}
            ${parent_target}
        )
    endforeach()
endfunction()

function(raze_make_unit root)
    if(ARGC GREATER 1 AND NOT ARGV1 MATCHES "\\.(cpp|h)$")
        set(parent_target ${ARGV1})
        set(start_idx 2)
    else()
        set(parent_target "unit")
        set(start_idx 1)
    endif()

    set(main_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

    foreach(i RANGE ${start_idx} ${ARGC})
        if(i LESS ARGC)
            list(GET ARGV ${i} file)

            raze_generate_test(
                ${root}
                "${main_source}"
                "${CMAKE_CURRENT_SOURCE_DIR}/"
                ${file}
                ${parent_target}
            )
        endif()
    endforeach()
endfunction()
