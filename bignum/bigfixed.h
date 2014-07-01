#pragma once

#include "bigint.h"
#include "mathprimatives.h"

namespace bignum
{
	template <size_t numwords, size_t numwords_frac>
	class bigfixed
	{
		public:
			typedef bigfixed <numwords, numwords_frac> this_t;
			typedef bigint<numwords + numwords_frac, true> internal_t;
			typedef bigint<numwords + numwords_frac * 2, true> intermediate_t;

			const static size_t size_words_whole = numwords;
			const static size_t size_words_frac  = numwords_frac;
			const static size_t size_words       = numwords + numwords_frac;

			const static size_t size_bits_whole = size_words_whole * 32;
			const static size_t size_bits_frac  = size_words_frac * 32;
			const static size_t size_bits       = size_words * 32;


		private:
			template <size_t numwords2, size_t numwords_frac2> friend class bigfixed;
			
			internal_t m_internalValue;

			internal_t& getInternal ()
			{
				return m_internalValue;
			}

			bigfixed (internal_t value) : m_internalValue(value) 
			{
			}

		public:
			bigfixed () : m_internalValue(0) 
			{
			}

			bigfixed (int value) : m_internalValue(value) 
			{
				m_internalValue <<= size_bits_frac;
			}

			bigfixed (__int64 value) : m_internalValue(value) 
			{
				m_internalValue <<= size_bits_frac;
			}

			bigfixed (double value)  
			{
				int sign     = mathprim::getDoubleSign(value);
				int exponent = mathprim::getDoubleExponent(value);

				if (exponent == 0)  // number is subnormal treat as 0;
					return;

				unsigned __int64 mantissa = mathprim::getDoubleMantissa(value) + 0x0010000000000000UL;

				m_internalValue = internal_t(mantissa);
				int shiftAmmount = exponent - 0x3ff + (size_bits_frac - 52);

				if (shiftAmmount < 0)
					m_internalValue >>= abs(shiftAmmount);

				if (shiftAmmount > 0)
					m_internalValue <<= shiftAmmount;

				if (sign)
					m_internalValue = -m_internalValue;
			}

			bigfixed (float value)  
			{
				*this = this_t ((double)value);
			}

			// ==============================================================
			//      Arithmetic operators
			// ==============================================================

			inline this_t operator+(const this_t& value) const
			{
				return *this + value;
			}

			inline this_t operator-(const this_t& value) const
			{
				return *this - value;
			}

			inline this_t operator-() const
			{
				return -m_internalValue;
			}

			inline this_t& operator+=(const this_t& value)
			{
				return *this += value;
			}

			inline this_t& operator-=(const this_t& value)
			{
				return *this -= value;
			}

			inline this_t operator*(const this_t& value) const
			{
				intermediate_t a = m_internalValue.cast<intermediate_t>();
				intermediate_t b = value.m_internalValue.cast<intermediate_t>();

				intermediate_t res = a * b;
				res =>> size_bits_frac;

				return this_t(res.cast<internal_t>());
			}

			inline this_t operator/(const this_t& value) const
			{
				intermediate_t a = m_internalValue.cast<intermediate_t>();
				intermediate_t b = value.m_internalValue.cast<intermediate_t>();

				a =<< size_bits_frac;
				b =<< size_bits_frac;	
				
				intermediate_t res = a / b;

				return this_t(res.cast<internal_t>());
			}

			inline this_t& operator*=(const this_t& value)
			{
				*this = *this * value;
				return *this;
			}

			inline this_t& operator/=(const this_t& value)
			{
				*this = *this / value;
				return *this;
			}

			// ==============================================================
			//      conversion functions
			// ==============================================================

			double toDouble () const
			{
				if (m_internalValue.isZero())
					return 0;

				bool neg = m_internalValue.isNegative();
				internal_t absValue = neg?-m_internalValue:m_internalValue;

				size_t indexMSB = absValue.indexMSB();
				size_t exponent = indexMSB - size_bits_frac; 

				if (indexMSB > 52) 
					absValue >>= (indexMSB - 52);

				if (indexMSB < 52)
					absValue <<= (52 - indexMSB);

				exponent += 0x3ff; 

				mathprim::compound_u64 mantissa;

				mantissa.lo = absValue.getWord (0);
				mantissa.hi = absValue.getWord (1);
				
				return mathprim::makeDouble(mantissa.u64_value, exponent, neg);
			}

			std::string toDecString () const
			{
				std::string out;

				if (m_internalValue == 0) return "0.0";

				bool neg = m_internalValue.isNegative();

				internal_t absValue = neg?-m_internalValue:m_internalValue;
				internal_t wholePart = absValue >> size_bits_frac;  // shift out the fractional part
				internal_t fractionalPart = absValue - (wholePart << size_bits_frac);

				out = wholePart.toDecString() + ".";

				std::string fracstr;
				while (fractionalPart != 0)
				{
					internal_t::mulBy10(fractionalPart, fractionalPart);
					fracstr += '0' + fractionalPart.getWord(size_words_frac);
					fractionalPart.setWord(size_words_frac, 0);
				}

				if (fracstr.size() == 0)
					fracstr += '0';

				return std::string (neg?"-":"") + out + fracstr;
			}

			static this_t fromDecString (const std::string& s)
			{
				return this_t;
			}

			// ==============================================================
			//      comparison operators
			// ==============================================================

			inline bool operator == (const this_t& value) const
			{
				return m_internalValue == value.m_internalValue;
			}

			inline bool operator >  (const this_t& value) const
			{
				return m_internalValue > value.m_internalValue;
			}

			inline bool operator <  (const this_t& value) const
			{
				return m_internalValue < value.m_internalValue;
			}

			inline bool operator >= (const this_t& value) const
			{
				return m_internalValue >= value.m_internalValue;
			}

			inline bool operator <= (const this_t& value) const
			{
				return m_internalValue <= value.m_internalValue;
			}

			inline bool operator != (const this_t& value) const
			{
				return m_internalValue != value.m_internalValue;
			}

			inline bool isNegative () const
			{
				return m_internalValue.isNegative();
			}

			// ==============================================================
			//      Misc
			// ==============================================================

			inline this_t maxValue () const
			{
				return m_internalValue.maxValue();
			}

			inline this_t minValue () const
			{
				return m_internalValue.minValue();
			}

	};

}
