#pragma once  

#include <raze/RazeNamespace.h>

#include <raze/compatibility/Warnings.h>
#include <raze/compatibility/BranchPrediction.h>

#include <cstdlib>
#include <cassert>

#include <iostream>


__RAZE_NAMESPACE_BEGIN


raze_disable_warning_msvc(6011);

struct static_locale {
	static_locale() noexcept {
		setlocale(LC_ALL, "");
	}
};

static const static_locale lc;

inline void fail(const char* message,
	const char* file, int	line) noexcept
{
	printf("Error: %s in File \"%s\", Line: %d\n", message, file, line);

	volatile auto nullptr_value = (int*)nullptr;
	*nullptr_value = 0;
	
	std::abort();
	std::terminate();
}

inline const char* extract_basename(const char* path, size_t size) noexcept {
	while (size != 0 && path[size - 1] != '/' && path[size - 1] != '\\')
		--size;

	return path + size;
}

#define raze_return_on_failure(message, file, line, return_value) \
	do { \
		printf("Error: %s in File \"%s\", Line: %d\n", message, file, line); \
		return return_value; \
	} \
		while (0)
	

#define assert_validation_condition(condition, message, file, line)\
	((raze_unlikely(!((condition))))\
		? raze::fail(message, file, line)\
		: void(0))

#define raze_assert_validation_condition_with_ret(condition, message, file, line, return_value)\
	if ((raze_unlikely(!(condition)))) \
		raze_return_on_failure(message, file, line, return_value)

#define raze_source_file_basename (raze::extract_basename(\
	__FILE__,\
	sizeof(__FILE__)))

#define raze_assert_log(condition, message) (assert_validation_condition(\
	condition,\
	message,\
	raze_source_file_basename,\
	__LINE__))

#define raze_assert_return(condition, message, return_value) raze_assert_validation_condition_with_ret(\
	condition,\
	message,\
	raze_source_file_basename,\
	__LINE__, \
	return_value)

#define raze_assert(condition) raze_assert_log((condition), "\"" #condition "\"")
#define raze_assert_unreachable() raze_assert(false)

#if !defined(NDEBUG)

#define raze_debug_assert_return	raze_assert_return
#define raze_debug_assert			raze_assert

#define raze_debug_assert_log		raze_assert_log

#else 

#define raze_debug_assert_return(condition, message, return_value)
#define raze_debug_assert(condition)

#define raze_debug_assert_log(condition, message)

#endif // !defined(NDEBUG)

__RAZE_NAMESPACE_END
