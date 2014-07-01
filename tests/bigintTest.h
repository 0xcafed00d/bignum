#pragma once

#include <string>

#include "bigint.h"

namespace neo
{
	class bigintTest
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
			bigintTest ();

			bool doTests();
	       
	};
}
