#ifndef JS3D_MATH_CONVERTERS_H
#define JS3D_MATH_CONVERTERS_H

#include <cinttypes>
#include <algorithm>

#undef _std
#define _std std::

// The hardware converts a byte to a float by division with 255 and in the
// vertex programs we convert the floating-point value in the range [0, 1]
// to the range [-1, 1] by multiplying with 2 and subtracting 1.
#define VERTEX_BYTE_TO_FLOAT( x )		( (x) * ( 2.0f / 255.0f ) - 1.0f )
#define VERTEX_FLOAT_TO_BYTE( x )		static_cast<uint8_t>( ( (x) + 1.0f ) * ( 255.0f / 2.0f ) + 0.5f )

// The hardware converts a byte to a float by division with 255 and in the
// fragment programs we convert the floating-point value in the range [0, 1]
// to the range [-1, 1] by multiplying with 2 and subtracting 1.
// This is the conventional OpenGL mapping which specifies an exact
// representation for -1 and +1 but not 0. The DirectX 10 mapping is
// in the comments which specifies a non-linear mapping with an exact
// representation of -1, 0 and +1 but -1 is represented twice.
#define NORMALMAP_BYTE_TO_FLOAT( x )	VERTEX_BYTE_TO_FLOAT( x )	//( (x) - 128.0f ) * ( 1.0f / 127.0f )
#define NORMALMAP_FLOAT_TO_BYTE( x )	VERTEX_FLOAT_TO_BYTE( x )	//idMath::Ftob( 128.0f + 127.0f * (x) + 0.5f )

/*
================================================
halfFloat_t
================================================
*/
typedef unsigned short halfFloat_t;

// GPU half-float bit patterns
#define HF_MANTISSA(x)	( x & 1023 )
#define HF_EXP(x)		( ( x & 32767 ) >> 10 )
#define HF_SIGN(x)		( ( x & 32768 ) ? -1 : 1 )

inline int8_t floatToSnorm8(float v)
{
	return static_cast<int8_t>(_std clamp(v * 127.0f, -128.0f, 127.0f));
}

inline float unorm8ToFloat(uint8_t v)
{
	return v / 255.0f;
}

inline uint8_t floatToUnorm8(float v)
{
	return static_cast<uint8_t>(_std clamp(v * 255.0f, 0.0f, 255.0f));
}

inline int16_t floatToUnorm16(float v)
{
	return static_cast<uint16_t>(_std clamp(v * 65535.0f, 0.0f, 65535.0f));
}

inline float unorm16ToFloat(uint16_t v)
{
	return v / 65535.0f;
}

inline int16_t floatToSnorm16(float v)
{
	return static_cast<int16_t>(_std clamp(v * 32767.0f, -32768.0f, 32767.0f));
}

inline int16_t floatToSnorm16DX(float v)
{
	//According to D3D10 rules, the value "-1.0f" has two representations:
	//  0x1000 and 0x10001
	//This allows everyone to convert by just multiplying by 32767 instead
	//of multiplying the negative values by 32768 and 32767 for positive.
	return static_cast<int16_t>(_std clamp(v >= 0.0f ?
		(v * 32767.0f + 0.5f) :
		(v * 32767.0f - 0.5f),
		-32768.0f,
		32767.0f));
}

inline float snorm16ToFloat(int16_t v)
{
	// -32768 & -32767 both map to -1 according to D3D10 rules.
	return _std max(v / 32767.0f, -1.0f);
}

inline halfFloat_t f32_to_f16(float f) {
	uint32_t x = (uint32_t)f;
	uint32_t sign = (unsigned short)(x >> 31);
	uint32_t mantissa;
	uint32_t exp;
	halfFloat_t hf;

	mantissa = x & ((1 << 23) - 1);
	exp = x & (0xFF << 23);
	if (exp >= 0x47800000) {
		// check if the original number is a NaN
		if (mantissa && (exp == (0xFF << 23))) {
			// single precision NaN
			mantissa = (1 << 23) - 1;
		}
		else {
			// half-float will be Inf
			mantissa = 0;
		}
		hf = (((halfFloat_t)sign) << 15) | (halfFloat_t)((0x1F << 10)) |
			(halfFloat_t)(mantissa >> 13);
	}
	// check if exponent is <= -15
	else if (exp <= 0x38000000) {
		hf = 0;  // too small to be represented
	}
	else {
		hf = (((halfFloat_t)sign) << 15) | (halfFloat_t)((exp - 0x38000000) >> 13) |
			(halfFloat_t)(mantissa >> 13);
	}

	return hf;
}

/*
========================
F16toF32
========================
*/
inline float F16toF32(halfFloat_t x)
{
	int e = HF_EXP(x);
	int m = HF_MANTISSA(x);
	int s = HF_SIGN(x);

	if (0 < e && e < 31)
	{
		return s * powf(2.0f, (e - 15.0f)) * (1 + m / 1024.0f);
	}
	else if (m == 0)
	{
		return s * 0.0f;
	}
	return s * powf(2.0f, -14.0f) * (m / 1024.0f);
}

/*
========================
F32toF16
========================
*/
inline halfFloat_t F32toF16(float a)
{
	unsigned int f = *(unsigned*)(&a);
	unsigned int signbit = (f & 0x80000000) >> 16;
	int exponent = ((f & 0x7F800000) >> 23) - 112;
	unsigned int mantissa = (f & 0x007FFFFF);

	if (exponent <= 0)
	{
		return 0;
	}
	if (exponent > 30)
	{
		return (halfFloat_t)(signbit | 0x7BFF);
	}

	return (halfFloat_t)(signbit | (exponent << 10) | (mantissa >> 13));
}

#endif // !JS3D_MATH_CONVERTERS_H
