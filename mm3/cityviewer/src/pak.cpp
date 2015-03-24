#include "pak.h"

#include <algorithm>

using namespace pak;

Cell::Cell()
{
	heightMap.data = 0;
}

Cell::~Cell()
{
	if (heightMap.data) {
		delete[] heightMap.data;
	}
}


void Cell::read(std::ifstream& ifs)
{
	unsigned baseOffset = ifs.tellg();
	parse(ifs, id);
	parse(ifs, shadowMapCount);
	parse(ifs, facadeSectionsCount);
	parse(ifs, facadeSectionOffset);
	parse(ifs, unknown0Offset);
	parse(ifs, unknown0Length);
	parse(ifs, facadeIndexCount);
	parse(ifs, facadeIndexOffset);
	parse(ifs, unknown1Count);
	parse(ifs, unknown1Offset);
	parse(ifs, junctionCount);
	parse(ifs, junctionOffset);
	parse(ifs, unknown2Count);
	parse(ifs, unknown2Offset);
	parse(ifs, unknown3Count);
	parse(ifs, unknown3Offset);
	parse(ifs, unknown4Count);
	parse(ifs, unknown4Offset);
	parse(ifs, unknown5Count);
	parse(ifs, unknown5Offset);
	parse(ifs, unknown6Offset1);
	parse(ifs, unknown6Offset2);
	parse(ifs, unknown6Count);
	parse(ifs, unknown7Offset);
	parse(ifs, unknown8Offset);
	parse(ifs, unknown9Count);
	parse(ifs, unknown9Offset);
	parse(ifs, unknown10);
	parse(ifs, unknown11);
	parse(ifs, heightMap.unknown);
	parse(ifs, heightMap.offset);
	parse(ifs, heightMap.width);

	// Map
	heightMap.data = new char[heightMap.width * heightMap.width];
	ifs.seekg(baseOffset + heightMap.offset);
	ifs.read(heightMap.data, heightMap.width * heightMap.width);
}

Cell* Cell::readFile(std::ifstream& ifs)
{
	Cell* cell = new Cell();

	cell->read(ifs);

	return cell;
}

Toc::Toc()
{
	entries = 0;
}

Toc::~Toc()
{
	if (entries) {
		delete[] entries;
	}
}

char* Toc::getPakData(unsigned subfile)
{
	if (!entries || subfile > entryCount) {
		return 0;
	}

	pak->seekg(entries[subfile].offset);

	char* data = new char[entries[subfile].length];

	pak->read(data, entries[subfile].length);

	return data;
}

Cell* Toc::getCell(unsigned subfile)
{
	if (!entries || subfile > entryCount) {
		return 0;
	}

	pak->seekg(entries[subfile].offset);

	return Cell::readFile(*pak);
}

void Toc::read(std::ifstream& ifs)
{
	parse(ifs, length);
	parse(ifs, unknown);
	parse(ifs, entryCount);

	entries = new TocEntry[entryCount];
	ifs.read(reinterpret_cast<char*>(entries), sizeof(TocEntry) * entryCount);

	// Sort entries by offset instead of unknown.
	std::sort(entries, entries + entryCount);
}

Toc* Toc::readFile(std::ifstream& ifs)
{
	Toc* toc = new Toc();

	toc->read(ifs);

	return toc;
}