#pragma once 


#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

class hkgTest : public CppUnit::TestFixture {
public:
	void setup() {}
	void tearDown() {}
	
	
	CPPUNIT_TEST_SUITE(hkgTest);
	CPPUNIT_TEST(TestCamera);	
	CPPUNIT_TEST_SUITE_END();
	
	void TestCamera();
	
private:
	void setupDefaultCamera();

	void init(); 

	void createVB();
	void createIB();
	void setTexture();
	void loadSkinMesh();
	void loadSceneData();
	void createLight();
	void createDisplayWorld();
	void createMesh();
};
