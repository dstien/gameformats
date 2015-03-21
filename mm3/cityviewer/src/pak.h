#pragma once

#include <cstdint>

#include "util.h"

namespace pak
{
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