/*
 * float32e8_t.h
 *
 *  Created on: 22.05.2011
 *      Author: Bernd
 */

#ifndef FLOAT32E8_T_H_
#define FLOAT32E8_T_H_

#include <iostream>
using namespace std;

#include <string>
#include <math.h>

#ifndef NO_SIMUTRANS
	#include "../simtypes.h"
#else
	typedef unsigned	long long	uint64;
	typedef 			long long	sint64;
	typedef unsigned	long		uint32;
	typedef 			long		sint32;
	typedef unsigned	short		uint16;
	typedef 			short		sint16;
	typedef unsigned	char  		uint8;
#endif

class float32e8_t
{
private:
	uint32 m;	// mantissa
	sint16 e;	// exponent
	bool ms:1;	// sign of manitissa

	inline void set_zero() { m = 0L; e = 0; ms = false; }
public:
	static const uint8 bpm = 32; // bits per mantissa
	static const uint8 bpe = 10; // bits per exponent
	static const sint16 min_exponent;
	static const sint16 max_exponent;
	static const uint32 max_mantissa;
	static const float32e8_t zero;
	static const float32e8_t half;
	static const float32e8_t one;

	inline float32e8_t() {};

	inline float32e8_t(const float32e8_t &value) { m = value.m; e = value.e; ms = value.ms; }
	inline float32e8_t(const uint32 mantissa, const sint16 exponent, const bool negative_man) { m = mantissa; e = exponent; ms = negative_man; }
	inline void set_value(const float32e8_t &value) { m = value.m; e = value.e; ms = value.ms; }

#ifdef USE_DOUBLE
	inline float32e8_t(const double value) { set_value(value); }
	void set_value(const double value);
	inline const float32e8_t & operator = (const double value)	{ set_value(value); return *this; }
#endif

	inline float32e8_t(const sint32 value) { set_value(value); }
	inline float32e8_t(const uint32 value) { set_value(value); }
	inline float32e8_t(const sint64 value) { set_value(value); }
	inline float32e8_t(const uint64 value) { set_value(value); }

	inline float32e8_t(const sint32 nominator, const sint32 denominator) { set_value(float32e8_t(nominator) / float32e8_t(denominator)); }
	inline float32e8_t(const uint32 nominator, const uint32 denominator) { set_value(float32e8_t(nominator) / float32e8_t(denominator)); }
	inline float32e8_t(const sint64 nominator, const sint64 denominator) { set_value(float32e8_t(nominator) / float32e8_t(denominator)); }
	inline float32e8_t(const uint64 nominator, const uint64 denominator) { set_value(float32e8_t(nominator) / float32e8_t(denominator)); }

	void set_value(const sint32 value);
	void set_value(const uint32 value);
	void set_value(const sint64 value);
	void set_value(const uint64 value);

	inline const float32e8_t & operator = (const sint32 value)	{ set_value(value);	return *this; }
	inline const float32e8_t & operator = (const uint32 value)	{ set_value(value);	return *this; }
	inline const float32e8_t & operator = (const sint64 value)	{ set_value(value);	return *this; }
	inline const float32e8_t & operator = (const uint64 value)	{ set_value(value);	return *this; }

	inline bool operator < (const float32e8_t &value) const
	{
		if (ms)
			return !value.ms ||	  e > value.e || (e == value.e && m > value.m);
		else
			return 	!value.ms && (e < value.e || (e == value.e && m < value.m));
	}

	inline bool operator <= (const float32e8_t &value) const
	{
		if (ms)
			return !value.ms ||	  e > value.e || (e == value.e && m >= value.m);
		else
			return !value.ms && (e < value.e || (e == value.e && m <= value.m));
	}

	inline bool operator > (const float32e8_t &value) const
	{
		if (ms)
			return  value.ms &&	 (e < value.e || (e == value.e && m < value.m));
		else
			return  value.ms ||  e > value.e || (e == value.e && m > value.m);
	}

	inline bool operator >= (const float32e8_t &value) const
	{
		if (ms)
			return  value.ms && (e < value.e || (e == value.e && m <= value.m));
		else
			return  value.ms ||  e > value.e || (e == value.e && m >= value.m);
	}

	inline bool operator == (const float32e8_t &value) const { return m == value.m && e == value.e && ms == value.ms; }
	inline bool operator != (const float32e8_t &value) const { return m != value.m || e != value.e || ms != value.ms; }

	inline bool operator <  (const sint32 value) const { return *this <  float32e8_t((sint32) value); }
	inline bool operator <= (const sint32 value) const { return *this <= float32e8_t((sint32) value); }
	inline bool operator == (const sint32 value) const { return *this == float32e8_t((sint32) value); }
	inline bool operator != (const sint32 value) const { return *this != float32e8_t((sint32) value); }
	inline bool operator >= (const sint32 value) const { return *this >= float32e8_t((sint32) value); }
	inline bool operator >  (const sint32 value) const { return *this >  float32e8_t((sint32) value); }

	inline bool operator <  (const sint64 value) const { return *this <  float32e8_t((sint64) value); }
	inline bool operator <= (const sint64 value) const { return *this <= float32e8_t((sint64) value); }
	inline bool operator == (const sint64 value) const { return *this == float32e8_t((sint64) value); }
	inline bool operator != (const sint64 value) const { return *this != float32e8_t((sint64) value); }
	inline bool operator >= (const sint64 value) const { return *this >= float32e8_t((sint64) value); }
	inline bool operator >  (const sint64 value) const { return *this >  float32e8_t((sint64) value); }

	inline const float32e8_t operator - () const { return float32e8_t(m, e, !ms); }

	const float32e8_t operator + (const float32e8_t &value) const;
	const float32e8_t operator - (const float32e8_t &value) const;
	const float32e8_t operator * (const float32e8_t &value) const;
	const float32e8_t operator / (const float32e8_t &value) const;

	inline const float32e8_t operator + (const sint32 value) const { return *this + float32e8_t(value); } 
	inline const float32e8_t operator - (const sint32 value) const { return *this - float32e8_t(value); } 
	inline const float32e8_t operator * (const sint32 value) const { return *this * float32e8_t(value); } 
	inline const float32e8_t operator / (const sint32 value) const { return *this / float32e8_t(value); } 

	inline const float32e8_t operator + (const uint32 value) const { return *this + float32e8_t(value); } 
	inline const float32e8_t operator - (const uint32 value) const { return *this - float32e8_t(value); } 
	inline const float32e8_t operator * (const uint32 value) const { return *this * float32e8_t(value); } 
	inline const float32e8_t operator / (const uint32 value) const { return *this / float32e8_t(value); } 

	inline const float32e8_t operator + (const sint64 value) const { return *this + float32e8_t(value); }
	inline const float32e8_t operator - (const sint64 value) const { return *this - float32e8_t(value); }
	inline const float32e8_t operator * (const sint64 value) const { return *this * float32e8_t(value); }
	inline const float32e8_t operator / (const sint64 value) const { return *this / float32e8_t(value); }

	inline const float32e8_t operator + (const uint64 value) const { return *this + float32e8_t(value); }
	inline const float32e8_t operator - (const uint64 value) const { return *this - float32e8_t(value); }
	inline const float32e8_t operator * (const uint64 value) const { return *this * float32e8_t(value); }
	inline const float32e8_t operator / (const uint64 value) const { return *this / float32e8_t(value); }

	inline const float32e8_t & operator += (const float32e8_t &value) { set_value(*this + value); return *this; }
	inline const float32e8_t & operator -= (const float32e8_t &value) { set_value(*this - value); return *this; }
	inline const float32e8_t & operator *= (const float32e8_t &value) { set_value(*this * value); return *this; }
	inline const float32e8_t & operator /= (const float32e8_t &value) { set_value(*this / value); return *this; }

	inline const float32e8_t & operator += (const sint32 value) { set_value(*this + value); return *this; }
	inline const float32e8_t & operator -= (const sint32 value) { set_value(*this - value); return *this; }
	inline const float32e8_t & operator *= (const sint32 value) { set_value(*this * value); return *this; }
	inline const float32e8_t & operator /= (const sint32 value) { set_value(*this / value); return *this; }

	inline const float32e8_t & operator += (const uint32 value) { set_value(*this + value); return *this; }
	inline const float32e8_t & operator -= (const uint32 value) { set_value(*this - value); return *this; }
	inline const float32e8_t & operator *= (const uint32 value) { set_value(*this * value); return *this; }
	inline const float32e8_t & operator /= (const uint32 value) { set_value(*this / value); return *this; }

	inline const float32e8_t & operator += (const sint64 value) { set_value(*this + value); return *this; }
	inline const float32e8_t & operator -= (const sint64 value) { set_value(*this - value); return *this; }
	inline const float32e8_t & operator *= (const sint64 value) { set_value(*this * value); return *this; }
	inline const float32e8_t & operator /= (const sint64 value) { set_value(*this / value); return *this; }

	inline const float32e8_t & operator += (const uint64 value) { set_value(*this + value); return *this; }
	inline const float32e8_t & operator -= (const uint64 value) { set_value(*this - value); return *this; }
	inline const float32e8_t & operator *= (const uint64 value) { set_value(*this * value); return *this; }
	inline const float32e8_t & operator /= (const uint64 value) { set_value(*this / value); return *this; }

	inline const float32e8_t abs() const { return ms ? float32e8_t(m, e, false) : *this; }
	const float32e8_t log2() const;
	const float32e8_t exp2() const;

private:
#ifdef USE_DOUBLE
public:
#else
	friend ostream & operator << (ostream &out, const float32e8_t &x);
#endif
	double to_double() const;
public:
	sint32 to_sint32() const;
	//const string to_string() const;

	inline operator sint32 () const { return to_sint32(); }
};

ostream & operator << (ostream &out, const float32e8_t &x);

inline const float32e8_t operator + (const sint32 x, const float32e8_t &y) {return float32e8_t(x) + y; }
inline const float32e8_t operator - (const sint32 x, const float32e8_t &y) {return float32e8_t(x) - y; }
inline const float32e8_t operator * (const sint32 x, const float32e8_t &y) {return float32e8_t(x) * y; }
inline const float32e8_t operator / (const sint32 x, const float32e8_t &y) {return float32e8_t(x) / y; }

inline const float32e8_t operator + (const uint32 x, const float32e8_t &y) {return float32e8_t(x) + y; }
inline const float32e8_t operator - (const uint32 x, const float32e8_t &y) {return float32e8_t(x) - y; }
inline const float32e8_t operator * (const uint32 x, const float32e8_t &y) {return float32e8_t(x) * y; }
inline const float32e8_t operator / (const uint32 x, const float32e8_t &y) {return float32e8_t(x) / y; }

inline const float32e8_t operator + (const sint64 x, const float32e8_t &y) {return float32e8_t(x) + y; }
inline const float32e8_t operator - (const sint64 x, const float32e8_t &y) {return float32e8_t(x) - y; }
inline const float32e8_t operator * (const sint64 x, const float32e8_t &y) {return float32e8_t(x) * y; }
inline const float32e8_t operator / (const sint64 x, const float32e8_t &y) {return float32e8_t(x) / y; }

inline const float32e8_t operator + (const uint64 x, const float32e8_t &y) {return float32e8_t(x) + y; }
inline const float32e8_t operator - (const uint64 x, const float32e8_t &y) {return float32e8_t(x) - y; }
inline const float32e8_t operator * (const uint64 x, const float32e8_t &y) {return float32e8_t(x) * y; }
inline const float32e8_t operator / (const uint64 x, const float32e8_t &y) {return float32e8_t(x) / y; }

inline const float32e8_t abs(const float32e8_t &x) { return x.abs(); }
inline const float32e8_t log2(const float32e8_t &x) { return x.log2(); }
inline const float32e8_t exp2(const float32e8_t &x) { return x.exp2(); }
inline const float32e8_t pow(const float32e8_t &base, const float32e8_t &expo) { return exp2(expo * base.log2()); }

class float32e8_exception_t {
private:
	std::string message;
public:
	float32e8_exception_t(const char* message)
	{
		this->message = message;
	}

	const char* get_message()  const { return message.c_str(); }
};


#endif /* FLOAT32E8_T_H_ */
