/*
 * OGEXImporter.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: paul
 */

#include "OGEXImporter.h"
#include <iostream>
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreMeshManager2.h>
#include <OgreSubMesh2.h>
#include <algorithm>
#include <limits>

USE_ODDLPARSER_NS;
using namespace std;

namespace Ogre {

OGEXImporter::OGEXImporter(const String& fileName) {
	// TODO Auto-generated constructor stub
	mFileName = fileName;
}

OGEXImporter::~OGEXImporter() {
	// TODO Auto-generated destructor stub
}

// inspired by url: http://www.ogre3d.org/forums/viewtopic.php?f=25&t=83999
// TODO:
// * take into account the up vector - implement converttoYup
MeshPtr OGEXImporter::loadMesh(const String& meshName) {
	cout<<"Loading mesh: "<<meshName<<endl;

    // Create the mesh:
    Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(meshName,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::SubMesh* subMesh = mesh->createSubMesh();

    cout<<"Done1 "<<endl;

    Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

    Ogre::VertexElement2Vec vertexElements;
    vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
    vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
    //    //uvs
    //    for(int i=0; i<currentBuffer->uvSetCount; i++){
    //        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));
    //    }

    cout<<"Done init"<<endl;

    ODDLParser::DDLNode* meshObj = findMeshNode(meshName);

    ODDLParser::DataArrayList* posList = getPosListFrom(meshObj);
    ODDLParser::DataArrayList* normList = getNormListFrom(meshObj);


    int vertexCount = std::min(posList->size(), normList->size()) + 1; // DDL size() returns last valid index, size is +1

    size_t vertexSize = vaoManager->calculateVertexSize(vertexElements);

    Ogre::Real* vertexData = static_cast<Ogre::Real*>( OGRE_MALLOC_SIMD( vertexSize * vertexCount, Ogre::MEMCATEGORY_GEOMETRY ) );
    Ogre::Real* pVertex = reinterpret_cast<Ogre::Real*>(vertexData);


    Ogre::Vector3 minBB(Ogre::Vector3::UNIT_SCALE * numeric_limits<float>::max());
    Ogre::Vector3 maxBB(Ogre::Vector3::UNIT_SCALE * numeric_limits<float>::min());

    while((posList != ddl_nullptr) && (normList != ddl_nullptr))
    {
        //Ogre::Vector3 pos = convertToYup(Ogre::Vector3(currentBuffer->vertexs.at(i).co[0],currentBuffer->vertexs.at(i).co[1],currentBuffer->vertexs.at(i).co[2]));
        //transform to Y-up

    	Ogre::Vector3 pos = getVertexFrom(posList);
        *pVertex++ = pos.x;
        *pVertex++ = pos.y;
        *pVertex++ = pos.z;

        //Ogre::Vector3 norm = convertToYup(Ogre::Vector3(currentBuffer->vertexs.at(i).no[0],currentBuffer->vertexs.at(i).no[1],currentBuffer->vertexs.at(i).no[2])).normalisedCopy();
        //Normals

        Ogre::Vector3 norm = getVertexFrom(normList);
        *pVertex++ = norm.x;
        *pVertex++ = norm.y;
        *pVertex++ = norm.z;

        //        //uvs
        //        for(int j=0; j<currentBuffer->uvSetCount; j++){
        //            *pVertex++ = currentBuffer->vertexs.at(i).uv[j].x;
        //            *pVertex++ = 1.0-currentBuffer->vertexs.at(i).uv[j].y;
        //        }

        //Calc Bounds
        minBB.makeFloor(pos);
        maxBB.makeCeil(pos);

        // move forward
        posList = posList->m_next;
        normList = normList->m_next;
    }

    Ogre::VertexBufferPackedVec vertexBuffers;

    Ogre::VertexBufferPacked *pVertexBuffer = vaoManager->createVertexBuffer( vertexElements, vertexCount, Ogre::BT_IMMUTABLE, vertexData, true );
    vertexBuffers.push_back(pVertexBuffer);

    cout<<"Done vertex arrays."<<endl;

    //Indices
    ODDLParser::DataArrayList* indList = getIndListFrom(meshObj);
    //unsigned int iBufSize = currentBuffer->triangles.size() * 3;
    uint32 triangleCount = indList->size() + 1; // DDL size() returns last valid index, size is +1
    uint32 iBufSize = triangleCount * 3;

    static const unsigned short index16BitClamp = (0xFFFF) - 1;

    //Index buffer
    Ogre::IndexBufferPacked::IndexType buff_type = (iBufSize > index16BitClamp) ?
                Ogre::IndexBufferPacked::IT_32BIT : Ogre::IndexBufferPacked::IT_16BIT;

    //Build index items
    bool using32 = buff_type == Ogre::IndexBufferPacked::IT_32BIT;

    Ogre::uint32 *indices32 = 0;
    Ogre::uint16 *indices16 = 0;

    if (!using32)
        indices16 = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * iBufSize, Ogre::MEMCATEGORY_GEOMETRY ) );
    else
        indices32 = reinterpret_cast<Ogre::uint32*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint32) * iBufSize, Ogre::MEMCATEGORY_GEOMETRY ) );


    for (uint32 cur = 0; cur < triangleCount; cur++)
    {
    	Value* dataArr = indList->m_dataList;
        for(unsigned int i=0; i<3; i++){
            if(using32) {
            	uint32 ind = (Ogre::uint32)dataArr->getUnsignedInt32();
                *indices32++ = ind;
            }
            else {
            	uint16 ind = (Ogre::uint16)dataArr->getUnsignedInt16();
                *indices16++ = ind;
            }
            dataArr = dataArr->getNext();
        }
        indList = indList->m_next;
    }

    Ogre::IndexBufferPacked *indexBuffer;
    if(using32){
        indexBuffer = vaoManager->createIndexBuffer( buff_type, iBufSize, Ogre::BT_IMMUTABLE, indices32, true );
    }
    else{
        indexBuffer = vaoManager->createIndexBuffer( buff_type, iBufSize, Ogre::BT_IMMUTABLE, indices16, true );
    }


    Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(
                vertexBuffers, indexBuffer, Ogre::v1::RenderOperation::OT_TRIANGLE_LIST );

    subMesh->mVao[0].push_back( vao );
    subMesh->mVao[1].push_back( vao );


    Ogre::Aabb bounds;
    bounds.merge(minBB);
    bounds.merge(maxBB);
    mesh->_setBounds(bounds,false);
    mesh->_setBoundingSphereRadius(bounds.getRadius());

    cout<<"Done."<<endl;

    return mesh;

}

void OGEXImporter::parseFile() {

    std::cout << "file to import: " << mFileName << std::endl;
    /* if( filename ) {
        std::cerr << "Invalid filename." << std::endl;
        return -1;
    } */

    FILE *fileStream = fopen( mFileName.c_str(), "r+" );
    if( NULL == fileStream ) {
        std::cerr << "Cannot open file " << mFileName << std::endl;
        return;
    }

    // obtain file size:
    fseek( fileStream, 0, SEEK_END );
    const size_t size( ftell( fileStream ) );
    rewind( fileStream );
    if( size > 0 ) {
        char *buffer = new char[ size ];
        const size_t readSize( fread( buffer, sizeof( char ), size, fileStream ) );
        assert( readSize == size );
        OpenDDLParser theParser;
        theParser.setBuffer( buffer, size );
        const bool result( theParser.parse() );
        if( !result ) {
            std::cerr << "Error while parsing file " << mFileName << "." << std::endl;
        }

        DDLNode *root = theParser.getRoot();
        mOGEXTreeRoot = root;

        DDLNode::DllNodeList childs = root->getChildNodeList();
        for ( size_t i=0; i<childs.size(); i++ ) {
        	DDLNode *child = childs[ i ];

        	cout<<"child["<<i<<"]: "<<child->getName();
        	std::string type = child->getType();     // to get the node type
        	cout<<" of type: "<<type<<endl;
        	if(type == "GeometryObject")
        		procGeometryObj(child);

        	//Property *prop   = child->getProperties(); // to get properties
        	//Value *values    = child->getValue();    // to get the data;


        }

    }
    return;
}

Vector3 OGEXImporter::getVertexFrom(DataArrayList* vertexObj) {
	Value* value = vertexObj->m_dataList;

	Real x = value->getFloat();
	value = value->getNext();

	Real y = value->getFloat();
	value = value->getNext();

	Real z = value->getFloat();
	value = value->getNext();

	return Vector3(x,y,z);
}

void OGEXImporter::printVertexArr(ODDLParser::DataArrayList* vaList) {
	DataArrayList* dataArr = vaList;
	cout<<"data arr list size: "<<dataArr->size()<<endl;

	while(dataArr) {
		Vector3 v = getVertexFrom(dataArr);
		cout<<"(x,y,z) = ("<<v.x<<","<<v.y<<","<<v.z<<") "<<endl;
		dataArr = dataArr->m_next;
	}
}

void OGEXImporter::procMesh(DDLNode* meshObj) {
	if(meshObj->getType() != "Mesh") { // precondition
		// throw Exception(Exception::ERR_INVALIDPARAMS, "DDLNode must be of type: Mesh !", "OGEXImporter");
	}

	ODDLParser::DataArrayList* posList = getPosListFrom(meshObj);
	ODDLParser::DataArrayList* normList = getNormListFrom(meshObj);

	cout<<"Positions:"<<endl;
	printVertexArr(posList);
	cout<<endl;
	cout<<"Normals:"<<endl;
	printVertexArr(normList);
	cout<<endl;
}

void OGEXImporter::procGeometryObj(DDLNode* geometryObj) {
	if(geometryObj->getType() != "GeometryObject") { // precondition
		//throw Exception(Exception::ERR_INVALIDPARAMS, "DDLNode must be of type: GeometryObject !", "OGEXImporter");
	}

	DDLNode::DllNodeList childs = geometryObj->getChildNodeList();
	DDLNode* meshChild = nullptr;
	if(childs.size() != 1 ||
	   childs[0]->getType() != "Mesh" ) {
		cout<<"Geometry Obj should have 1 Mesh child ?";
		return;
	}
	else
		meshChild = childs[0];

	procMesh(meshChild);
}

ODDLParser::DataArrayList* OGEXImporter::getPosListFrom(ODDLParser::DDLNode* meshObj) {
	if(meshObj->getType() != "Mesh") { // precondition
		//throw Exception(Exception::ERR_INVALIDPARAMS, "DDLNode must be of type: Mesh !", "OGEXImporter");
	}

	DDLNode::DllNodeList childs = meshObj->getChildNodeList();
	ODDLParser::DataArrayList* daList = nullptr;

	ODDLParser::DDLNode* vaPosNode = getAttribNodeFrom(meshObj, "position", "VertexArray");
	if(vaPosNode != ddl_nullptr)
		daList = vaPosNode->getDataArrayList();

	return daList;
}

ODDLParser::DataArrayList* OGEXImporter::getNormListFrom(ODDLParser::DDLNode* meshObj) {
	if(meshObj->getType() != "Mesh") { // precondition
		//throw Exception(Exception::ERR_INVALIDPARAMS, "DDLNode must be of type: Mesh !", "OGEXImporter");
	}

	DDLNode::DllNodeList childs = meshObj->getChildNodeList();
	ODDLParser::DataArrayList* daList = nullptr;

	ODDLParser::DDLNode* vaNormNode = getAttribNodeFrom(meshObj, "normal", "VertexArray");
	if(vaNormNode != ddl_nullptr)
		daList = vaNormNode->getDataArrayList();

	return daList;
}

ODDLParser::DataArrayList* OGEXImporter::getIndListFrom(ODDLParser::DDLNode* meshObj) {
	if(meshObj->getType() != "Mesh") { // precondition
		//throw Exception(Exception::ERR_INVALIDPARAMS, "DDLNode must be of type: Mesh !", "OGEXImporter");
	}

	DDLNode::DllNodeList childs = meshObj->getChildNodeList();
	ODDLParser::DataArrayList* daList = nullptr;

	for(uint32 i = 0; i < childs.size(); i++) {
		DDLNode* childNode = childs[i];
		if(childNode->getType() == "IndexArray"){ // must be "nodeType"
			daList = childNode->getDataArrayList();
			break;
		}
	}
	return daList;
}

ODDLParser::DDLNode* OGEXImporter::getAttribNodeFrom(ODDLParser::DDLNode* parrentNode,
		const String& attribValue, const String& nodeType) {

	DDLNode::DllNodeList childs = parrentNode->getChildNodeList();
	for(uint32 i = 0; i < childs.size(); i++) {
		DDLNode* childNode = childs[i];
		if(childNode->getType() != nodeType) // must be "nodeType"
			continue;

		ODDLParser::Property* attribProp = childNode->findPropertyByName("attrib");
		if(attribProp == ddl_nullptr) // must have attrib
			continue;

		String attrVal( attribProp->m_value->getString() );
		if(attrVal == attribValue) { // attrib = "attribValue"
			return childNode;
			break;
		}

	}
	return ddl_nullptr;
}

ODDLParser::DDLNode* OGEXImporter::findMeshNode(String geomObjName) {
	DDLNode::DllNodeList childs = mOGEXTreeRoot->getChildNodeList();
	for(uint32 i = 0; i < childs.size(); i++) {
		DDLNode* childNode = childs[i];
		if( (childNode->getType() == "GeometryObject") && (childNode->getName() == geomObjName))
			return childNode->getChildNodeList()[0]; // first child of GeomObj should be a MeshObj
	}
	return ddl_nullptr;
}

} /* namespace Ogre */
