//============================================================================
// Name        : ogex-importer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cassert>
#include <OGEXImporter.h>

USE_ODDLPARSER_NS;
using namespace std;

const char* kFilePath = "/home/paul/work/ogre-opengex-module/default_cube.ogex";

int main() {
	cout << "Testing ogex importer: " << endl; // prints !!!Hello World!!!
	Ogre::OGEXImporter* imp = new Ogre::OGEXImporter(kFilePath);

	imp->parseFile();

	return 0;
}
