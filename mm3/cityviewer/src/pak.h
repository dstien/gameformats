#pragma once

#include <cstdint>

#include "util.h"

namespace pak
{
	class Cell : public util::Element
	{
		public:
			Cell();
			virtual ~Cell();

			virtual void read(std::ifstream& ifs);
			static Cell* readFile(std::ifstream& ifs);

			uint32_t id;
			uint32_t shadowMapCount;
			uint32_t facadeSectionsCount;
			uint32_t facadeSectionOffset;
			uint32_t unknown0Offset;
			uint32_t unknown0Length;
			uint32_t facadeIndexCount;
			uint32_t facadeIndexOffset;
			uint32_t unknown1Count;
			uint32_t unknown1Offset;
			uint32_t junctionCount;
			uint32_t junctionOffset;
			uint32_t unknown2Count;
			uint32_t unknown2Offset;
			uint32_t unknown3Count;
			uint32_t unknown3Offset;
			uint32_t unknown4Count;
			uint32_t unknown4Offset;
			uint32_t unknown5Count;
			uint32_t unknown5Offset;
			uint32_t unknown6Offset1;
			uint32_t unknown6Offset2;
			uint32_t unknown6Count;
			uint32_t unknown7Offset;
			uint32_t unknown8Offset;
			uint32_t unknown9Count;
			uint32_t unknown9Offset;
			uint32_t unknown10;
			float    unknown11[4];

			struct {
				uint32_t unknown;
				uint32_t offset;
				uint32_t width;
				char*    data;
			} heightMap;
	};

	struct TocEntry
	{
		bool operator< (TocEntry &e) { return offset < e.offset; }

		uint32_t unknown;
		uint32_t length;
		uint32_t offset;
	};

	class Toc : public util::Element
	{
		public:
			Toc();
			virtual ~Toc();

			void  setPakStream(std::ifstream* ifs) { pak = ifs; }
			char* getPakData(unsigned subfile);
			Cell* getCell(unsigned subfile);

			virtual void read(std::ifstream& ifs);
			static Toc*  readFile(std::ifstream& ifs);

			uint32_t  length;
			uint32_t  unknown;
			uint32_t  entryCount;
			TocEntry* entries;

		private:
			std::ifstream* pak;
	};
}