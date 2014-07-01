
#include "neo/Logging.h"

#include <limits>

#include "bigfixedTest.h"


//CREATE_LOGGING_CATEGORY (test);
USE_LOGGING_CATEGORY (test);

namespace neo
{

	bigfixedTest::bigfixedTest() : m_passed (true), m_numPassed(0), m_numFailed(0)
	{
	    
	}

	bool bigfixedTest::doTests()
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

	void bigfixedTest::verify (const std::string& testname, bool outcome)
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

	void bigfixedTest::testPrimatives()
	{
		TRACE_FUNCTION();
	}

	void bigfixedTest::testInitialise()
	{
		TRACE_FUNCTION();

		fixed_128_64 v;
		verify ("init default", v.toDecString() == "0.0");

		v = fixed_128_64(100);
		verify ("init 100", v.toDecString() == "100.0");
		
		v = fixed_128_64(-100);
		verify ("init -100", v.toDecString() == "-100.0");

		v = fixed_128_64(1.0);
		verify ("init 1.0", v.toDecString() == "1.0");

		v = fixed_128_64 (1000.0);
		verify ("init 1000.0", v.toDecString() == "1000.0");

		v = fixed_128_64 (1000.125);
		verify ("init 1000.125", v.toDecString() == "1000.125");

		v = fixed_128_64 (0.0001220703125);
		verify ("init 0.0001220703125", v.toDecString() == "0.0001220703125");

		v = fixed_128_64 (-0.00000762939453125);
		verify ("init -0.00000762939453125", v.toDecString() == "-0.00000762939453125");

		fixed_128_128 v2 = fixed_128_128 (std::numeric_limits<double>::denorm_min());
		verify ("init subnormal", v2.toDecString() == "0.0");

		v = fixed_128_64 (-2000.123456);
		double d = v.toDouble();
	}

	void bigfixedTest::testCompare()
	{
		verify ("compare: ==", fixed_128_64(123.1234) == fixed_128_64(123.1234));
		verify ("compare: !=", fixed_128_64(123.1234) != fixed_128_64(-123.1234));
		verify ("compare: > ", fixed_128_64(5.12345)  >  fixed_128_64(5.12341));
		verify ("compare: < ", fixed_128_64(-123.123) <  fixed_128_64(-123));
		verify ("compare: <=", fixed_128_64(987)      <= fixed_128_64(999));
		verify ("compare: <=", fixed_128_64(987)      <= fixed_128_64(987));
		verify ("compare: >=", fixed_128_64(999999)   >= fixed_128_64(98765));
		verify ("compare: >=", fixed_128_64(77)       >= fixed_128_64(77));

		verify ("compare: ! ==",  !(fixed_128_64(123.1233)  == fixed_128_64(123.1234)));
		verify ("compare: ! !=",  !(fixed_128_64(123.1234) != fixed_128_64(123.1234)));
		verify ("compare: ! > ",  !(fixed_128_64(5.12341)   >  fixed_128_64(5.12345) ));
		verify ("compare: ! < ",  !(fixed_128_64(-123)      <  fixed_128_64(-123.123)));
		verify ("compare: ! <=",  !(fixed_128_64(999)       <= fixed_128_64(987)     ));
		verify ("compare: ! >=",  !(fixed_128_64(98765)     >= fixed_128_64(999999)  ));
	}

	void bigfixedTest::testAddSub()
	{
		TRACE_FUNCTION();
	}

	void bigfixedTest::testShift()
	{
	}

	// http://world.std.com/~reinhold/BigNumCalc.html

	void bigfixedTest::testMul()
	{
		TRACE_FUNCTION();
	}

	void bigfixedTest::testDiv()
	{
		TRACE_FUNCTION();
	}

}
