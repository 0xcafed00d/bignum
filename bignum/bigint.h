#pragma once

#include <string>
#include <algorithm>
#include <stdexcept>
#include <ctype.h>

#include "mathprimatives.h"

namespace bignum
{
	template <size_t numwords, bool issigned>
	class bigint
	{
		public:
			typedef bigint <numwords, issigned> this_t;
			static const size_t size_bits  = numwords * 32;
			static const size_t size_words = numwords;
			static const bool is_signed = issigned;

		private:
			template <size_t numwords2, bool issigned2> friend class bigint;

			// m_words[0] == LSW, m_words[numwords-1] = MSW
			mathprim::u32 m_words[numwords];

			// ==============================================================
			//      Utility Functions
			// ==============================================================

		public:
			static void add (const this_t& a, const this_t& b, this_t& result) 
			{
				mathprim::u32 carry = 0;
				for (int n = 0; n < numwords; n++)
					result.m_words[n] = mathprim::addWithCarry(a.m_words[n], b.m_words[n], carry);
			}

			static void sub (const this_t& a, const this_t& b, this_t& result) 
			{
				this_t b_neg;
				twosComplement(b, b_neg);
				add (a, b_neg, result);
			}

			// result = a << 32;
			static void shiftLeft32Bit (const this_t& a, this_t& result) 
			{
				for (int n = numwords - 2; n >= 0; n--)
					result.m_words[n + 1] = a.m_words[n];
				result.m_words[0] = 0;	
			}

			// result = a << bits;
			static void shiftLeft (const this_t& a, size_t bits, this_t& result) 
			{
				this_t val = a;

				for (;bits >= 32; bits -= 32)
					shiftLeft32Bit (val, val);

				if (bits)
				{
					for (int n = numwords - 1; n >= 0; n--)
					{
						mathprim::compound_u64 v = mathprim::LSL(val.m_words[n], bits);
						val.m_words[n] = v.lo;

						if (n != numwords - 1)
							val.m_words[n+1] |= v.hi;
					}
				}

				result = val;
			}

			// result = a >> 32 (unsigned shift);
			static void shiftRight32BitUnsigned (const this_t& a, this_t& result) 
			{
				for (int n = 1; n < numwords; n++)
					result.m_words[n - 1] = a.m_words[n];
				result.m_words[numwords - 1] = 0;	
			}

			// result = a >> 32 (signed shift - sign bit extended);
			static void shiftRight32BitSigned (const this_t& a, this_t& result) 
			{
				bool isNeg = a.isNegative();
				
				shiftRight32BitUnsigned(a, result);
				result.m_words[numwords - 1] = isNeg?mathprim::u32(-1):0;
			}

			static void shiftRightSigned (const this_t& a, size_t bits, this_t& result) 
			{			
				this_t val = a;

				for (;bits >= 32; bits -= 32)
					shiftRight32BitSigned (val, val);

				if (bits)
				{
					for (int n = 0; n < numwords - 1; n++)
					{
						mathprim::compound_u64 v = mathprim::LSR(val.m_words[n], bits);
						val.m_words[n] = v.hi;

						if (n != 0)
							val.m_words[n-1] |= v.lo;
					}
					mathprim::compound_u64 v = mathprim::ASR(val.m_words[numwords - 1], bits);
					val.m_words[numwords - 1] = v.hi;
					val.m_words[numwords - 2] |= v.lo;
				}
				result = val;
			}

			static void shiftRightUnsigned (const this_t& a, size_t bits, this_t& result) 
			{
				this_t val = a;

				for (;bits >= 32; bits -= 32)
					shiftRight32BitUnsigned (val, val);

				if (bits)
				{
					for (int n = 0; n < numwords; n++)
					{
						mathprim::compound_u64 v = mathprim::LSR(val.m_words[n], bits);
						val.m_words[n] = v.hi;

						if (n != 0)
							val.m_words[n-1] |= v.lo;
					}
				}
				result = val;
			}

			static void and (const this_t& a, const this_t& b, this_t& result) 
			{
				for (int n = 0; n < numwords; n++)
					result.m_words[n] = a.m_words[n] & b.m_words[n];
			}

			static void or (const this_t& a, const this_t& b, this_t& result) 
			{
				for (int n = 0; n < numwords; n++)
					result.m_words[n] = a.m_words[n] | b.m_words[n];
			}

			static void xor (const this_t& a, const this_t& b, this_t& result) 
			{
				for (int n = 0; n < numwords; n++)
					result.m_words[n] = a.m_words[n] ^ b.m_words[n];
			}

			static void onesCompliment (const this_t& a, this_t& result) 
			{
				for (int n = 0; n < numwords; n++)
					result.m_words[n] = ~a.m_words[n];
			}

			static void smul (const this_t& a, const this_t& b, this_t& result) 
			{
				bool aneg = a.isNegative();
				bool bneg = b.isNegative();

				this_t a2 = aneg?-a:a;
				this_t b2 = bneg?-b:b;

				umul (a2, b2, result);

				if (aneg != bneg)
					this_t::twosComplement(result, result);
			}

			static void umul (const this_t& a, const this_t& b, this_t& result) 
			{
				this_t res = 0;
				for (int n = 0; n < numwords; n++)
				{
					for (int m = 0; m < numwords; m++)
					{
						if (a.m_words[n] == 0 || b.m_words[m] == 0) 
							continue;

						mathprim::compound_u64 x = mathprim::mul32x32(a.m_words[n], b.m_words[m]);
						this_t part (x.u64_value);
						shiftLeft (part, (n + m) * 32, part);
						add (res, part, res);
					}
				}

				result = res;
			}

			// result = dividend/divisor  - signed
			static void sdiv (const this_t& a, const this_t& b, this_t& result, this_t& modulo) 
			{
				bool aneg = a.isNegative();
				bool bneg = b.isNegative();

				this_t a2 = aneg?-a:a;
				this_t b2 = bneg?-b:b;

				udiv (a2, b2, result, modulo);

				if (aneg != bneg)
					this_t::twosComplement(result, result);
			}

			// result = dividend/divisor  - unsigned
			static void udiv (const this_t& dividend, const this_t& divisor, this_t& result, this_t& modulo) 
			{
				this_t quotient(0);
				this_t temp(0);

				if (divisor == 0) 
					throw std::invalid_argument("Divide By Zero");
	            
				for (int n = size_bits - 1; n >= 0; n--)
				{
					temp <<= 1;
					temp.setBit(0, dividend.getBit(n));

					if (temp >= divisor)
					{
						quotient.setBit(n, true);
						temp -= divisor;
					}
				}

				modulo = temp;
				result = quotient;
			}

	//         static void udivBy10 (const this_t& a, this_t& result) 
	//         {
	//             unsigned q, r;
	//             q = (a >> 1) + (a >> 2);
	//             q = q + (q >> 4);
	//             q = q + (q >> 8);
	//             q = q + (q >> 16);
	//             q = q >> 3;
	//             r = a - q*10;
	//             return q + ((r + 6) >> 4);
	//         }

			static void udivBy10 (const this_t& a, this_t& result) 
			{
				this_t q;
				this_t r;
				this_t temp;

				shiftRightSigned(a, 1, q);
				shiftRightSigned(a, 2, temp);
				add (q, temp, q);

				size_t shiftval = 4;
				for (int n = 0; n < numwords * 3; n++)
				{
					shiftRightSigned(q, shiftval, temp);
					add (q, temp, q);
					shiftval *= 2;
				}

				shiftRightSigned(q, 3, q);

				umul (q, 10, temp);
				sub (a, temp, r);
				
				add (r, 6, r);
				shiftRightSigned(r, 4, r);

				add (q, r, result);
			}
			
			static void mulBy10 (const this_t& a, this_t& result) 
			{
				this_t temp;
				shiftLeft(a, 3, temp);
				add (a, temp, temp);
				add (a, temp, result);
			}

			// return < 0 if a < b;  0 if a == b; > 0 if a > b 
			static int unsignedCompare (const this_t& a, const this_t& b) 
			{
				for (int n = numwords - 1; n >= 0; n--)
				{
					if (a.m_words[n] < b.m_words[n]) return -1;
					if (a.m_words[n] > b.m_words[n]) return 1;
				}

				return 0;
			}

			// return < 0 if a < b;  0 if a == b; > 0 if a > b 
			static int signedCompare (const this_t& a, const this_t& b) 
			{
				bool a_neg = a.isNegative();
				bool b_neg = b.isNegative();

				if (a_neg && !b_neg) return -1;
				if (!a_neg && b_neg) return 1;

				return unsignedCompare(a, b);
			}

			static void twosComplement (const this_t& a, this_t& result) 
			{
				onesCompliment (a, result);	
				add (result, mathprim::u32(1), result);
			}

		public: 

	// ==============================================================
	//      Construction
	// ==============================================================

			bigint ()
			{
				for (int n = 0; n < numwords; n++)
					m_words[n] = 0;
			}

			bigint (int value)
			{
				if (value < 0)
				{
					*this = this_t(unsigned int(-value));
					twosComplement(*this, *this);
				}
				else
				{
					*this = this_t(unsigned int(value));
				}
			}

			bigint (__int64 value)
			{
				if (value < 0)
				{
					*this = this_t(unsigned __int64(-value));
					twosComplement(*this, *this);
				}
				else
				{
					*this = this_t(unsigned __int64(value));
				}
			}


			bigint (unsigned int value)
			{
				m_words[0] = value;
				for (int n = 1; n < numwords; n++)
					m_words[n] = 0;
			}

			bigint (unsigned __int64 value)
			{
				mathprim::compound_u64 cvalue; 
				cvalue.u64_value = value;

				m_words[0] = cvalue.lo;
				m_words[1] = cvalue.hi;
				for (int n = 2; n < numwords; n++)
					m_words[n] = 0;
			}

	// ==============================================================
	//      Conversion
	// ==============================================================

			template < typename new_bigint_t >
			new_bigint_t cast ()
			{
				new_bigint_t result;

				bool signExtend = this_t::is_signed && isNegative();

				for (size_t n = 0; n < std::min(numwords, new_bigint_t::size_words); n++)
				{
					result.m_words[n] = m_words[n];
				}

				for (size_t n = numwords; n < new_bigint_t::size_words; n++)
				{
					result.m_words[n] = signExtend?0xffffffff:0;
				}
				return result;
			}

			std::string toDecString () const
			{
				std::string out;

				if (*this == 0) return "0";

				bool neg = isNegative();

				this_t temp = neg?-*this:*this;
				this_t temp2 = temp;

				while (temp != 0)
				{
					udivBy10(temp, temp);
					mathprim::u32 mod10 = (temp2 - (temp * 10)).m_words[0]; 
					out += ('0' + mod10);
					temp2 = temp;
				}

				if (neg) out += '-';
				
				std::reverse (out.begin(), out.end());
				return out;
			}

			std::string toHexString () const
			{
				const size_t bufferSize = sizeof(mathprim::u32) * numwords * 2;

				char buffer[bufferSize + 1];
				buffer[bufferSize] = 0;

				size_t index = bufferSize - 1;
				for (size_t w = 0; w < numwords; w++)
				{
					mathprim::u32 word = m_words[w];

					for (size_t n = 0; n < (sizeof(mathprim::u32) * 2); n++)
					{
						buffer[index] = "0123456789abcdef"[word & 0xf];
						word = word >> 4;
						index--;
					}
				}

				return buffer;
			}

			int toInt () const
			{
				return (int)m_words[0];
			}

			__int64 toInt64 () const
			{
				compound_u64 res;
				res.lo = m_words[0];
				res.hi = m_words[1];

				return (__int64)res.u64_value;
			}

			unsigned int toUnsignedInt () const
			{
				return m_words[0];
			}

			unsigned __int64 toInsignednt64 () const
			{
				compound_u64 res;
				res.lo = m_words[0];
				res.hi = m_words[1];

				return res.u64_value;
			}

			static this_t fromDecString (std::string& str)
			{
				return fromDecString (str.c_str());
			}		

			static this_t fromDecString (const char* str)
			{
				this_t value = 0;
				bool isneg = false;

				if (*str == '-')
				{
					isneg = true;
					str++;
				}

				while (*str)
				{
					if (isdigit(*str))
					{
						mulBy10(value, value);
						value += *str - '0';
						str++;
					}
					else
					{
						throw std::invalid_argument("Invalid Decimal Digit");
					}
				}

				if (isneg)
					twosComplement(value, value);

				return value;
			}

			static this_t fromHexString (std::string& str)
			{
				return fromHexString (str.c_str());
			}		

			static this_t fromHexString (const char* str)
			{
				this_t value = 0;
				bool isneg = false;

				if (*str == '-')
				{
					isneg = true;
					str++;
				}

				if (_strnicmp (str, "0x", 2) == 0)
					str += 2;
				else
					throw std::invalid_argument("Invalid Hexadecimal Format");

				while (*str)
				{
					if (isxdigit(*str))
					{
						shiftLeft(value, 4, value);
						value += mathprim::hexCharTou32 (*str);
						str++;
					}
					else
					{
						throw std::invalid_argument("Invalid Hexadecimal Digit");
					}
				}

				if (isneg)
					twosComplement(value, value);

				return value;
			}


	// ==============================================================
	//      bit manipulation
	// ==============================================================

			inline bool getBit (size_t index) const 
			{
				size_t wordIndex = index / 32;
				size_t bitIndex = index - wordIndex * 32;
				return (m_words[wordIndex] & (0x1 << bitIndex)) != 0;
			}

			inline void setBit (size_t index, bool value)
			{
				size_t wordIndex = index / 32;
				size_t bitIndex = index - wordIndex * 32;

				if (value)
					m_words[wordIndex] = m_words[wordIndex] | (1 << bitIndex);
				else
					m_words[wordIndex] = m_words[wordIndex] & ~(1 << bitIndex);
			}


	// ==============================================================
	//      Arithmetic operators
	// ==============================================================

			inline this_t operator+(const this_t& value) const
			{
				this_t result;
				add (*this, value, result);
				return result;
			}

			inline this_t operator-(const this_t& value) const
			{
				this_t result;
				sub (*this, value, result);
				return result;
			}

			inline this_t operator-() const
			{
				this_t result;
				twosComplement(*this, result);
				return result;
			}

			inline this_t operator*(const this_t& value) const
			{
				this_t result;
				if (issigned)
					smul (*this, value, result);
				else
					umul (*this, value, result);

				return result;
			}

			inline this_t operator/(const this_t& value) const
			{
				this_t quotient;
				this_t modulo;
				if (issigned)
					sdiv (*this, value, quotient, modulo);
				else
					udiv (*this, value, quotient, modulo);

				return quotient;
			}

			inline this_t& operator+=(const this_t& value)
			{
				add (*this, value, *this);
				return *this;
			}

			inline this_t& operator-=(const this_t& value)
			{
				sub (*this, value, *this);
				return *this;
			}

			inline this_t& operator*=(const this_t& value)
			{
				if (issigned)
					smul (*this, value, *this);
				else
					umul (*this, value, *this);

				return *this;
			}

			inline this_t& operator/=(const this_t& value)
			{
				this_t modulo;
				if (issigned)
					sdiv (*this, value, *this, modulo);
				else
					udiv (*this, value, *this, modulo);

				return *this;
			}

			inline this_t& operator++ ()    // prefix ++x
			{
				add (*this, 1, *this);
				return *this;
			}

			inline this_t  operator++ (int) // postfix x++
			{
				this_t temp = *this;
				add (*this, 1, *this);
				return temp;
			}

			inline this_t& operator-- ()    // prefix --x
			{
				sub (*this, 1, *this);
				return *this;
			}

			inline this_t  operator-- (int) // postfix x--
			{
				this_t temp = *this;
				sub (*this, 1, *this);
				return temp;
			}

			inline this_t operator% (const this_t& value) const
			{
				this_t quotient;
				this_t modulo;
				if (issigned)
					sdiv (*this, value, quotient, modulo);
				else
					udiv (*this, value, quotient, modulo);

				return modulo;
			}

			inline this_t& operator%= (const this_t& value)
			{
				this_t quotient;
				if (issigned)
					sdiv (*this, value, quotient, *this);
				else
					udiv (*this, value, quotient, *this);

				return *this;
			}
	        

	// ==============================================================
	//      logical operators
	// ==============================================================

			inline this_t operator>>(size_t shift) const
			{
				this_t result;
				if (issigned)
					shiftRightSigned(*this, shift, result);
				else
					shiftRightUnsigned(*this, shift, result);

				return result;
			}

			inline this_t  operator<<(size_t shift) const
			{
				this_t result;
				shiftLeft(*this, shift, result);
				return result;
			}

			inline this_t& operator>>=(size_t shift)
			{
				if (issigned)
					shiftRightSigned(*this, shift, *this);
				else
					shiftRightUnsigned(*this, shift, *this);

				return *this;
			}

			inline this_t& operator<<=(size_t shift)
			{
				shiftLeft(*this, shift, *this);
				return *this;
			}

			inline this_t  operator^(const this_t& value) const
			{
				this_t result;
				xor (*this, value, result);
				return result;
			}

			inline this_t  operator|(const this_t& value) const
			{
				this_t result;
				or (*this, value, result);
				return result;
			}

			inline this_t  operator&(const this_t& value) const
			{
				this_t result;
				and (*this, value, result);
				return result;
			}

			inline this_t& operator^=(const this_t& value)
			{
				xor (*this, value, *this);
				return *this;
			}

			inline this_t& operator|=(const this_t& value)
			{
				or (*this, value, *this);
				return *this;
			}

			inline this_t& operator&=(const this_t& value)
			{
				and (*this, value, *this);
				return *this;
			}

			this_t operator ~() const
			{
				this_t result;
				onesCompliment(*this, result);
				return result;
			}

	//         inline operator bool () const
	//         {
	//             return unsignedCompare(*this, 0) != 0;
	//         }

	// ==============================================================
	//      comparison operators
	// ==============================================================

			inline bool operator == (const this_t& value) const
			{
				return unsignedCompare(*this, value) == 0;
			}

			inline bool operator >  (const this_t& value) const
			{
				int result;
				if (issigned)
					result = signedCompare(*this, value);
				else
					result = unsignedCompare(*this, value);

				return result > 0;
			}

			inline bool operator <  (const this_t& value) const
			{
				int result;
				if (issigned)
					result = signedCompare(*this, value);
				else
					result = unsignedCompare(*this, value);

				return result < 0;
			}

			inline bool operator >= (const this_t& value) const
			{
				int result;
				if (issigned)
					result = signedCompare(*this, value);
				else
					result = unsignedCompare(*this, value);

				return result > 0 || result == 0;
			}

			inline bool operator <= (const this_t& value) const
			{
				int result;
				if (issigned)
					result = signedCompare(*this, value);
				else
					result = unsignedCompare(*this, value);

				return result < 0 || result == 0;
			}

			inline bool operator != (const this_t& value) const
			{
				return unsignedCompare(*this, value) != 0;
			}

			inline bool isNegative () const
			{
				return (m_words[numwords - 1] & mathprim::MSB_mask) != 0;
			}

			inline bool isZero () const
			{
				for (int n = 0; n < numwords; n++)
					if (m_words[n])
						return false;

				return true;
			}

	// ==============================================================
	//      Misc
	// ==============================================================

			inline this_t maxValue () const
			{
				this_t res;
				for (int n = 0; n < numwords - 1; n++)
					res.m_words[n] = 0xffffffff;

				res.m_words[numwords - 1] = issigned?0x7fffffff:0xffffffff;

				return res;
			}

			inline this_t minValue () const
			{
				this_t res;
				if (signed)
				{
					for (int n = 0; n < numwords; n++)
						res.m_words[n] = 0;
				}
				else
				{
					for (int n = 0; n < numwords - 1; n++)
						res.m_words[n] = 0;

					res.m_words[numwords - 1] = 0x80000000;
				}

				return res;
			}

			mathprim::u32 getWord (size_t wordIndex) const
			{
				return m_words[wordIndex];
			}

			void setWord (size_t wordIndex, mathprim::u32 value)
			{
				m_words[wordIndex] = value;
			}

			// returns the index of the most significant bit set - or 0xffffffff if no bits set 
			size_t indexMSB () const 
			{
				size_t index = size_bits;

				for (int n = numwords - 1; n >= 0; n--)
				{
					index -= 32;
					if (m_words[n] != 0)
					{
						index += mathprim::indexMSB(m_words[n]);
						return index;
					}
				}

				return -1;
			}
	};

	template <typename numericType>
	numericType abs (const numericType& val)
	{
		if (val < numericType())
			return -val;
		else
			return val;
	}

	typedef bigint<4, true>   int128;
	typedef bigint<4, false> uint128;
	typedef bigint<8, true>   int256;
	typedef bigint<8, false> uint256;

}


