#include "xbc.h"

#include <sstream>

using namespace xbc;

const char*    xbc::KnownVersion       = "1.53";
const unsigned xbc::RoadTextureCount   = 5;
const unsigned xbc::FacadeTextureCount = 6;
const unsigned xbc::SeasonTextureCount = 12;

void MeshSection::read(std::ifstream& ifs)
{
	parse(ifs, unknown0);
	parse(ifs, meshId);
	parse(ifs, type);
	parse(ifs, minIndex);
	parse(ifs, vertexCount);
	parse(ifs, offset);
	parse(ifs, count);
	parse(ifs, unknown1);
	parse(ifs, unknown2);
	parse(ifs, objectOffset);
	parse(ifs, objectCount);
	parse(ifs, indexInObjectsUnknown0);
	parse(ifs, entriesInObjectsUnknown0);
	parse(ifs, aabb1);
	parse(ifs, aabb2);
	parse(ifs, unknown3);

	// Additional data for facades.
	if (unknown3 > 0x100) {
		parse(ifs, unknown4);
	}
}

void TextureHeader::read(std::ifstream& ifs)
{
	parse(ifs, dataLength);
	parse(ifs, name);
	parse(ifs, type);
	parse(ifs, unknown);
	parse(ifs, width);
	parse(ifs, height);
	parse(ifs, stride);
	parse(ifs, mips);
	parse(ifs, format);
}

Texture::Texture()
{
	mainData = 0;
	maskData = 0;
}

Texture::~Texture()
{
	if (mainData) {
		delete[] mainData;
	}

	if (maskData) {
		delete[] maskData;
	}
}

void Texture::read(std::ifstream& ifs)
{
	TextureHeader::read(ifs);

	char* tmp = new char[dataLength];
	ifs.read(tmp, dataLength);

	if (isInterleaved()) {
		mainData = new char[actualDataLength()];
		maskData = new char[actualDataLength()];

		// De-interleave 8-byte blocks.
		for (unsigned i = 0; i < dataLength / 16; i++) {
			((uint64_t*)maskData)[i] = ((uint64_t*)tmp)[i * 2];
			((uint64_t*)mainData)[i] = ((uint64_t*)tmp)[i * 2 + 1];
		}
	}
	else {
		mainData = tmp;
	}
}

void ProcessedTexture::read(std::ifstream& ifs)
{
	TextureHeader::read(ifs);

	texture.read(ifs);
}

template <typename VertexType>
Mesh<VertexType>::Mesh()
{
	vertices = 0;
	indices = 0;
}

template <typename VertexType>
Mesh<VertexType>::~Mesh()
{
	if (vertices) {
		delete[] vertices;
	}

	if (indices) {
		delete[] indices;
	}
}

template <typename VertexType>
void Mesh<VertexType>::read(std::ifstream& ifs)
{
	parse(ifs, vertexCount);
	parse(ifs, indexCount);

	vertices = new VertexType[vertexCount];
	ifs.read(reinterpret_cast<char*>(vertices), sizeof(VertexType) * vertexCount);

	indices = new uint16_t[indexCount];
	ifs.read(reinterpret_cast<char*>(indices), sizeof(uint16_t) * indexCount);
}

TreeBase::TreeBase()
{
	unknown = 0;
}

TreeBase::~TreeBase()
{
	if (unknown) {
		delete[] unknown;
	}
}

void TreeBase::read(std::ifstream& ifs)
{
	parse(ifs, name);
	parse(ifs, unknownCount);
	unknown = new uint16_t[unknownCount];
	ifs.read(reinterpret_cast<char*>(unknown), sizeof(uint16_t) * unknownCount);
}

TreeMesh::TreeMesh()
{
	vertices1 = 0;
	vertices2 = 0;
	indices = 0;
}

TreeMesh::~TreeMesh()
{
	if (vertices1) {
		delete[] vertices1;
	}

	if (vertices2) {
		delete[] vertices2;
	}

	if (indices) {
		delete[] indices;
	}
}

void TreeMesh::read(std::ifstream& ifs)
{
	parse(ifs, vertex1Count);
	vertices1 = new TreeVertex1[vertex1Count];
	ifs.read(reinterpret_cast<char*>(vertices1), sizeof(TreeVertex1) * vertex1Count);

	parse(ifs, vertex2Count);
	vertices2 = new TreeVertex2[vertex2Count];
	ifs.read(reinterpret_cast<char*>(vertices2), sizeof(TreeVertex2) * vertex2Count);

	parse(ifs, indexCount);
	indices = new uint16_t[indexCount];
	ifs.read(reinterpret_cast<char*>(indices), sizeof(uint16_t) * indexCount);
}

Xbc::Xbc()
{
	unknownPerCell = 0;
	subfilesPerCell = 0;
	matrices = 0;
	roads.meshes = 0;
	roads.meshSections = 0;
	roads.objectIndices = 0;
	roads.objectPositions = 0;
	facades.meshes = 0;
	facades.meshSections = 0;
	facades.objectIndices = 0;
	facades.objectPositions = 0;
	objects.unknown0 = 0;
	objects.unknown1 = 0;
	objects.names = 0;
	objects.unknown2 = 0;
	objects.unknown3 = 0;
	objects.unknown4 = 0;
	objects.unknown5 = 0;
	objects.unknown6 = 0;
	objects.unknown7 = 0;
	trees.unknown0 = 0;
	trees.bases = 0;
	trees.meshes = 0;
	unknown.unknown2 = 0;
	unknown.unknown3 = 0;
	unknown.unknown4 = 0;
	textures.textures = 0;
}

Xbc::~Xbc()
{
	if (unknownPerCell) {
		delete[] unknownPerCell;
	}

	if (subfilesPerCell) {
		delete[] subfilesPerCell;
	}

	if (matrices) {
		delete[] matrices;
	}

	if (roads.meshes) {
		delete[] roads.meshes;
	}

	if (roads.meshSections) {
		delete[] roads.meshSections;
	}

	if (roads.objectIndices) {
		delete[] roads.objectIndices;
	}

	if (roads.objectPositions) {
		delete[] roads.objectPositions;
	}

	if (facades.meshes) {
		delete[] facades.meshes;
	}

	if (facades.meshSections) {
		delete[] facades.meshSections;
	}

	if (facades.objectIndices) {
		delete[] facades.objectIndices;
	}

	if (facades.objectPositions) {
		delete[] facades.objectPositions;
	}

	if (objects.unknown0) {
		delete[] objects.unknown0;
	}

	if (objects.unknown1) {
		delete[] objects.unknown1;
	}

	if (objects.names) {
		delete[] objects.names;
	}

	if (objects.unknown2) {
		delete[] objects.unknown2;
	}

	if (objects.unknown3) {
		delete[] objects.unknown3;
	}

	if (objects.unknown4) {
		delete[] objects.unknown4;
	}

	if (objects.unknown5) {
		delete[] objects.unknown5;
	}

	if (objects.unknown6) {
		delete[] objects.unknown6;
	}

	if (objects.unknown7) {
		delete[] objects.unknown7;
	}

	if (trees.unknown0) {
		delete[] trees.unknown0;
	}

	if (trees.bases) {
		delete[] trees.bases;
	}

	if (trees.meshes) {
		delete[] trees.meshes;
	}

	if (unknown.unknown2) {
		delete[] unknown.unknown2;
	}

	if (unknown.unknown3) {
		delete[] unknown.unknown3;
	}

	if (unknown.unknown4) {
		delete[] unknown.unknown4;
	}

	if (textures.textures) {
		delete[] textures.textures;
	}
}

void Xbc::read(std::ifstream& ifs)
{
	// Header
	parse(ifs, version);

	if (version.compare(KnownVersion) != 0) {
		std::ostringstream msg;
		msg << "Unexpected format version. Expected \"" << KnownVersion << "\", got \"" << version << "\".";
		throw std::runtime_error(msg.str());
	}

	parse(ifs, colCount);
	parse(ifs, rowCount);
	parse(ifs, name);
	parse(ifs, aabb3);
	parse(ifs, aabb2);
	parse(ifs, maxY);
	parse(ifs, unknown0);
	parse(ifs, unknown1);

	parse(ifs, cellCount1);
	unknownPerCell = new uint32_t[cellCount1];
	ifs.read(reinterpret_cast<char*>(unknownPerCell), sizeof(uint32_t) * cellCount1);

	parse(ifs, cellCount2);
	subfilesPerCell = new uint32_t[cellCount2];
	ifs.read(reinterpret_cast<char*>(subfilesPerCell), sizeof(uint32_t) * cellCount2);

	parse(ifs, unknown2);

	parse(ifs, matrixCount);
	matrices = new Matrix[matrixCount];
	ifs.read(reinterpret_cast<char*>(matrices), sizeof(Matrix) * matrixCount);

	// Roads
	parse(ifs, roads.meshCount);
	roads.meshes = new Mesh<RoadVertex>[roads.meshCount];
	for (unsigned i = 0; i < roads.meshCount; i++) {
		roads.meshes[i].read(ifs);
	}

	parse(ifs, roads.textureLength);
	for (unsigned i = 0; i < RoadTextureCount; i++) {
		roads.textures[i].read(ifs);
	}

	parse(ifs, roads.meshSectionCount);
	roads.meshSections = new MeshSection[roads.meshSectionCount];
	for (unsigned i = 0; i < roads.meshSectionCount; i++) {
		roads.meshSections[i].read(ifs);
	}

	parse(ifs, roads.objectIndexCount);
	roads.objectIndices = new uint16_t[roads.objectIndexCount];
	ifs.read(reinterpret_cast<char*>(roads.objectIndices), sizeof(uint16_t) * roads.objectIndexCount);

	parse(ifs, roads.objectPositionCount);
	roads.objectPositions = new RoadObjectPosition[roads.objectPositionCount];
	ifs.read(reinterpret_cast<char*>(roads.objectPositions), sizeof(RoadObjectPosition) * roads.objectPositionCount);

	// Facades
	parse(ifs, facades.meshCount);
	facades.meshes = new Mesh<FacadeVertex>[facades.meshCount];
	for (unsigned i = 0; i < facades.meshCount; i++) {
		facades.meshes[i].read(ifs);
	}

	parse(ifs, facades.textureLength);
	for (unsigned i = 0; i < FacadeTextureCount; i++) {
		facades.textures[i].read(ifs);
	}

	parse(ifs, facades.meshSectionCount);
	facades.meshSections = new MeshSection[facades.meshSectionCount];
	for (unsigned i = 0; i < facades.meshSectionCount; i++) {
		facades.meshSections[i].read(ifs);
	}

	parse(ifs, facades.objectIndexCount);
	facades.objectIndices = new uint16_t[facades.objectIndexCount];
	ifs.read(reinterpret_cast<char*>(facades.objectIndices), sizeof(uint16_t) * facades.objectIndexCount);

	parse(ifs, facades.objectPositionCount);
	facades.objectPositions = new FacadeObjectPosition[facades.objectPositionCount];
	ifs.read(reinterpret_cast<char*>(facades.objectPositions), sizeof(FacadeObjectPosition) * facades.objectPositionCount);

	// Objects
	parse(ifs, objects.unknown0Count);
	objects.unknown0 = new ObjectUnknown0[objects.unknown0Count];
	ifs.read(reinterpret_cast<char*>(objects.unknown0), sizeof(ObjectUnknown0) * objects.unknown0Count);

	parse(ifs, objects.unknown1Count);
	objects.unknown1 = new ObjectUnknown1[objects.unknown1Count];
	ifs.read(reinterpret_cast<char*>(objects.unknown1), sizeof(ObjectUnknown1) * objects.unknown1Count);

	parse(ifs, objects.nameCount);
	objects.names = new std::string[objects.nameCount];
	for (unsigned i = 0; i < objects.nameCount; i++) {
		parse(ifs, objects.names[i]);
	}

	parse(ifs, objects.unknown2Count);
	objects.unknown2 = new Vec3f[objects.unknown2Count];
	ifs.read(reinterpret_cast<char*>(objects.unknown2), sizeof(Vec3f) * objects.unknown2Count);

	parse(ifs, objects.unknown3Count);
	objects.unknown3 = new Vec3f[objects.unknown3Count * 2];
	ifs.read(reinterpret_cast<char*>(objects.unknown3), sizeof(Vec3f) * objects.unknown3Count * 2);

	parse(ifs, objects.unknown4Count);
	objects.unknown4 = new Vec3f[objects.unknown4Count];
	ifs.read(reinterpret_cast<char*>(objects.unknown4), sizeof(Vec3f) * objects.unknown4Count);

	parse(ifs, objects.unknown5Count);
	objects.unknown5 = new Vec4f[objects.unknown5Count];
	ifs.read(reinterpret_cast<char*>(objects.unknown5), sizeof(Vec4f) * objects.unknown5Count);

	parse(ifs, objects.unknown6Count);
	objects.unknown6 = new float[objects.unknown6Count * 7];
	ifs.read(reinterpret_cast<char*>(objects.unknown6), sizeof(float) * objects.unknown6Count * 7);

	parse(ifs, objects.unknown7Count);
	objects.unknown7 = new Vec3f[objects.unknown7Count * 2];
	ifs.read(reinterpret_cast<char*>(objects.unknown7), sizeof(Vec3f) * objects.unknown7Count * 2);

	// Trees
	parse(ifs, trees.unknown0Count);
	trees.unknown0 = new Vec4f[trees.unknown0Count];
	ifs.read(reinterpret_cast<char*>(trees.unknown0), sizeof(Vec4f) * trees.unknown0Count);

	parse(ifs, trees.baseCount);
	trees.bases = new TreeBase[trees.baseCount];
	for (unsigned i = 0; i < trees.baseCount; i++) {
		trees.bases[i].read(ifs);
	}

	parse(ifs, trees.meshCount);
	trees.meshes = new TreeMesh[trees.meshCount];
	for (unsigned i = 0; i < trees.meshCount; i++) {
		trees.meshes[i].read(ifs);
	}

	// Seasons
	for (unsigned i = 0; i < SeasonTextureCount; i++) {
		seasons[i].read(ifs);
	}

	// Unknown
	parse(ifs, unknown.unknown0);
	parse(ifs, unknown.unknown1);

	parse(ifs, unknown.unknown2Count);
	unknown.unknown2 = new uint16_t[unknown.unknown2Count * 4];
	ifs.read(reinterpret_cast<char*>(unknown.unknown2), sizeof(uint16_t) * unknown.unknown2Count * 4);

	parse(ifs, unknown.unknown3Count);
	unknown.unknown3 = new uint32_t[unknown.unknown3Count];
	ifs.read(reinterpret_cast<char*>(unknown.unknown3), sizeof(uint32_t) * unknown.unknown3Count);

	parse(ifs, unknown.unknown4Count);
	unknown.unknown4 = new uint32_t[unknown.unknown4Count];
	ifs.read(reinterpret_cast<char*>(unknown.unknown4), sizeof(uint32_t) * unknown.unknown4Count);

	// Textures
	parse(ifs, textures.textureCount);
	textures.textures = new ProcessedTexture[textures.textureCount];
	for (unsigned i = 0; i < textures.textureCount; i++) {
		textures.textures[i].read(ifs);
	}

	textures.noise.read(ifs);
}

Xbc* Xbc::readFile(std::ifstream& ifs)
{
	Xbc* xbc = new Xbc();

	xbc->read(ifs);

	return xbc;
}