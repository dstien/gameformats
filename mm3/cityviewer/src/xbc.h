#pragma once

#include <cstdint>

#include "util.h"

namespace xbc
{
	extern const char*    KnownVersion;
	extern const unsigned RoadTextureCount;
	extern const unsigned FacadeTextureCount;
	extern const unsigned SeasonTextureCount;

	struct Vec2f
	{
		float x, y;
	};

	struct Vec3f
	{
		float x, y, z;
	};

	struct Vec4f
	{
		float x, y, z, w;
	};

	struct BoundBox2
	{
		Vec2f min, max;
	};

	struct BoundBox3
	{
		Vec3f min, max;
	};

	struct Matrix
	{
		Vec4f x, y, z;
		Vec3f w;
	};

	struct RoadVertex
	{
		// Position
		int16_t x;
		int16_t y;
		int16_t z;
		int16_t unknown;
		// Normal, D3DVSDT_NORMPACKED3
		int   nx : 11;
		int   ny : 11;
		int   nz : 10;
		// Us, D3DVSDT_NORMPACKED3
		int   u0 : 11;
		int   u1 : 11;
		int   u2 : 10;
		// Vs, D3DVSDT_NORMPACKED3
		int   v0 : 11;
		int   v1 : 11;
		int   v2 : 10;
	};

	struct FacadeVertex
	{
		// Position, D3DVSDT_NORMPACKED3
		int     x : 11;
		int     y : 11;
		int     z : 10;
		int32_t unknown;
		// Us, D3DVSDT_NORMPACKED3
		int     u0 : 11;
		int     u1 : 11;
		int     u2 : 10;
		// Vs, D3DVSDT_NORMPACKED3
		int     v0 : 11;
		int     v1 : 11;
		int     v2 : 10;
	};

	struct RoadObjectPosition
	{
		Vec4f    rotation;
		Vec3f    position;
	};

	struct FacadeObjectPosition
	{
		float    unknown;
		Vec3f    position;
	};

	struct ObjectUnknown0
	{
		int16_t unknown0, unknown1;
		float   unknown2;
	};

	struct ObjectUnknown1
	{
		uint32_t unknown0;
		uint16_t unknown1;
		uint16_t id;
		BoundBox3 aabb;
		float     unknown2[42];
		uint16_t  unknown3;
		uint16_t  bound;
		uint16_t  unknown4;
		uint16_t  unknown5;
		uint16_t  verticesIndex;
		uint16_t  indicesIndex;
		uint16_t  unknown6;
		uint16_t  unknown7[15];
	};

	class MeshSection : public util::Element
	{
		public:
			enum Type : uint16_t
			{
				TriangleList = 0x6001,
				QuadList     = 0xC001,
			};

			virtual void read(std::ifstream& ifs);
			
			uint32_t     unknown0;
			uint32_t     meshId;

			Type         type;
			uint16_t     minIndex;
			uint16_t     vertexCount;
			uint16_t     offset;
			uint16_t     count;

			uint16_t     unknown1;
			uint16_t     unknown2[10];
			uint32_t     objectOffset;
			uint32_t     objectCount;
			uint32_t     indexInObjectsUnknown0;
			uint32_t     entriesInObjectsUnknown0;
			BoundBox3    aabb1;
			BoundBox3    aabb2;
			uint32_t     unknown3;
			uint32_t     unknown4[5];
	};

	template <typename VertexType>
	class Mesh : public util::Element
	{
		public:
			Mesh();
			virtual ~Mesh();
			virtual void read(std::ifstream& ifs);

			uint32_t     vertexCount;
			uint32_t     indexCount;
			VertexType*  vertices;
			uint16_t*    indices;
	};

	class TextureHeader : public util::Element
	{
		public:
			enum Format : uint32_t
			{
				DXT1          = 0,
				DXT1GlassMask = 1,
				DXT1AlphaMask = 2,
				L8            = 3,
			};

			virtual void read(std::ifstream& ifs);
			bool         isInterleaved()    const { return format == Format::DXT1GlassMask || format == Format::DXT1AlphaMask; }
			bool         hasDataInPak()     const { return type >= 10 && type <= 13; }
			unsigned     actualDataLength() const { return isInterleaved() ? dataLength / 2 : dataLength; }

			uint32_t     dataLength;
			std::string  name;
			uint32_t     type;
			uint32_t     unknown;
			uint32_t     width;
			uint32_t     height;
			uint32_t     stride;
			uint32_t     mips;
			Format       format;
	};

	class Texture : public TextureHeader
	{
		public:
			Texture();
			virtual ~Texture();
			virtual void read(std::ifstream& ifs);

			char*        mainData;
			char*        maskData;
	};

	class ProcessedTexture : public TextureHeader
	{
		public:
			virtual void read(std::ifstream& ifs);

			Texture      texture;
	};

	class TreeBase : public util::Element
	{
		public:
			TreeBase();
			virtual ~TreeBase();
			virtual void read(std::ifstream& ifs);

			std::string name;
			uint32_t    unknownCount;
			uint16_t*   unknown;
	};

	struct TreeVertex1
	{
		float x, y, z, u, v;
		int16_t unknown0, unknown1;
	};

	struct TreeVertex2
	{
		uint32_t unknown[4];
	};

	class TreeMesh : public util::Element
	{
		public:
			TreeMesh();
			virtual ~TreeMesh();
			virtual void read(std::ifstream& ifs);

			uint32_t     vertex1Count;
			TreeVertex1* vertices1;
			uint32_t     vertex2Count;
			TreeVertex2* vertices2;
			uint32_t     indexCount;
			uint16_t*    indices;
	};

	class Xbc : public util::Element
	{
		public:
			Xbc();
			virtual ~Xbc();

			virtual void read(std::ifstream& ifs);
			static Xbc*  readFile(std::ifstream& ifs);

			std::string  version;
			uint32_t     colCount;
			uint32_t     rowCount;
			std::string  name;
			BoundBox3    aabb3;
			BoundBox2    aabb2;
			float        maxY;
			uint32_t     unknown0[11];
			float        unknown1[7];
			uint32_t     cellCount1;
			uint32_t*    unknownPerCell;
			uint32_t     cellCount2;
			uint32_t*    subfilesPerCell;
			uint8_t      unknown2[16];
			uint32_t     matrixCount;
			Matrix*      matrices;

			struct {
				uint32_t              meshCount;
				Mesh<RoadVertex>*     meshes;
				uint32_t              textureLength;
				Texture               textures[5];
				uint32_t              meshSectionCount;
				MeshSection*          meshSections;
				uint32_t              objectIndexCount;
				uint16_t*             objectIndices;
				uint32_t              objectPositionCount;
				RoadObjectPosition*   objectPositions;
			} roads;

			struct {
				uint32_t              meshCount;
				Mesh<FacadeVertex>*   meshes;
				uint32_t              textureLength;
				Texture               textures[6];
				uint32_t              meshSectionCount;
				MeshSection*          meshSections;
				uint32_t              objectIndexCount;
				uint16_t*             objectIndices;
				uint32_t              objectPositionCount;
				FacadeObjectPosition* objectPositions;
			} facades;

			struct {
				uint32_t              unknown0Count;
				ObjectUnknown0*       unknown0;
				uint32_t              unknown1Count;
				ObjectUnknown1*       unknown1;
				uint32_t              nameCount;
				std::string*          names;
				uint32_t              unknown2Count;
				Vec3f*                unknown2;
				uint32_t              unknown3Count;
				Vec3f*                unknown3;
				uint32_t              unknown4Count;
				Vec3f*                unknown4;
				uint32_t              unknown5Count;
				Vec4f*                unknown5;
				uint32_t              unknown6Count;
				float*                unknown6;
				uint32_t              unknown7Count;
				Vec3f*                unknown7;
			} objects;

			struct {
				uint32_t              unknown0Count;
				Vec4f*                unknown0;
				uint32_t              baseCount;
				TreeBase*             bases;
				uint32_t              meshCount;
				TreeMesh*             meshes;
			} trees;

			Texture seasons[12];

			struct {
				uint32_t              unknown0;
				uint32_t              unknown1;
				uint32_t              unknown2Count;
				uint16_t*             unknown2;
				uint32_t              unknown3Count;
				uint32_t*             unknown3;
				uint32_t              unknown4Count;
				uint32_t*             unknown4;
			} unknown;

			struct {
				uint32_t              textureCount;
				ProcessedTexture*     textures;
				Texture               noise;
			} textures;
	};
}