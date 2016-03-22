//============================================================================
// Name        : ogex-importer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cassert>
#include <openddlparser/OpenDDLParser.h>

USE_ODDLPARSER_NS;
using namespace std;

const char* kFilePath = "/home/paul/work/ogre-opengex-module/default_cube.ogex";

void procVertex(DataArrayList* vertexObj) {
	Value* value = vertexObj->m_dataList;

	float x = value->getFloat();
	value = value->getNext();

	float y = value->getFloat();
	value = value->getNext();

	float z = value->getFloat();
	value = value->getNext();

	cout<<"X = "<<x<<" ";
	cout<<"Y = "<<y<<" ";
	cout<<"Z = "<<z<<" "<<endl;
}

void procVertexArr(DDLNode* vertexArrObj) {
	cout<<"vaObj type:"<<vertexArrObj->getType()<<endl;
	if(vertexArrObj->getType() != "VertexArray") { // precondition
		cerr<<"DDLNode must be of type: VertexArray !"<<endl;
		return;
	}

	DataArrayList* dataArr = vertexArrObj->getDataArrayList();
	cout<<"data arr list size: "<<dataArr->size()<<endl;

	while(dataArr) {
		procVertex(dataArr);
		dataArr = dataArr->m_next;
	}
	DDLNode* float3Node = vertexArrObj->getChildNodeList()[0];
	cout<<"procVertexArr: f3nodeType: "<<float3Node->getType();
}

void procMesh(DDLNode* meshObj) {
	if(meshObj->getType() != "Mesh") { // precondition
			cerr<<"DDLNode must be of type: Mesh !"<<endl;
	}

	DDLNode::DllNodeList childs = meshObj->getChildNodeList();
	for(int i = 0; i < childs.size(); i++) {
		DDLNode* childNode = childs[i];

		if(childNode->getType() == "VertexArray") {
			// get type: normal/position
			cout<<"Vertex array (type = TODO) :"<<endl;
			procVertexArr(childNode);
			break;

		}
		else if(childNode->getType() == "IndexArray / Submesh") {
			cout<<"Index array (type = TODO) : todo"<<endl;
		}
		else {
			cout<<"todo Mesh/type "<<endl;
		}
	}
}

void procGeometryObj(DDLNode* geometryObj) {
	if(geometryObj->getType() != "GeometryObject") { // precondition
		cerr<<"DDLNode must be of type: GeometryObject !"<<endl;
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

int test() {
    char filename[100];
    strcpy(filename, kFilePath);

    std::cout << "file to import: " << filename << std::endl;
    /* if( filename ) {
        std::cerr << "Invalid filename." << std::endl;
        return -1;
    } */

    FILE *fileStream = fopen( filename, "r+" );
    if( NULL == filename ) {
        std::cerr << "Cannot open file " << filename << std::endl;
        return 1;
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
            std::cerr << "Error while parsing file " << filename << "." << std::endl;
        }

        DDLNode *root = theParser.getRoot();
        DDLNode::DllNodeList childs = root->getChildNodeList();
        for ( size_t i=0; i<childs.size(); i++ ) {
        	DDLNode *child = childs[ i ];

        	cout<<"child["<<i<<"]: "<<child->getName();
        	std::string type = child->getType();     // to get the node type
        	cout<<" of type: "<<type<<endl;
        	if(type == "GeometryObject")
        		procGeometryObj(child);

        	Property *prop   = child->getProperties(); // to get properties
        	Value *values    = child->getValue();    // to get the data;


        }

    }
    return 0;
}

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	test();
	return 0;
}
