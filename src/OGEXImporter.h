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
#include <OgreException.h>

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
	ODDLParser::DDLNode* mOGEXTreeRoot;

	// Helper methods for mesh loading

	void procGeometryObj(ODDLParser::DDLNode* geometryObj);
	void procMesh(ODDLParser::DDLNode* meshObj);
	void printVertexArr(ODDLParser::DataArrayList* vaList);
	Ogre::Vector3 getVertexFrom(ODDLParser::DataArrayList* vertexObj);
	ODDLParser::DataArrayList* getPosListFrom(ODDLParser::DDLNode* meshObj);
	ODDLParser::DataArrayList* getNormListFrom(ODDLParser::DDLNode* meshObj);
	ODDLParser::DataArrayList* getIndListFrom(ODDLParser::DDLNode* meshObj);

	/// Get a DDLNode that has "attrib" property set to value "atribValue" and is of type "nodeType"
	ODDLParser::DDLNode* getAttribNodeFrom(ODDLParser::DDLNode* parrentNode, const String& attribValue, const String& nodeType);

	/// Get the mesh node of the first GeometryObject named "geomObjName"
	/// Q: Ogre mesh name = OGEX GeomObj name ??????????????????????????????????????
	ODDLParser::DDLNode* findMeshNode(String geomObjName);

};

} /* namespace Ogre */

#endif /* OGEXIMPORTER_H_ */
