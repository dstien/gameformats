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
		int x : 11;
		int y : 11;
		int z : 10;
		uint32_t unknown[5];

		float scaleX(float scale) { return ((float)x / 1024.0) * scale; }
		float scaleY(float scale) { return ((float)y / 1024.0) * scale; }
		float scaleZ(float scale) { return ((float)z /  512.0) * scale; }
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
				Mesh1     = 2,
				Axis      = 3,
				Light     = 4,
				Smoke     = 5,
				Mesh2     = 6,
			};

			Node(Version version, Type type);
			virtual ~Node() = 0;
			virtual void read(std::ifstream& ifs);
			static Node* readNode(std::ifstream& ifs, Version version);

			Type type;
	};

	class GroupNode : public Node
	{
		public:
			GroupNode(Version version, Type type) : Node(version, type) {}
			virtual ~GroupNode() = 0;
			virtual void read(std::ifstream& ifs);

			BoundBox           aabb;
			std::vector<Node*> children;
	};

	class RootNode : public GroupNode
	{
		public:
			RootNode(Version version) : GroupNode(version, Root) {}
			virtual ~RootNode() {}
			virtual void read(std::ifstream& ifs);
			static RootNode* readFile(std::ifstream& ifs);

			uint32_t    unknown0;
			uint32_t    unknown1;
			uint16_t    unknown2;
			uint8_t     unknown3;
			std::string path;
			uint8_t     unknown4[541];
			Vec3f       unknown5;
			uint32_t    unknown6;
			uint32_t    unknown7;
	};

	class TransformNode : public GroupNode
	{
		public:
			TransformNode(Version version) : GroupNode(version, Transform) {}
			virtual ~TransformNode() {}
			virtual void read(std::ifstream& ifs);

			float       unknown0[29];
			int32_t     unknown1;
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
			LightNode(Version version) : Node(version, Light) {}
			virtual ~LightNode() {};
			virtual void read(std::ifstream& ifs);

			int32_t unknown0;
			int32_t unknown1;
			float   unknown2[6];
			float   unknown3[2];
	};

	class SmokeNode : public Node
	{
		public:
			SmokeNode(Version version) : Node(version, Smoke) {}
			virtual ~SmokeNode() {};
			virtual void read(std::ifstream& ifs);

			int32_t unknown0;
	};

	struct Attribute
	{
		virtual ~Attribute() {}
		uint8_t type;
		uint8_t subtype;
	};

	struct MaterialAttribute : public Attribute
	{
		uint16_t unknown[12];
	};

	struct TrianglesAttribute : public Attribute
	{
		uint16_t unknown0;
		uint16_t unknown1;
		uint16_t offset;
		uint16_t length;
		uint16_t unknown2[5];
	};

	struct TriangleStripAttribute : public Attribute
	{
		uint16_t offset;
		uint16_t length;
		uint16_t unknown[5];
	};

	class Mesh : public Element
	{
		public:
			Mesh(Version version);
			virtual ~Mesh();
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

			uint32_t    attributeCount;
			std::vector<Attribute*> attributes;

			int         unparsedLength;
			uint8_t*    unparsed;
	};

	class MeshNode : public Node
	{
		public:
			MeshNode(Version version, Type type) : Node(version, type) {}
			virtual ~MeshNode();
			virtual void read(std::ifstream& ifs);
			bool hasBound() { return type == Mesh2; }

			int32_t            unknown0;
			int32_t            unknown1;
			int32_t            unknown2;
			int32_t            unknown3;
			BoundBox           aabb;
			std::vector<Mesh*> meshes;
	};
}
