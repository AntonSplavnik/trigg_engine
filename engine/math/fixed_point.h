#ifndef FIXED_POINT_H
#define FIXED_POINT_H


#include "pico/stdlib.h"


struct Fixed_q16 {

	int32_t raw;

	Fixed_q16() : raw(0){}
	Fixed_q16(int32_t input) : raw(to_fixed(input)) {}

	inline int32_t to_fixed(const int32_t input) {
		return (input << 16);
	}

	inline int32_t to_int() {
		return (raw >> 16);
	}

	Fixed_q16 operator+(const Fixed_q16& other) const {
		Fixed_q16 result;
		result.raw = raw + other.raw;
		return result;
	}
	Fixed_q16 operator-(const Fixed_q16& other) const {
		Fixed_q16 result;
		result.raw = raw - other.raw;
		return result;
	}
	Fixed_q16 operator*(const Fixed_q16& other) const {
		Fixed_q16 result;
		result.raw = (static_cast<int64_t>(raw) * other.raw) >> 16;
		return result;
	}
	Fixed_q16 operator/(const Fixed_q16& other) const {
		Fixed_q16 result;
		result.raw = (static_cast<int64_t>(raw) << 16) / other.raw;
		return result;
	}
};

#endif
