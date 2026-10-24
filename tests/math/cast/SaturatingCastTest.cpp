#include <tests/rts/rts.h>
#include <raze/math/Math.h>
#include <limits>
#include <cstdint>

RTTS_CASE("raze::math::saturating_cast.integral_same_type") {
    RTTS_EXPECT(raze::math::saturating_cast<int>(42) == 42);
    RTTS_EXPECT(raze::math::saturating_cast<int>(-42) == -42);
    RTTS_EXPECT(raze::math::saturating_cast<int>(0) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<int>(std::numeric_limits<int>::max()) == std::numeric_limits<int>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int>(std::numeric_limits<int>::min()) == std::numeric_limits<int>::min());

    RTTS_EXPECT(raze::math::saturating_cast<unsigned>(42u) == 42u);
    RTTS_EXPECT(raze::math::saturating_cast<unsigned>(0u) == 0u);
    RTTS_EXPECT(raze::math::saturating_cast<unsigned>(std::numeric_limits<unsigned>::max()) == std::numeric_limits<unsigned>::max());
};

RTTS_CASE("raze::math::saturating_cast.integral_widening") {
    RTTS_EXPECT(raze::math::saturating_cast<int64_t>(int8_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<int64_t>(int8_t(127)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int64_t>(int8_t(-128)) == -128);
    RTTS_EXPECT(raze::math::saturating_cast<int64_t>(std::numeric_limits<int8_t>::max()) == int64_t(std::numeric_limits<int8_t>::max()));
    RTTS_EXPECT(raze::math::saturating_cast<int64_t>(std::numeric_limits<int8_t>::min()) == int64_t(std::numeric_limits<int8_t>::min()));

    RTTS_EXPECT(raze::math::saturating_cast<uint64_t>(uint8_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint64_t>(uint8_t(255)) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint64_t>(std::numeric_limits<uint8_t>::max()) == uint64_t(std::numeric_limits<uint8_t>::max()));

    RTTS_EXPECT(raze::math::saturating_cast<int32_t>(std::numeric_limits<int16_t>::max()) == int32_t(std::numeric_limits<int16_t>::max()));
    RTTS_EXPECT(raze::math::saturating_cast<int32_t>(std::numeric_limits<int16_t>::min()) == int32_t(std::numeric_limits<int16_t>::min()));
};

RTTS_CASE("raze::math::saturating_cast.integral_narrowing") {
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(100)) == 100);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(-100)) == -100);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(127)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(-128)) == -128);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(128)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(-129)) == -128);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(1000)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(int64_t(-1000)) == -128);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(std::numeric_limits<int64_t>::max()) == std::numeric_limits<int8_t>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(std::numeric_limits<int64_t>::min()) == std::numeric_limits<int8_t>::min());

    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(uint64_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(uint64_t(255)) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(uint64_t(256)) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(std::numeric_limits<uint64_t>::max()) == std::numeric_limits<uint8_t>::max());

    RTTS_EXPECT(raze::math::saturating_cast<int16_t>(std::numeric_limits<int32_t>::max()) == std::numeric_limits<int16_t>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int16_t>(std::numeric_limits<int32_t>::min()) == std::numeric_limits<int16_t>::min());
    RTTS_EXPECT(raze::math::saturating_cast<int16_t>(int32_t(1000)) == 1000);
};

RTTS_CASE("raze::math::saturating_cast.integral_mixed_sign") {
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(-1)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(-100)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(100)) == 100);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(255)) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(int32_t(256)) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(std::numeric_limits<int32_t>::max()) == 255);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(std::numeric_limits<int32_t>::min()) == 0);

    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(uint32_t(0)) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(uint32_t(100)) == 100);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(uint32_t(127)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(uint32_t(128)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(uint32_t(1000)) == 127);
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(std::numeric_limits<uint32_t>::max()) == 127);

    RTTS_EXPECT(raze::math::saturating_cast<int32_t>(std::numeric_limits<uint32_t>::max()) == std::numeric_limits<int32_t>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int32_t>(uint32_t(1000)) == 1000);
};

RTTS_CASE("raze::math::saturating_cast.float_to_double") {
    RTTS_EXPECT(raze::math::saturating_cast<double>(0.0f) == 0.0);
    RTTS_EXPECT(raze::math::saturating_cast<double>(1.5f) == 1.5);
    RTTS_EXPECT(raze::math::saturating_cast<double>(-1.5f) == -1.5);
    RTTS_EXPECT(raze::math::saturating_cast<double>(std::numeric_limits<float>::max()) == double(std::numeric_limits<float>::max()));
    RTTS_EXPECT(raze::math::saturating_cast<double>(std::numeric_limits<float>::lowest()) == double(std::numeric_limits<float>::lowest()));
    RTTS_EXPECT(raze::math::saturating_cast<double>(std::numeric_limits<float>::infinity()) == std::numeric_limits<double>::infinity());
    RTTS_EXPECT(raze::math::saturating_cast<double>(-std::numeric_limits<float>::infinity()) == -std::numeric_limits<double>::infinity());
};

RTTS_CASE("raze::math::saturating_cast.double_to_float") {
    RTTS_EXPECT(raze::math::saturating_cast<float>(0.0) == 0.0f);
    RTTS_EXPECT(raze::math::saturating_cast<float>(1.5) == 1.5f);
    RTTS_EXPECT(raze::math::saturating_cast<float>(-1.5) == -1.5f);

    RTTS_EXPECT(raze::math::saturating_cast<float>(1e300) == std::numeric_limits<float>::max());
    RTTS_EXPECT(raze::math::saturating_cast<float>(-1e300) == -std::numeric_limits<float>::max());
    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<double>::max()) == std::numeric_limits<float>::max());
    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<double>::lowest()) == -std::numeric_limits<float>::max());
    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<double>::infinity()) == std::numeric_limits<float>::max());
    RTTS_EXPECT(raze::math::saturating_cast<float>(-std::numeric_limits<double>::infinity()) == -std::numeric_limits<float>::max());
};

RTTS_CASE("raze::math::saturating_cast.float_to_integral") {
    RTTS_EXPECT(raze::math::saturating_cast<int>(0.0) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<int>(1.0) == 1);
    RTTS_EXPECT(raze::math::saturating_cast<int>(-1.0) == -1);
    RTTS_EXPECT(raze::math::saturating_cast<int>(1.9) == 1);
    RTTS_EXPECT(raze::math::saturating_cast<int>(-1.9) == -1);

    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(1e10) == std::numeric_limits<int8_t>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(-1e10) == std::numeric_limits<int8_t>::min());
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(1e300) == std::numeric_limits<int8_t>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int8_t>(-1e300) == std::numeric_limits<int8_t>::min());
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(-1e300) == 0);
    RTTS_EXPECT(raze::math::saturating_cast<uint8_t>(1e300) == std::numeric_limits<uint8_t>::max());

    RTTS_EXPECT(raze::math::saturating_cast<int>(1e20) == std::numeric_limits<int>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int>(-1e20) == std::numeric_limits<int>::min());
    RTTS_EXPECT(raze::math::saturating_cast<int>(std::numeric_limits<double>::infinity()) == std::numeric_limits<int>::max());
    RTTS_EXPECT(raze::math::saturating_cast<int>(-std::numeric_limits<double>::infinity()) == std::numeric_limits<int>::min());
};

RTTS_CASE("raze::math::saturating_cast.integral_to_float") {
    RTTS_EXPECT(raze::math::saturating_cast<float>(0) == 0.0f);
    RTTS_EXPECT(raze::math::saturating_cast<float>(1) == 1.0f);
    RTTS_EXPECT(raze::math::saturating_cast<float>(-1) == -1.0f);

    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<int64_t>::max()) == float(std::numeric_limits<int64_t>::max()));
    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<int64_t>::min()) == float(std::numeric_limits<int64_t>::min()));
    RTTS_EXPECT(raze::math::saturating_cast<float>(std::numeric_limits<uint64_t>::max()) == float(std::numeric_limits<uint64_t>::max()));
};