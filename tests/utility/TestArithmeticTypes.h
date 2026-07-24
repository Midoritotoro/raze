#pragma once 

#include <utility>

namespace raze::tests::utility {
	template <class _Fn_>
	void run_for_arithmetic_types(_Fn_&& __fn) {
		__fn.template operator()<int8_t>();
		__fn.template operator()<uint8_t>();
		__fn.template operator()<int16_t>();
		__fn.template operator()<uint16_t>();
		__fn.template operator()<int32_t>();
		__fn.template operator()<uint32_t>();
		__fn.template operator()<int64_t>();
		__fn.template operator()<uint64_t>();
		__fn.template operator()<float>();
		__fn.template operator()<double>();
	}
}