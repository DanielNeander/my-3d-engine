#pragma once 


#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

class hkTest : public CppUnit::TestFixture {
public:
		
	CPPUNIT_TEST_SUITE(hkTest);
	CPPUNIT_TEST(loadAsset);
	CPPUNIT_TEST_SUITE_END();

public:
	void setup() {}
	void tearDown() {}	

private:
	void loadAsset();

	

};

