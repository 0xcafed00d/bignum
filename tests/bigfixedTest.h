#pragma once

#include <string>

#include "bigfixed.h"

namespace neo
{
	typedef bigfixed<4, 2> fixed_128_64;
	typedef bigfixed<4, 4> fixed_128_128;

	class bigfixedTest
	{
		private:
			bool m_passed;
			int m_numPassed;
			int m_numFailed;

			void verify (const std::string& testname, bool outcome); 

			void testPrimatives ();
			void testInitialise ();
			void testCompare ();
			void testAddSub ();
			void testShift ();
			void testMul ();
			void testDiv ();

		public:
			bigfixedTest ();

			bool doTests();
	};
}



