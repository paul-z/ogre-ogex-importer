/*
 * OGEXImporter.h
 *
 *  Created on: Mar 23, 2016
 *      Author: paul
 */

#ifndef OGEXIMPORTER_H_
#define OGEXIMPORTER_H_

#include <OgreMesh2.h>
#include <Vao/OgreVaoManager.h>

#include <openddlparser/OpenDDLParser.h>

namespace Ogre {

class OGEXImporter {

public:
	/**
	 * @brief OGEXImporter constructor
	 * @param fileName	[in] name of *.ogex file
	 */
	OGEXImporter(const String& fileName);
	virtual ~OGEXImporter();

	/**
	 * Parse the current ogex file
	 */
	void parseFile();

	/**
	 * @brief Load a mesh named meshName from the current OpenGEX file
	 * @param meshName	[in] meshName - the name of the mesh to import
	 * @return a pointer to a v2 Mesh
	 */
	MeshPtr loadMesh(const String& meshName);

private:
	String mFileName;

	// Helper methods for mesh loading
	void procGeometryObj(ODDLParser::DDLNode* geometryObj);
	void procMesh(ODDLParser::DDLNode* meshObj);
	void procVertexArr(ODDLParser::DDLNode* vertexArrObj);
	void procVertex(ODDLParser::DataArrayList* vertexObj);
};

} /* namespace Ogre */

#endif /* OGEXIMPORTER_H_ */
