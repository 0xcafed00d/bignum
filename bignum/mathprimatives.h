#pragma once

namespace bignum
{
	namespace mathprim 
	{
		typedef unsigned int u32;
		typedef int i32;
		typedef unsigned __int64 u64;
		typedef __int64 i64;

	    static const u64 u64_topmask = 0xffffffff00000000LL;
	    static const u64 u64_botmask = 0x00000000ffffffffLL;
	    static const u32 MSB_mask = 0x80000000U;

		union doubleVal
		{
			double value;
			unsigned __int64 intval;
		};

		inline unsigned __int64 getDoubleMantissa (double value)
		{
			doubleVal v;
			v.value = value;

			unsigned __int64 mask = ((__int64(1) << 52) - 1);

			return v.intval & mask;
		}

		inline int getDoubleExponent (double value)
		{
			doubleVal v;
			v.value = value;

			int exponent = (int)((v.intval >> 52) & 0x7ff);

			return exponent;
		}

		inline int getDoubleSign (double value)
		{
			doubleVal v;
			v.value = value;

			unsigned __int64 mask = ((__int64(1) << 63));

			return (v.intval & mask) != 0;
		}

		inline double makeDouble (unsigned __int64 mantissa, size_t exponent, int sign)
		{
			doubleVal val;
			
			val.intval = mantissa & ((__int64(1) << 52) - 1);
			val.intval |= unsigned __int64(exponent & 0x7ff) << 52;
			
			if (sign)
				val.intval |= ((unsigned __int64(1) << 63));

			return val.value;
		}


		inline size_t numLeadingZeros (unsigned int x) 
		{
			size_t n; 

			if (x == 0) return(32); 
			n = 1; 
			if ((x >> 16) == 0) {n = n +16; x = x <<16;} 
			if ((x >> 24) == 0) {n = n + 8; x = x << 8;} 
			if ((x >> 28) == 0) {n = n + 4; x = x << 4;} 
			if ((x >> 30) == 0) {n = n + 2; x = x << 2;} 
			n = n - (x >> 31); 
			return n; 
		}

		// returns the index of the most significant bit set - or 0xffffffff if no bits set 
		inline size_t indexMSB (unsigned int x)
		{
			return 31 - numLeadingZeros (x);
		}

		union compound_u64 
		{
			struct 
			{
				u32 lo;
				u32 hi;
			};
			u64 u64_value;
		};


	    inline u32 hexCharTou32 (char c)
	    {
	        c = tolower(c);
	        if (c >= '0' && c <= '9')
	            return c - '0';

	        if (c >= 'a' && c <= 'f')
	            return c - 'a' + 10;

	        return 0;
	    }

	    inline u32 addWithCarry (u32 a, u32 b, u32& carry)
	    {
	        compound_u64 res;
	        res.u64_value = u64(a) + u64(b) + u64(carry);
	        carry = res.u64_value & u64_topmask ? 1 : 0;
	        return res.lo;
	    }

	    inline compound_u64 mul32x32 (u32 a, u32 b)
	    {
	        compound_u64 res;
	        res.u64_value = u64(a) * u64(b);
	        return res;
	    }

	    // logical shift left
	    inline compound_u64 LSL (u32 a, size_t bits)
	    {
	        compound_u64 res;
	        res.hi = 0;
	        res.lo = a;

	        res.u64_value = res.u64_value << bits;
	        return res;
	    }

	    // arithmetic shift right
	    inline compound_u64 ASR (u32 a, size_t bits)
	    {
	        compound_u64 res;
	        res.hi = a;
	        res.lo = 0;

	        res.u64_value = u64(i64(res.u64_value) >> bits);
	        return res;
	    }

	    // logical shift right
	    inline compound_u64 LSR (u32 a, size_t bits)
	    {
	        compound_u64 res;
	        res.hi = a;
	        res.lo = 0;

	        res.u64_value = res.u64_value >> bits;
	        return res;
	    }
	}
}