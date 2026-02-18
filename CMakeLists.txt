cmake_minimum_required (VERSION 4.1  FATAL_ERROR)

project ("raze"
    VERSION 1.0.0
    DESCRIPTION "..."
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_LINK_ $<IF:$<CONFIG:Debug>, /NODEFAULTLIB:LIBCMT, /DEBUG;/OPT:REF> /INCREMENTAL:NO)

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()
if (NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin")
endif()
if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "include")
endif()

option(RAZE_INSTALL "" ${PROJECT_IS_TOP_LEVEL})
set(RAZE_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/raze")

set(PRIVATE_SOURCES  
    "src/raze/algorithm/vectorized/find/FindVectorized.cpp"
)

set(PRIVATE_HEADERS 
    "src/raze/algorithm/vectorized/find/FindVectorized.h"
    
    "src/raze/algorithm/vectorized/find/ContainsVectorized.h"
    src/raze/algorithm/vectorized/find/CountVectorized.h
    "src/raze/algorithm/vectorized/find/FindLastVectorized.h"
    src/raze/algorithm/vectorized/find/FindVectorized.h
    "src/raze/algorithm/vectorized/find/MismatchVectorized.h"
    src/raze/algorithm/vectorized/replace/ReplaceVectorized.h
    src/raze/algorithm/vectorized/replace/ReplaceCopyVectorized.h
    src/raze/algorithm/vectorized/order/ReverseCopyVectorized.h
    src/raze/algorithm/vectorized/order/ReverseVectorized.h
    src/raze/algorithm/vectorized/swap/SwapRangesVectorized.h
    "src/raze/algorithm/vectorized/find/EqualVectorized.h"
    "src/raze/algorithm/vectorized/find/FindEndVectorized.h"
    src/raze/algorithm/vectorized/copy/CopyVectorized.h
    src/raze/algorithm/vectorized/copy/MoveVectorized.h
    src/raze/algorithm/vectorized/fill/FillVectorized.h
    src/raze/algorithm/vectorized/remove/RemoveVectorized.h
    src/raze/algorithm/vectorized/remove/RemoveCopyVectorized.h
    src/raze/algorithm/vectorized/transform/TransformVectorized.h
    src/raze/algorithm/vectorized/minmax/MinmaxElementVectorized.h
    src/raze/algorithm/vectorized/minmax/MinElementVectorized.h
    src/raze/algorithm/vectorized/minmax/MaxElementVectorized.h
    src/raze/algorithm/vectorized/minmax/MinVectorized.h
    src/raze/algorithm/vectorized/minmax/MaxVectorized.h
    src/raze/algorithm/vectorized/minmax/MinmaxVectorized.h

    src/raze/algorithm/AlgorithmDebug.h
    src/raze/algorithm/AdvanceBytes.h

    "src/raze/algorithm/unchecked/find/FindUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindIfUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindIfNotUnchecked.h"
    "src/raze/algorithm/unchecked/find/ContainsUnchecked.h"
    "src/raze/algorithm/unchecked/find/MismatchUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindEndUnchecked.h" 
    "src/raze/algorithm/unchecked/find/SearchUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindLastUnchecked.h"
    "src/raze/algorithm/unchecked/find/EqualUnchecked.h"
    "src/raze/algorithm/unchecked/find/CountUnchecked.h"
    "src/raze/algorithm/unchecked/find/CountIfUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindLastIfNotUnchecked.h"
    "src/raze/algorithm/unchecked/find/FindLastIfUnchecked.h"

    src/raze/algorithm/unchecked/copy/CopyUnchecked.h
    src/raze/algorithm/unchecked/copy/CopyIfUnchecked.h
    src/raze/algorithm/unchecked/copy/CopyBackwardUnchecked.h
    src/raze/algorithm/unchecked/copy/MoveUnchecked.h
    src/raze/algorithm/unchecked/copy/MoveBackwardUnchecked.h

    src/raze/algorithm/unchecked/replace/ReplaceCopyIfUnchecked.h
    src/raze/algorithm/unchecked/replace/ReplaceCopyUnchecked.h

    src/raze/algorithm/unchecked/replace/ReplaceIfUnchecked.h
    src/raze/algorithm/unchecked/replace/ReplaceUnchecked.h

    src/raze/algorithm/unchecked/transform/TransformUnchecked.h

    src/raze/algorithm/unchecked/batch/ForEachUnchecked.h
    src/raze/algorithm/unchecked/batch/ForEachNUnchecked.h

    src/raze/algorithm/unchecked/remove/RemoveUnchecked.h
    src/raze/algorithm/unchecked/remove/RemoveIfUnchecked.h

    src/raze/algorithm/unchecked/remove/RemoveCopyUnchecked.h
    src/raze/algorithm/unchecked/remove/RemoveCopyIfUnchecked.h

    src/raze/algorithm/unchecked/minmax/MinmaxElementUnchecked.h
    src/raze/algorithm/unchecked/minmax/MinElementUnchecked.h
    src/raze/algorithm/unchecked/minmax/MaxElementUnchecked.h

    src/raze/algorithm/unchecked/minmax/MinUnchecked.h
    src/raze/algorithm/unchecked/minmax/MaxUnchecked.h
    src/raze/algorithm/unchecked/minmax/MinmaxUnchecked.h

    src/raze/math/CountTrailingZeros.h
    src/raze/math/CountLeadingZeros.h
    src/raze/math/PopulationCount.h


    "src/raze/type_traits/CanMemcmpElements.h"
    "src/raze/type_traits/ContainerComparison.h"
    "src/raze/type_traits/FillMemsetSafety.h"
    "src/raze/type_traits/FunctionPass.h"
    "src/raze/type_traits/IntegralProperties.h"
    "src/raze/type_traits/IsConstantEvaluated.h"
    "src/raze/type_traits/IteratorCategory.h"
    "src/raze/type_traits/IteratorCheck.h" 
    "src/raze/type_traits/LexicographicalCompareTraits.h"
    "src/raze/type_traits/Ordering.h" 
    "src/raze/type_traits/SimdAlgorithmSafety.h"
    "src/raze/type_traits/SwapSafety.h" 
    "src/raze/type_traits/TypeCheck.h" 
    "src/raze/type_traits/TypeTraits.h"
    "src/raze/type_traits/UnwrapEnum.h"
    "src/raze/type_traits/SimdTypeCheck.h"
    "src/raze/type_traits/IsVirtualBaseOf.h"
    src/raze/type_traits/OperatorWrappers.h
    src/raze/type_traits/Invoke.h
    src/raze/type_traits/FunctionInformation.h

    "src/raze/utility/Assert.h"

    src/raze/datapar/IntrinBitcast.h
    src/raze/datapar/SimdIntegralTypesCheck.h

    src/raze/datapar/SimdIndexMask.inl
    src/raze/datapar/SimdCompareResult.inl
    src/raze/datapar/SimdCompareAdapters.h
    src/raze/datapar/SimdCompareAdapters.inl
    src/raze/datapar/SimdMaskCommonOperations.h
    src/raze/datapar/SimdMaskCommonOperations.inl
    src/raze/datapar/SimdElementMaskOperations.h
    src/raze/datapar/SimdIndexMaskOperations.h
    src/raze/datapar/SimdElementMaskOperations.inl
    src/raze/datapar/SimdIndexMaskOperations.inl
    src/raze/datapar/MaskTypeSelector.h
    src/raze/datapar/SimdMaskTypeCheck.h

    src/raze/datapar/CachePrefetcher.h

    src/raze/datapar/BasicSimd.inl
    src/raze/datapar/SimdCast.inl
    src/raze/datapar/BasicSimdElementReference.inl
    src/raze/datapar/IsComparable.h
    src/raze/datapar/SimdDispatcher.h
    src/raze/datapar/SizedSimdDispatcher.h
    src/raze/concurrency/WindowsThread.h
    src/raze/concurrency/ThreadYield.h
    src/raze/concurrency/WindowsThreadPool.h
    src/raze/concurrency/WindowsThreadPoolTasks.h
    
    "src/raze/algorithm/vectorized/find/SearchVectorized.h"
    src/raze/algorithm/vectorized/traits/SearchLoop.h
    src/raze/algorithm/FixedMemcmp.h
    src/raze/algorithm/MsvcIteratorUnwrap.h

    src/raze/datapar/bitwise/BitAnd.h
    src/raze/datapar/bitwise/BitOr.h
    src/raze/datapar/bitwise/BitXor.h
    src/raze/datapar/bitwise/BitNot.h
    src/raze/datapar/bitwise/ToMask.h
    src/raze/datapar/bitwise/ToVector.h
    src/raze/datapar/bitwise/ToIndexMask.h
    src/raze/datapar/bitwise/MaskConvert.h
    src/raze/datapar/bitwise/IndexMaskDivisor.h

    src/raze/datapar/arithmetic/Sub.h
    src/raze/datapar/arithmetic/Add.h
    src/raze/datapar/arithmetic/Mul.h
    src/raze/datapar/arithmetic/Div.h
    src/raze/datapar/arithmetic/Negate.h
    src/raze/datapar/arithmetic/VerticalMax.h
    src/raze/datapar/arithmetic/VerticalMin.h
    src/raze/datapar/arithmetic/Abs.h

    src/raze/datapar/compare/Less.h
    src/raze/datapar/compare/LessEqual.h
    src/raze/datapar/compare/Greater.h
    src/raze/datapar/compare/GreaterEqual.h
    src/raze/datapar/compare/Equal.h
    src/raze/datapar/compare/NotEqual.h

    src/raze/datapar/memory/CompressStore.h
    src/raze/datapar/memory/Load.h
    src/raze/datapar/memory/Store.h
    src/raze/datapar/memory/MaskLoad.h
    src/raze/datapar/memory/MaskzLoad.h
    src/raze/datapar/memory/MaskStore.h
    src/raze/datapar/memory/NonTemporalLoad.h
    src/raze/datapar/memory/NonTemporalStore.h
    src/raze/datapar/memory/StreamingFence.h
    src/raze/datapar/memory/AlignmentPolicy.h
    src/raze/datapar/memory/MakeTailMask.h
    src/raze/datapar/memory/MaskLoadSupport.h
    src/raze/datapar/memory/MaskStoreSupport.h

    src/raze/datapar/reduce/ReduceAdd.h
    src/raze/datapar/reduce/ReduceAddType.h
    src/raze/datapar/reduce/Fold.h
    src/raze/datapar/reduce/HorizontalMax.h
    src/raze/datapar/reduce/HorizontalMin.h
    src/raze/datapar/reduce/HorizontalMinmax.h

    src/raze/datapar/shuffle/ShuffleTables.h
    src/raze/datapar/shuffle/MaskExpand.h
    src/raze/datapar/shuffle/Insert.h
    src/raze/datapar/shuffle/InsertMask.h
    src/raze/datapar/shuffle/Extract.h
    src/raze/datapar/shuffle/Compress.h
    src/raze/datapar/shuffle/Broadcast.h
    src/raze/datapar/shuffle/BroadcastZeros.h
    src/raze/datapar/shuffle/Reverse.h
    src/raze/datapar/shuffle/Blend.h

    src/raze/datapar/Shuffle.h
    src/raze/datapar/Bitwise.h
    src/raze/datapar/Reduce.h
    src/raze/datapar/Arithmetic.h
    src/raze/datapar/Compare.h
    src/raze/datapar/Memory.h
)

set(PUBLIC_HEADERS
    include/raze/algorithm/Algorithm.h

    include/raze/math/Math.h
    include/raze/math/BitMath.h
    include/raze/math/IntegralTypesConversions.h

    include/raze/arch/ProcessorFeatures.h
    include/raze/arch/ProcessorDetection.h
    include/raze/arch/CpuId.h
    include/raze/arch/CpuFeature.h
    include/raze/arch/ProcessorInformation.h

    include/raze/concurrency/Thread.h
    include/raze/concurrency/Mutex.h
    include/raze/concurrency/ThreadHandle.h
    include/raze/concurrency/ThreadId.h
    include/raze/concurrency/ThreadPool.h
    include/raze/concurrency/Execution.h

    include/raze/concurrency/algorithm/find/ParallelFind.h
    include/raze/concurrency/algorithm/find/ParallelFindIf.h
    include/raze/concurrency/algorithm/find/ParallelFindIfNot.h

    include/raze/concurrency/algorithm/find/ParallelAllOf.h
    include/raze/concurrency/algorithm/find/ParallelAnyOf.h
    include/raze/concurrency/algorithm/find/ParallelNoneOf.h

    include/raze/algorithm/batch/ForEach.h
    include/raze/algorithm/batch/ForEachN.h

    include/raze/compatibility/AlignmentMacros.h
    include/raze/compatibility/BranchPrediction.h
    include/raze/compatibility/CallingConventions.h
    include/raze/compatibility/Compatibility.h
    include/raze/compatibility/CxxVersionDetection.h
    include/raze/compatibility/CompilerDetection.h
    include/raze/compatibility/FunctionAttributes.h
    include/raze/compatibility/Inline.h
    include/raze/compatibility/LanguageFeatures.h
    include/raze/compatibility/MemoryMacros.h
    include/raze/compatibility/Nodiscard.h
    include/raze/compatibility/SystemDetection.h
    include/raze/compatibility/UnreachableCode.h
    include/raze/compatibility/Warnings.h
    include/raze/compatibility/SimdCompatibility.h
    include/raze/compatibility/StaticOperators.h

    include/raze/Types.h
    include/raze/RazeNamespace.h

    include/raze/algorithm/find/Find.h
    include/raze/algorithm/find/Search.h
    include/raze/algorithm/find/Contains.h
    include/raze/algorithm/find/Count.h
    include/raze/algorithm/find/Mismatch.h
    include/raze/algorithm/find/FindFirstOf.h
    include/raze/algorithm/find/StartsWith.h
    include/raze/algorithm/find/EndsWith.h
    include/raze/algorithm/find/AllOf.h
    include/raze/algorithm/find/NoneOf.h
    include/raze/algorithm/find/AnyOf.h
    include/raze/algorithm/find/FindEnd.h
    include/raze/algorithm/find/FindLast.h

    include/raze/algorithm/copy/Copy.h
    include/raze/algorithm/copy/CopyN.h
    include/raze/algorithm/copy/CopyBackward.h
    include/raze/algorithm/copy/Move.h
    include/raze/algorithm/copy/MoveBackward.h

    include/raze/algorithm/fill/Fill.h
    include/raze/algorithm/fill/FillN.h

    include/raze/algorithm/swap/Swap.h

    include/raze/algorithm/remove/Remove.h
    include/raze/algorithm/remove/RemoveCopy.h

    include/raze/algorithm/replace/Replace.h
    include/raze/algorithm/replace/ReplaceCopy.h

    include/raze/algorithm/order/Reverse.h
    include/raze/algorithm/order/ReverseCopy.h

    include/raze/algorithm/minmax/Min.h
    include/raze/algorithm/minmax/Max.h
    include/raze/algorithm/minmax/Minmax.h
    include/raze/algorithm/minmax/MaxElement.h
    include/raze/algorithm/minmax/MinElement.h
    include/raze/algorithm/minmax/MinmaxElement.h



    include/raze/system/Handle.h

    include/raze/memory/Intersects.h
    include/raze/memory/PointerToIntegral.h
    include/raze/memory/Alignment.h

    include/raze/datapar/Simd.h
    include/raze/datapar/BasicSimd.h
    include/raze/datapar/BasicSimdElementReference.h
    include/raze/datapar/BasicSimdMask.h
    include/raze/datapar/BasicSimdShuffleMask.h
    include/raze/datapar/SimdCast.h
    include/raze/datapar/SimdIndexMask.h
    include/raze/datapar/SimdCompareResult.h
    include/raze/datapar/SimdDataparAlgorithms.h
    include/raze/datapar/SimdConfig.h
)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

message(${CMAKE_CXX_COMPILER})

add_library(raze STATIC ${PRIVATE_SOURCES} ${PUBLIC_HEADERS})
add_library(raze::raze ALIAS raze)

include(cmake/Util_ExportConfig.cmake)

target_sources(raze PUBLIC 
    $<BUILD_INTERFACE:${PRIVATE_HEADERS}> 
    PRIVATE ${PRIVATE_SOURCES})


target_include_directories(raze
    PUBLIC
         "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_include_directories(raze
    PUBLIC
        "$<INSTALL_INTERFACE:include>"
)

target_include_directories(raze
    PUBLIC
        "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(raze
    PUBLIC
        "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)


set_target_properties(
    raze PROPERTIES
    
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF

    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"

    EXPORT_NAME raze
    OUTPUT_NAME raze

    LINKER_LANGUAGE CXX
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_features(raze PUBLIC cxx_std_23)
    set(CMAKE_DEBUG_TARGET_PROPERTIES "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	if((CMAKE_VS_PLATFORM_NAME STREQUAL "x64") OR (CMAKE_VS_PLATFORM_NAME STREQUAL "x86"))
		include(cmake/x86_Simd.cmake)
    endif()
endif()


if((CMAKE_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
		include(cmake/x86_Simd.cmake)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -Ob3) 
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O2 -Ot -Ob3 -GS- -Oy)
    endif()

    target_link_options(raze PRIVATE "/LTCG;/opt:ref,icf;")

    target_compile_options(
        raze 
        PRIVATE
            /bigobj
            /permissive-
            /W4
            
            /utf-8

            /MP
            /EHsc

            /guard:cf

            /w15038 
            /w14265 
            /wd4018
            /wd4100
            /wd4242
            /wd4244
            /wd4245
            /wd4267
            /wd4305
            /wd4324
            /wd4389
            /wd4456
            /wd4457
            /wd4458
            /wd4459
            /wd4611
            /wd4702
            /wd4828

            /Zi
            /Zc:__cplusplus
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(
        raze 
        PRIVATE
            /permissive-
            /W4
            /utf-8
            /MP
            /EHsc
            /Zi
            /Zc:__cplusplus
        )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
        raze 
        PRIVATE
            /utf-8

            /O3
            /W4

            /EHsc   

            /Zi
            /Zc:__cplusplus
    )
endif()

set(RAZE_FORCE_LIST
    RAZE_FORCE_SSE2
    RAZE_FORCE_SSE3
    RAZE_FORCE_SSSE3
    RAZE_FORCE_SSE41
    RAZE_FORCE_SSE42
    RAZE_FORCE_AVX
    RAZE_FORCE_AVX2
    RAZE_FORCE_AVX512F
    RAZE_FORCE_AVX512BW
    RAZE_FORCE_AVX512CD
    RAZE_FORCE_AVX512DQ
    RAZE_FORCE_AVX512VL
    RAZE_FORCE_AVX512VBMI
    RAZE_FORCE_AVX512VBMI2
)

set(RAZE_USER_FORCE_ENABLED OFF)

foreach(flag ${RAZE_FORCE_LIST})
    if(DEFINED ${flag})
        set(RAZE_USER_FORCE_ENABLED ON)
    endif()
endforeach()

function(raze_test_msvc_arch flag result_var) 
    set(src "${CMAKE_BINARY_DIR}/simd_test_${flag}.cpp") 
    file(WRITE "${src}" "int main(){return 0;}") 

    try_compile(${result_var} 
        "${CMAKE_BINARY_DIR}" 
        "${src}" CMAKE_FLAGS 
        "-DCMAKE_CXX_FLAGS=/arch:${flag}" 
        OUTPUT_VARIABLE out) 
endfunction()

if(RAZE_USER_FORCE_ENABLED)
    message(STATUS "RAZE: ISA FORCE mode enabled")

    if(MSVC)

        message(STATUS "RAZE: MSVC detected — validating /arch: flags")

        if(RAZE_FORCE_SSE2)
            add_compile_definitions(RAZE_FORCE_SSE2=1)
        endif()

        if(RAZE_FORCE_AVX)
            raze_test_msvc_arch("AVX" AVX_OK)
            if(NOT AVX_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX")
            endif()
            add_compile_options(/arch:AVX)
            add_compile_definitions(RAZE_FORCE_AVX=1)
        endif()

        if(RAZE_FORCE_AVX2)
            raze_test_msvc_arch("AVX2" AVX2_OK)
            if(NOT AVX2_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX2")
            endif()
            add_compile_options(/arch:AVX2)
            add_compile_definitions(RAZE_FORCE_AVX2=1)
        endif()


        if(RAZE_FORCE_AVX512F OR RAZE_FORCE_AVX512BW OR RAZE_FORCE_AVX512CD
           OR RAZE_FORCE_AVX512DQ OR RAZE_FORCE_AVX512VL
           OR RAZE_FORCE_AVX512VBMI OR RAZE_FORCE_AVX512VBMI2)

            raze_test_msvc_arch("AVX512" AVX512_OK)
            if(NOT AVX512_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX512")
            endif()

            add_compile_options(/arch:AVX512)

            add_compile_definitions(
                RAZE_FORCE_AVX512F=1
                RAZE_FORCE_AVX512BW=1
                RAZE_FORCE_AVX512CD=1
                RAZE_FORCE_AVX512DQ=1
                RAZE_FORCE_AVX512VL=1
                RAZE_FORCE_AVX512VBMI=1
                RAZE_FORCE_AVX512VBMI2=1
            )
        endif()

    else()
        include(CheckCXXCompilerFlag)

        macro(raze_require flag compileflag)
            if(${flag})
                check_cxx_compiler_flag(${compileflag} ${flag}_SUPPORTED)
                if(NOT ${flag}_SUPPORTED)
                    message(FATAL_ERROR "${flag}=1 requires ${compileflag}, but compiler does not support it")
                endif()
                add_compile_options(${compileflag})
                add_compile_definitions(${flag}=1)
            endif()
        endmacro()

        raze_require(RAZE_FORCE_SSE2        "-msse2")
        raze_require(RAZE_FORCE_SSE3        "-msse3")
        raze_require(RAZE_FORCE_SSSE3       "-mssse3")
        raze_require(RAZE_FORCE_SSE41       "-msse4.1")
        raze_require(RAZE_FORCE_SSE42       "-msse4.2")
        raze_require(RAZE_FORCE_AVX         "-mavx")
        raze_require(RAZE_FORCE_AVX2        "-mavx2")
        raze_require(RAZE_FORCE_AVX512F     "-mavx512f")
        raze_require(RAZE_FORCE_AVX512BW    "-mavx512bw")
        raze_require(RAZE_FORCE_AVX512CD    "-mavx512cd")
        raze_require(RAZE_FORCE_AVX512DQ    "-mavx512dq")
        raze_require(RAZE_FORCE_AVX512VL    "-mavx512vl")
        raze_require(RAZE_FORCE_AVX512VBMI  "-mavx512vbmi")
        raze_require(RAZE_FORCE_AVX512VBMI2 "-mavx512vbmi2")

    endif()

    add_compile_definitions(RAZE_ISA_FORCE_ENABLED=1)

else()

    message(STATUS "RAZE: No FORCE ISA specified — using runtime dispatch")
    add_compile_definitions(RAZE_ISA_FORCE_ENABLED=0)

endif()


if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(benchmarks)
endif()

if(RAZE_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/Util_InstallRules.cmake)
endif()
