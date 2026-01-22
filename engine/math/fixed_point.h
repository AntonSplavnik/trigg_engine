#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#ifdef PLATFORM_DESKTOP
    #include <cstdint>
#else
    #include "pico/stdlib.h"
#endif


struct Fixed_q16 {

	int32_t raw;

	Fixed_q16() : raw(0){}
	Fixed_q16(int32_t input) : raw(to_fixed(input)) {}

	#ifdef PLATFORM_DESKTOP
		Fixed_q16(float input) : raw(static_cast<int32_t>(input * 65536.0f)) {}
	#endif

	inline int32_t to_fixed(const int32_t input) const {
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

	bool operator>(const Fixed_q16& other) const {
		Fixed_q16 result;
		return (raw > other.raw) ;
	}
	bool operator<(const Fixed_q16& other) const {
		Fixed_q16 result;
		return (raw < other.raw);
	}

	bool operator>(const int32_t other) const {
		Fixed_q16 result;
		return (raw > to_fixed(other)) ;
	}
	bool operator<(const int32_t other) const {
		Fixed_q16 result;
		return (raw < to_fixed(other));
	}

	bool operator>(const uint16_t other) const {
		Fixed_q16 result;
		return (raw > to_fixed(static_cast<int32_t>(other))) ;
	}
	bool operator<(const uint16_t other) const {
		Fixed_q16 result;
		return (raw < to_fixed(static_cast<int32_t>(other)));
	}

	bool operator>=(const int32_t other) const {
		Fixed_q16 result;
		return (raw >= to_fixed(other)) ;
	}
	bool operator<=(const int32_t other) const {
		Fixed_q16 result;
		return (raw <= to_fixed(other));
	}

	bool operator>=(const int16_t other) const {
		Fixed_q16 result;
		return (raw >= to_fixed(other)) ;
	}
	bool operator<=(const int16_t other) const {
		Fixed_q16 result;
		return (raw <= to_fixed(other));
	}

	Fixed_q16& operator+=(const int32_t other) {

		raw += to_fixed(other);
		return *this;
	}
	Fixed_q16& operator-=(const int32_t other) {

		raw -= to_fixed(other);
		return *this;
	}

	Fixed_q16& operator-=(const Fixed_q16& other) {
		raw -= other.raw;
		return *this;
	}
	Fixed_q16& operator+=(const Fixed_q16& other) {
		raw += other.raw;
		return *this;
	}

	Fixed_q16 operator+(const uint32_t other) const {
		Fixed_q16 result;
		result.raw = raw + to_fixed(other);
		return result;
	}
	Fixed_q16 operator-(const uint32_t other) const {
		Fixed_q16 result;
		result.raw = raw - to_fixed(other);
		return result;
	}
	Fixed_q16 operator*(const uint32_t other) const {
		Fixed_q16 result;
		result.raw = raw * other;
		return result;
	}
};

#endif
