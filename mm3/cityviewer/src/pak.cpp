#include "pak.h"

#include <algorithm>

using namespace pak;

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