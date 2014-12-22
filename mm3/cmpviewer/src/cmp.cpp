#include "cmp.h"

#include <sstream>
#include <stdexcept>

using namespace cmp;

Element::Element(Version version)
{
	switch (version) {
		case Version109:
		case Version114:
		case Version115:
			break;
		default:
			std::ostringstream msg;
			msg << "Unknown CMP version. Expected one of " << Version109 << "/" << Version114 << "/" << Version115 << ", got " << version << ".";
			throw std::runtime_error(msg.str());
	}

	this->version = version;
}

Node::Node(Version version, Type type) : Element(version)
{
	switch (type) {
		case Root:
		case Transform:
		case Mesh1:
		case Axis:
		case Light:
		case Smoke:
		case Mesh2:
			break;
		default:
			std::ostringstream msg;
			msg << "Unknown node type. Expected " << Root << " - " << Mesh2 << ", got " << type << ".";
			throw std::runtime_error(msg.str());
	}

	this->type = type;
}

Node::~Node()
{
}

void Node::read(std::ifstream& ifs)
{
	parse(ifs, name);
}

Node* Node::readNode(std::ifstream& ifs, Version version)
{
	Node* node;

	Type type;
	parse(ifs, type);

	switch (type) {
		case Root:
			throw std::runtime_error("Unexpected root node. Use RootNode::readFile() to parse file from the beginning.");
		case Transform:
			node = new TransformNode(version);
			break;
		case Mesh1:
		case Mesh2:
			node = new MeshNode(version, type);
			break;
		case Axis:
			node = new AxisNode(version);
			break;
		case Light:
			node = new LightNode(version);
			break;
		case Smoke:
			node = new SmokeNode(version);
			break;
		default:
			std::ostringstream msg;
			msg << "Unknown node type. Expected " << Transform << " - " << Mesh2 << ", got " << type << ".";
			throw std::runtime_error(msg.str());
	}

	node->read(ifs);

	return node;
}

GroupNode::~GroupNode()
{
	for (Node* child : children) {
		delete child;
	}
}

void GroupNode::read(std::ifstream& ifs)
{
	uint32_t childCount;
	parse(ifs, childCount);

	for (uint32_t i = 0; i < childCount; i++) {
		children.push_back(readNode(ifs, version));
	}
}

void GroupNode::findMeshes(MeshList* meshList)
{
	for (Node* child : children) {
		child->findMeshes(meshList);
	}
}

RootNode::RootNode(Version version) : GroupNode(version, Root)
{
	rootEntries = 0;
}

RootNode::~RootNode()
{
	if (rootEntries) {
		delete[] rootEntries;
	}
}

RootNode* RootNode::readFile(std::ifstream& ifs)
{
	Type type;
	parse(ifs, type);

	if (type != Root) {
		std::ostringstream msg;
		msg << "Unexpected root node type. Expected " << Root << ", got " << type << ".";
		throw std::runtime_error(msg.str());
	}

	Version version;
	parse(ifs, version);

	RootNode* root = new RootNode(version);

	root->read(ifs);

	root->resolveReferences();

	return root;
}

void RootNode::read(std::ifstream& ifs)
{
	Node::read(ifs);

	parse(ifs, unknown0);
	parse(ifs, aabb);
	parse(ifs, unknown1);
	parse(ifs, unknown2);
	parse(ifs, unknown3);

	if (version >= Version114) {
		parse(ifs, path);
	}

	parse(ifs, transformation);
	parse(ifs, unknown4);
	parse(ifs, unknown5);
	parse(ifs, aabb2);
	parse(ifs, rootEntryCount);
	parse(ifs, unknown6);
	parse(ifs, unknown7);
	parse(ifs, unknown8);
	parse(ifs, unknown9);

	rootEntries = new RootEntry[rootEntryCount];
	ifs.read(reinterpret_cast<char*>(rootEntries), sizeof(RootEntry) * rootEntryCount);

	parse(ifs, meshNodeCount);

	GroupNode::read(ifs);
}

void RootNode::resolveReferences()
{
	std::vector<Mesh*> meshes;
	findMeshes(&meshes);

	for (Mesh* empty : meshes) {
		if (empty->length == 0) {
			for (Mesh* mesh : meshes) {
				if (mesh != empty && mesh->length > 0 && mesh->name == empty->name) {
					empty->reference = mesh;
					break;
				}
			}
		}
	}
}

void TransformNode::read(std::ifstream& ifs)
{
	Node::read(ifs);

	parse(ifs, transformation);
	parse(ifs, meshIndex);
	parse(ifs, aabb);

	GroupNode::read(ifs);
}

void AxisNode::read(std::ifstream& ifs)
{
	Node::read(ifs);
}

void LightNode::read(std::ifstream& ifs)
{
	Node::read(ifs);

	parse(ifs, unknown0);
	parse(ifs, unknown1);
	parse(ifs, unknown2);

	if (version >= Version114) {
		parse(ifs, unknown3);
	}
}

void SmokeNode::read(std::ifstream& ifs)
{
	Node::read(ifs);
	parse(ifs, unknown0);
}

Mesh::Mesh(Version version) : Element(version)
{
	indices = 0;
	vertices = 0;
	numberPlateVertices = 0;
	reference = 0;
}

Mesh::~Mesh()
{
	if (indices) {
		delete[] indices;
	}

	if (vertices) {
		delete[] vertices;
	}

	for (Primitive* primitive : primitives) {
		delete primitive;
	}

	for (Material* material : materials) {
		delete material;
	}

	if (numberPlateVertices) {
		delete numberPlateVertices;
	}
}

void Mesh::read(std::ifstream& ifs)
{
	parse(ifs, name);
	parse(ifs, length);

	if (!length) {
		return;
	}

	int meshStartOffset = (int)ifs.tellg();

	parse(ifs, unknown0);
	parse(ifs, aabb);
	parse(ifs, vertexCount1);
	parse(ifs, indexCount);
	parse(ifs, unknown1);
	parse(ifs, unknown2);
	parse(ifs, unknown3);

	if (version >= Version115) {
		parse(ifs, color);
	}
	else {
		parse(ifs, color.a);
	}

	parse(ifs, path);

	parse(ifs, hasIndices);
	if (hasIndices) {
		parse(ifs, unknown4);
		parse(ifs, indicesLength);

		indices = new uint16_t[indexCount];
		ifs.read(reinterpret_cast<char*>(indices), indicesLength);
	}

	parse(ifs, unknown5);
	parse(ifs, unknown6);
	parse(ifs, unknown7);

	parse(ifs, vertexCount2);

	if (vertexCount1 != vertexCount2) {
		std::ostringstream msg;
		msg << "vertexCount1 (" << vertexCount1 << ") != vertexCount2 (" << vertexCount2 << ") in mesh \"" << name << "\".";
		throw std::runtime_error(msg.str());
	}

	parse(ifs, vertexStride);

	if (vertexStride != sizeof(Vertex)) {
		std::ostringstream msg;
		msg << "vertexStride (" << vertexStride << ") != sizeof(Vertex) (" << sizeof(Vertex) << ") in mesh \"" << name << "\".";
		throw std::runtime_error(msg.str());
	}

	parse(ifs, verticesLength);
	parse(ifs, unknown8);

	vertices = new Vertex[vertexCount2];
	ifs.read(reinterpret_cast<char*>(vertices), verticesLength);

	parse(ifs, primitiveAndMaterialCount);

	for (int i = 0; i < primitiveAndMaterialCount / 2; i++) {
		Primitive::Type type;
		parse(ifs, type);

		Primitive* primitive = 0;

		switch (type) {
			case Primitive::TriangleList:
			{
				TriangleList* list = new TriangleList();
				primitive = list;
				parse(ifs, list->minIndex);
				parse(ifs, list->vertexCount);
				break;
			}
			case Primitive::TriangleStrip:
			{
				TriangleStrip* strip = new TriangleStrip();
				primitive = strip;
				break;
			}
			default:
				std::ostringstream msg;
				msg << "Unknown primitive type " << type << " in mesh \"" << name << "\".";
				throw std::runtime_error(msg.str());
		}

		parse(ifs, primitive->offset);
		parse(ifs, primitive->count);
		parse(ifs, primitive->unknown);
		primitives.push_back(primitive);
	}

	parse(ifs, materialCount);

	for (int i = 0; i < materialCount; i++) {
		Material* material = new Material();
		parse(ifs, material->minIndex);
		parse(ifs, material->vertexCount);
		parse(ifs, material->offset);
		parse(ifs, material->count);
		parse(ifs, material->isTriangleStrip);
		parse(ifs, material->material);
		materials.push_back(material);
	}

	parse(ifs, hasNumberPlate);

	if (hasNumberPlate) {
		parse(ifs, numberPlateVertexCount);
		numberPlateVertices = new NumberPlateVertex[numberPlateVertexCount];
		ifs.read(reinterpret_cast<char*>(numberPlateVertices), sizeof(NumberPlateVertex) * numberPlateVertexCount);
	}

	int unparsedLength = length - ((int)ifs.tellg() - meshStartOffset);

	if (unparsedLength < 0) {
		std::ostringstream msg;
		msg << "Read " << -unparsedLength << " bytes past expected length of " << length << " bytes in mesh \"" << name << "\".";
		throw std::runtime_error(msg.str());
	}
	else if (unparsedLength > 0) {
		std::ostringstream msg;
		msg << "Finished reading mesh \"" << name << "\" with " << unparsedLength << " unparsed bytes left.";
		throw std::runtime_error(msg.str());
	}
}

MeshNode::~MeshNode()
{
	for (Mesh* mesh : meshes) {
		delete mesh;
	}
}

void MeshNode::findMeshes(MeshList* meshList)
{
	for (cmp::Mesh* mesh : meshes) {
		meshList->push_back(mesh);
	}
}

void MeshNode::read(std::ifstream& ifs)
{
	Node::read(ifs);

	parse(ifs, unknown0);
	parse(ifs, unknown1);
	parse(ifs, unknown2);
	parse(ifs, unknown3);

	int maxMeshes = 2;
	if (type == Mesh2) {
		maxMeshes = 3;
		parse(ifs, aabb);
	}

	for (int i = 0; i < maxMeshes; i++) {
		uint8_t meshFollows;
		parse(ifs, meshFollows);

		if (meshFollows) {
			Mesh* mesh = new Mesh(version);
			mesh->read(ifs);
			meshes.push_back(mesh);
		}
		else {
			break;
		}
	}
}
