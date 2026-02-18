#pragma once  

#include <raze/RazeNamespace.h>

#include <raze/compatibility/Warnings.h>
#include <raze/compatibility/BranchPrediction.h>

#include <cstdlib>
#include <cassert>

#include <iostream>


__RAZE_NAMESPACE_BEGIN


raze_disable_warning_msvc(6011);

struct __static_locale {
	__static_locale() noexcept {
		setlocale(LC_ALL, "");
	}
};

static const __static_locale __lc;

inline void __fail(
	const char* __message,
	const char* __file,
	int			__line) noexcept
{
	printf("Error: %s in File \"%s\", Line: %d\n", __message, __file, __line);

	volatile auto __nullptr_value = (int*)nullptr;
	*__nullptr_value = 0;
	
	std::abort();
	std::terminate();
}

inline const char* __extract_basename(
	const char* __path,
	size_t		__size) noexcept
{
	while (__size != 0 && __path[__size - 1] != '/' && __path[__size - 1] != '\\')
		--__size;

	return __path + __size;
}

#define __return_on_failure(__message, __file, __line, __return_value) \
	do { \
		printf("Error: %s in File \"%s\", Line: %d\n", __message, __file, __line); \
		return __return_value; \
	} \
		while (0)
	

#define __assert_validation_condition(__condition, __message, __file, __line)\
	((raze_unlikely(!((__condition))))\
		? raze::__fail(__message, __file, __line)\
		: void(0))

#define __assert_validation_condition_with_ret(__condition, __message, __file, __line, __return_value)\
	if ((raze_unlikely(!(__condition)))) \
		__return_on_failure(__message, __file, __line, __return_value)

#define __source_file_basename (raze::__extract_basename(\
	__FILE__,\
	sizeof(__FILE__)))

#define raze_assert_log(__condition, __message) (__assert_validation_condition(\
	__condition,\
	__message,\
	__source_file_basename,\
	__LINE__))

#define raze_assert_return(__condition, __message, __return_value) __assert_validation_condition_with_ret(\
	__condition,\
	__message,\
	__source_file_basename,\
	__LINE__, \
	__return_value)

#define raze_assert(__condition) raze_assert_log((__condition), "\"" #__condition "\"")
#define raze_assert_unreachable() raze_assert(false)

#if !defined(NDEBUG)

#define raze_debug_assert_return	raze_assert_return
#define raze_debug_assert			raze_assert

#define raze_debug_assert_log		raze_assert_log

#else 

#define raze_debug_assert_return(__condition, __message, __return_value)
#define raze_debug_assert(__condition)

#define raze_debug_assert_log(__condition, __message)

#endif // !defined(NDEBUG)

__RAZE_NAMESPACE_END
