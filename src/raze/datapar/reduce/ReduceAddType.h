#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <typename _Type_>
struct __reduce_type_helper {
    static constexpr auto _Size = sizeof(_Type_);

    using type =
        std::conditional_t<
            _Size == 1,
            std::conditional_t<std::is_unsigned_v<_Type_>, uint32, int32>,
        std::conditional_t<
            _Size == 2,
            std::conditional_t<std::is_unsigned_v<_Type_>, uint64, int64>,
        std::conditional_t<
            _Size == 4,
            std::conditional_t<std::is_floating_point_v<_Type_>, double,
                std::conditional_t<std::is_unsigned_v<_Type_>, uint64, int64>>,
        std::conditional_t<
            _Size == 8,
            std::conditional_t<std::is_floating_point_v<_Type_>, double,
                std::conditional_t<std::is_unsigned_v<_Type_>, uint64, int64>>,
        int64>>>>;
    
    static_assert(!std::is_same_v<type, void>, "Unsupported type size for __reduce_type_helper");
};

template <typename _Type_>
using __reduce_type = typename __reduce_type_helper<_Type_>::type;

__RAZE_DATAPAR_NAMESPACE_END
