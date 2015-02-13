#pragma once

#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

namespace cmp
{
	enum Version : uint32_t
	{
		Version109 = 109,
		Version114 = 114,
		Version115 = 115,
	};

	struct Vertex
	{
		int      x  : 11;
		int      y  : 11;
		int      z  : 10;
		int      nx : 11;
		int      ny : 11;
		int      nz : 10;
		int      u0 : 11;
		int      v0 : 11;
		int      u1 : 10;
		uint8_t  materialId;
		uint8_t  matrixId;
		uint8_t  v1;
		uint8_t  demolitionId;
		int      dx : 11;
		int      dy : 11;
		int      dz : 10;
		uint8_t  specularPower;
		uint8_t  envMapIntensity;
		uint8_t  ambientIntensity;
		uint8_t  specularIntensity;

		float    scaleX(float scale)       { return ((float)x / 1024.0) * scale; }
		float    scaleY(float scale)       { return ((float)y / 1024.0) * scale; }
		float    scaleZ(float scale)       { return ((float)z /  512.0) * scale; }
		float    scaleNX()                 { return (float)nx / 1024.0; }
		float    scaleNY()                 { return (float)ny / 1024.0; }
		float    scaleNZ()                 { return (float)nz /  512.0; }
		float    scaleU0()                 { return (float)u0 / 1024.0; }
		float    scaleV0()                 { return (float)v0 / 1024.0; }
		float    scaleU1()                 { return (float)u1 /  512.0; }
		float    scaleV1()                 { return (float)v1 /  256.0; }
		float    scaleDX()                 { return (float)dx / 1024.0; }
		float    scaleDY()                 { return (float)dy / 1024.0; }
		float    scaleDZ()                 { return (float)dz /  512.0; }
		unsigned actualMaterialId()        { return materialId    / 11; }
		unsigned actualMatrixId()          { return matrixId      / 37; }
		unsigned actualDemolitionId()      { return demolitionId  / 23; }
		float    actualEnvMapIntensity()   { return (float)envMapIntensity   / 256.0; }
		float    actualAmbientIntensity()  { return (float)ambientIntensity  / 256.0; }
		float    actualSpecularIntensity() { return (float)specularIntensity / 256.0; }
		float    actualSpecularPower()     { return (float)specularPower     / 256.0; }
	};

	struct  Vec3f
	{
		float x, y, z;
	};

	struct BoundBox
	{
		Vec3f min, max;
	};

	struct Color4f
	{
		float r, g, b, a;
	};

	struct Color4b {
		uint8_t b, g, r, a;
	};

	struct Mat4x3
	{
		float a[4][3];
	};

	struct Transformation
	{
		uint32_t flags;
		Mat4x3   world;
		Mat4x3   relative;
		Vec3f    unknown0;
		float    unknown1;
	};

	class MeshData;
	typedef std::vector<MeshData*> MeshList;

	class Element
	{
		public:
			Element(Version version);
			virtual ~Element() {}
			virtual void read(std::ifstream& ifs) = 0;

			Version     version;
			std::string name;

		protected:
			template<class T>
			static void parse(std::istream& in, T& var) { in.read(reinterpret_cast<char*>(&var), sizeof(var)); }
			static void parse(std::istream& in, std::string& var) { std::getline(in, var, '\0'); }
	};

	class Node : public Element
	{
		public:
			enum Type : uint32_t
			{
				Root      = 0,
				Transform = 1,
				Mesh      = 2,
				Axis      = 3,
				Light     = 4,
				Smoke     = 5,
				MultiMesh = 6,
			};

			Node(Version version, Type type);
			virtual ~Node() = 0;
			virtual void read(std::ifstream& ifs);
			static Node* readNode(std::ifstream& ifs, Version version);
			virtual void findMeshes(MeshList* meshList) {}

			Type type;
	};

	class GroupNode : public Node
	{
		public:
			GroupNode(Version version, Type type) : Node(version, type) {}
			virtual ~GroupNode() = 0;
			virtual void read(std::ifstream& ifs);
			virtual void findMeshes(MeshList* meshList);

			BoundBox           aabb;
			std::vector<Node*> children;
	};

	struct RootEntry {
		Vec3f    unknown0;
		uint32_t unknown1;
	};

	class RootNode : public GroupNode
	{
		public:
			RootNode(Version version);
			virtual ~RootNode();
			virtual void read(std::ifstream& ifs);
			static RootNode* readFile(std::ifstream& ifs);
			void resolveReferences();

			uint32_t       unknown0;
			uint32_t       unknown1;
			uint16_t       unknown2;
			uint8_t        unknown3;
			std::string    path;
			Transformation transformation;
			uint32_t       unknown4;
			uint8_t        unknown5;
			BoundBox       aabb2;
			uint32_t       rootEntryCount;
			uint32_t       unknown6;
			uint32_t       unknown7;
			uint32_t       unknown8;
			float          unknown9[3];
			RootEntry*     rootEntries;
			uint32_t       matrixCount;
	};

	class TransformNode : public GroupNode
	{
		public:
			TransformNode(Version version) : GroupNode(version, Transform) {}
			virtual ~TransformNode() {}
			virtual void read(std::ifstream& ifs);

			Transformation transformation;
			int32_t        matrixId;
	};

	class AxisNode : public Node
	{
		public:
			AxisNode(Version version) : Node(version, Axis) {}
			virtual ~AxisNode() {};
			virtual void read(std::ifstream& ifs);
	};

	class LightNode : public Node
	{
		public:
			enum LightType : uint32_t
			{
				HeadLight    = 0,
				BackLight    = 1,
				BrakeLight   = 2,
				ReverseLight = 3,
				Siren        = 4,
				SignalLeft   = 5,
				SignalRight  = 6,
				HeadLightEnv = 8,
				SirenEnv     = 9,
			};

			LightNode(Version version) : Node(version, Light) {}
			virtual ~LightNode() {};
			virtual void read(std::ifstream& ifs);

			LightType lightType;
			int32_t   isTogglable;
			float     unknown0;
			float     brightness;
			float     unknown1;
			float     size;
			float     unknown2;
			Color4b   color;
			float     unknown3;
			float     unknown4;
	};

	class SmokeNode : public Node
	{
		public:
			SmokeNode(Version version) : Node(version, Smoke) {}
			virtual ~SmokeNode() {};
			virtual void read(std::ifstream& ifs);

			int32_t unknown0;
	};

	struct Primitive
	{
		enum Type : uint16_t
		{
			TriangleList  = 0x6001,
			TriangleStrip = 0x8801,
		};

		Primitive(Type type) : type(type) {}
		virtual ~Primitive() {}
		Type type;
		uint16_t offset;
		uint16_t count;
		uint16_t unknown[5];
	};

	struct TriangleList : public Primitive
	{
		TriangleList() : Primitive(Type::TriangleList) {}

		uint16_t minIndex;
		uint16_t vertexCount;
	};

	struct TriangleStrip : public Primitive
	{
		TriangleStrip() : Primitive(Type::TriangleStrip) {}
	};

	struct Material
	{
		uint32_t minIndex;
		uint32_t vertexCount;
		uint32_t offset;
		uint32_t count;
		uint8_t  isTriangleStrip;
		uint32_t material;

		Primitive::Type Type() { return isTriangleStrip ? Primitive::Type::TriangleStrip : Primitive::Type::TriangleList; }
	};

	struct NumberPlateVertex{
		uint32_t unknown[7];
	};

	class MeshData : public Element
	{
		public:
			MeshData(Version version);
			virtual ~MeshData();
			virtual void read(std::ifstream& ifs);

			uint32_t    length;
			float       unknown0;
			BoundBox    aabb;
			uint32_t    vertexCount1;
			uint32_t    indexCount;
			uint32_t    unknown1;
			uint32_t    unknown2;
			uint32_t    unknown3;
			Color4f     color;

			std::string path;

			uint8_t     hasIndices;
			uint32_t    unknown4;
			uint32_t    indicesLength;
			uint16_t*   indices;

			uint16_t    unknown5;
			uint16_t    unknown6;
			uint8_t     unknown7;

			uint32_t    vertexCount2;
			uint32_t    vertexStride;
			uint32_t    verticesLength;
			uint32_t    unknown8;
			Vertex*     vertices;

			uint32_t    primitiveAndMaterialCount;
			std::vector<Primitive*> primitives;

			uint32_t    materialCount;
			std::vector<Material*> materials;

			uint32_t    hasNumberPlate;
			uint32_t    numberPlateVertexCount;
			NumberPlateVertex* numberPlateVertices;

			MeshData*   reference;
	};

	class MeshNode : public Node
	{
		public:
			MeshNode(Version version, Type type) : Node(version, type) {}
			virtual ~MeshNode();
			virtual void read(std::ifstream& ifs);
			virtual void findMeshes(MeshList* meshList);
			bool hasBound() { return type == MultiMesh; }

			int32_t            unknown0;
			int32_t            loose;
			int32_t            drop;
			int32_t            unknown1;
			BoundBox           aabb;
			std::vector<MeshData*> meshes;
	};
}
