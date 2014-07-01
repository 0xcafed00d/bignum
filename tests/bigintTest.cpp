
#include "neo/Logging.h"

#include "bigintTest.h"


CREATE_LOGGING_CATEGORY (test);
USE_LOGGING_CATEGORY (test);

namespace neo
{

	bigintTest::bigintTest() : m_passed (true), m_numPassed(0), m_numFailed(0)
	{
	    
	}

	bool bigintTest::doTests()
	{
		TRACE_FUNCTION();
	 
		testPrimatives();
		testInitialise();
		testCompare();
		testAddSub ();
		testShift ();
		testMul ();
		testDiv ();

		LOGMSG (INFO, "");
		LOGMSG (INFO, neo::makeString("************************************************************"));
		LOGMSG (INFO, neo::makeString("**  Tests complete: ", m_passed?" [PASSED] ":" [FAILED] "));
		LOGMSG (INFO, neo::makeString("**    Tests Passed: ", m_numPassed));
		LOGMSG (INFO, neo::makeString("**    Tests Failed: ", m_numFailed));
		LOGMSG (INFO, neo::makeString("************************************************************"));
		LOGMSG (INFO, "");

		return m_passed;
	}

	void bigintTest::verify (const std::string& testname, bool outcome)
	{   
		if (!outcome) 
		{
			m_passed = false;
			m_numFailed++;
		}
		else
		{
			m_numPassed++;
		}

		LOGMSG (INFO, neo::makeString(outcome?"[PASSED] ":"[FAILED] ", testname));
	}

	void bigintTest::testPrimatives()
	{
		TRACE_FUNCTION();
	    
		mathprim::compound_u64 a = mathprim::LSL(0x1234, 24);
		verify ("LSL", a.u64_value == 0x0000001234000000LL);

		mathprim::compound_u64 b = mathprim::LSR(0xF2345432, 24);
		verify ("LSR", b.u64_value == 0x000000F234543200LL);

		mathprim::compound_u64 c = mathprim::ASR(0xF2345432, 24);
		verify ("ASR", c.u64_value == 0xFFFFFFF234543200LL);

		mathprim::u32 carry = 1;
		mathprim::u32 result = mathprim::addWithCarry(0x7fffffff, 0x80000000, carry);
		verify ("addWithCarry, overflow, carry", carry  == 1);
		verify ("addWithCarry, overflow, result", result == 0);

		carry = 0;
		result = mathprim::addWithCarry(0x7fffffff, 0x80000000, carry);
		verify ("addWithCarry, no overflow, carry", carry  == 0);
		verify ("addWithCarry, no overflow, result", result == 0xffffffff);

		mathprim::compound_u64 mulres = mathprim::mul32x32(0x123456, 0x1234);
		verify("mul32x32", mulres.u64_value == 0x14B60AD78LL);
	}

	void bigintTest::testInitialise()
	{
		TRACE_FUNCTION();

		if (true)
		{
			int128 a (int(0xffffffff));
			verify ("signed bigint, from i32", a.toHexString() == "ffffffffffffffffffffffffffffffff");

			int128 b (unsigned int(0xffffffff));
			verify ("signed bigint, from u32", b.toHexString() == "000000000000000000000000ffffffff");

			int128 c (__int64(0xffffffffffffffffLL));
			verify ("signed bigint, from i64", c.toHexString() == "ffffffffffffffffffffffffffffffff");

			int128 d (unsigned __int64(0xffffffffffffffffLL));
			verify ("signed bigint, from u64", d.toHexString() == "0000000000000000ffffffffffffffff");
		}

		if (true)
		{
			uint128 a (int(0xffffffff));
			verify ("unsigned bigint, from i32", a.toHexString() == "ffffffffffffffffffffffffffffffff");

			uint128 b (unsigned int(0xffffffff));
			verify ("unsigned bigint, from u32", b.toHexString() == "000000000000000000000000ffffffff");

			uint128 c (__int64(0xffffffffffffffffLL));
			verify ("unsigned bigint, from i64", c.toHexString() == "ffffffffffffffffffffffffffffffff");

			uint128 d (unsigned __int64(0xffffffffffffffffLL));
			verify ("unsigned bigint, from u64", d.toHexString() == "0000000000000000ffffffffffffffff");
		}


		int128 a (-1);
		int256 b = a.cast<int256>();
		verify ("int256 <- int128 (-1)", b == int256(-1));
		
		uint256 c = a.cast<uint256>();
		verify ("int256 <- int128 (-1)", c == uint256(-1));

		int128 d = int128::fromDecString("-1237612627387465253764");
		verify ("fromDecString -ve", d.toDecString() == "-1237612627387465253764");

		d = int128::fromDecString("1237612627387465253764");
		verify ("fromDecString +ve", d.toDecString() == "1237612627387465253764");

		d = int128::fromHexString("0x123761262738abcde746f5253764");
		verify ("fromHexString +ve", d.toHexString() == "0000123761262738abcde746f5253764");

		int128 e1 = int128::fromHexString("-0x123761262738abcde746f5253764");
		int128 e2 = -int128::fromHexString("0x123761262738abcde746f5253764");

		verify ("fromHexString -ve", e1 == e2);

	}

	void bigintTest::testCompare()
	{
		TRACE_FUNCTION();
		
		verify ("signed 1 > -1",  int128(1)  > int128(-1));
		verify ("signed -1 < 1",  int128(-1) <  1);
		verify ("signed 1 == 1",  int128(1)  == 1);
		verify ("signed 1 > 0",   int128(1)  >  0);
		verify ("signed 0 < 1",   int128(0)  <  1);
		verify ("signed -2 < -1", int128(-2) < -1);
		verify ("signed -1 > -2", int128(-1) > -2);
		

		verify ("unsigned 1 > -1",  int128::unsignedCompare(1, -1) < 0 );
		verify ("unsigned -1 < 1",  int128::unsignedCompare(-1, 1) > 0 );
		verify ("unsigned 1 == 1",  int128::unsignedCompare(1, 1) == 0 );
		verify ("unsigned 1 > 0",   int128::unsignedCompare(1, 0) > 0 );
		verify ("unsigned 0 < 1",   int128::unsignedCompare(0, 1) < 0 );
		verify ("unsigned -2 < -1", int128::unsignedCompare(-2, -1) < 0 );
		verify ("unsigned -1 > -2", int128::unsignedCompare(-1, -2) > 0 );
	}

	void bigintTest::testAddSub()
	{
		TRACE_FUNCTION();

		int128 a = -19234;
		verify ("signed abs (-19234)",  neo::abs(a).toInt() == 19234);

		uint128 b = -19234;
		verify ("unsigned abs (-19234)",  neo::abs(b).toUnsignedInt() == -19234);

	}

	void bigintTest::testShift()
	{
		TRACE_FUNCTION();

		int128 a (0x123456789);

		int128::shiftLeft32Bit (a, a);
		verify ("shiftLeft32Bit 1",  a.toHexString() == "00000000000000012345678900000000");

		int128::shiftLeft32Bit (a, a);
		verify ("shiftLeft32Bit 2",  a.toHexString() == "00000001234567890000000000000000");

		int128::shiftLeft32Bit (a, a);
		verify ("shiftLeft32Bit 3",  a.toHexString() == "23456789000000000000000000000000");

		int128::shiftLeft32Bit (a, a);
		verify ("shiftLeft32Bit 4",  a.toHexString() == "00000000000000000000000000000000");

		int128 b (0x123456789);
		int128::shiftLeft (b, 56, b);
		verify ("0x123456789 << 56",  b.toHexString() == "00000000012345678900000000000000");
														  
		int128 cs (0x123456789);
		int128::twosComplement(cs, cs);
		int128::shiftLeft32Bit (cs, cs);

		int128 cu = cs;

		int128::shiftRight32BitUnsigned(cu, cu);
		verify ("ffffffffffffffedcba987700000000 >> 32 unsigned", cu.toHexString() == "00000000fffffffffffffffedcba9877");

		int128::shiftRight32BitSigned(cs, cs);
		verify ("ffffffffffffffedcba987700000000 >> 32 signed", cs.toHexString() ==   "fffffffffffffffffffffffedcba9877");

		int128 d (0x123456789);
		int128::shiftLeft (d, 77, d);
		int128::shiftRightUnsigned (d, 77, d);

		verify ("unsigned (0x123456789 << 77) >> 77", int128::unsignedCompare (d, int128(0x123456789)) == 0);

		int128 e (0x123456);
		int128::twosComplement(e, e);

		int128 e_res = e;

		int128::shiftLeft (e, 77, e);
		int128::shiftRightSigned (e, 77, e);

		verify ("signed (-0x123456 << 77) >> 77", int128::unsignedCompare (e, e_res) == 0);

		int128 f (1);

		f <<= 40;
		verify ("indexMSB (f << 40)", f.indexMSB() == 40);

		f = 0;
		verify ("indexMSB (0)", f.indexMSB() == -1);

	}

	// http://world.std.com/~reinhold/BigNumCalc.html

	void bigintTest::testMul()
	{
		TRACE_FUNCTION();
		int128 result;

		verify ("mul: 2 * 2 = 4", (int128(2) * int128(2)) == 4);
		verify ("mul: 2 * -2 = -4", (int128(2) * int128(-2)) == -4);
		verify ("mul: -2 * -2 = 4", (int128(-2) * int128(-2)) == 4);

		int128::umul(0x123456789abcdef0UL, 0x937472435af38478UL, result);
		verify ("umul: 0x123456789abcdef0UL * 0x937472435af38478UL",  result.toHexString() == "0a7c557e882c9f916f7538c9e94c4080");

		// calculate 32!
		int128 factorial = 1;

		for (int n = 1; n <= 32; n++)
		{
			factorial = factorial * n;
		}

		verify ("32!",  factorial.toHexString() == "0032ad5a155c6748ac18b9a580000000");

	}

	void bigintTest::testDiv()
	{
		TRACE_FUNCTION();

		if (true)
		{
			int128 result (0x123456789abcdef0UL);

			for (int n = 0; n < 20; n++)
				int128::mulBy10(result, result);

			for (int n = 0; n < 20; n++)
				int128::udivBy10(result, result);

			verify ("udivBy10: 0x123456789abcdef0UL", result.toHexString() == "0000000000000000123456789abcdef0");
		}

		if (true)
		{
			int128 dividend (0x123456789abcdef0UL);

			dividend = (dividend << 64) + int128(0x123456789abcdef0UL);

			verify ("dividend: 128 bit number", dividend.toHexString() == "123456789abcdef0123456789abcdef0");
	                                                                       
			int128 quotient;
			int128 modulus;
			int128 divisor (0x8765);

			quotient = dividend / divisor;
			modulus  = dividend % divisor;
			verify ("0x123456789abcdef0123456789abcdef0 / 0x8765", quotient.toHexString() == "0000226ba133361b8a90e1be26c7b044");
			verify ("0x123456789abcdef0123456789abcdef0 % 0x8765", modulus.toDecString()  == "30748"); 
	                
			divisor = int128(0x20394856783);
			quotient = dividend / divisor;
			modulus  = dividend % divisor;
			verify ("0x123456789abcdef0123456789abcdef0 / 0x20394856783", quotient.toHexString() == "00000000000909fd02ba713734287825");
			verify ("0x123456789abcdef0123456789abcdef0 % 0x20394856783", modulus.toDecString()  == "351423398145");

			quotient = dividend / dividend;
			modulus  = dividend % dividend;
			verify ("0x123456789abcdef0123456789abcdef0 / 0x123456789abcdef0123456789abcdef0", quotient.toDecString() == "1");
			verify ("0x123456789abcdef0123456789abcdef0 % 0x123456789abcdef0123456789abcdef0", modulus.toDecString() == "0");

			bool divExcept = false;

			try
			{
				quotient = dividend / 0;
			}
			catch (std::invalid_argument&)
			{
				divExcept = true;
			}
			verify ("divide by zero exception", divExcept);

		}
	}



}

