#include <iomanip>
#include <iostream>
#include <sstream>

#include "pak.h"
#include "xbc.h"

struct DdsHdr {
	uint32_t magic;
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t plsize;
	uint32_t depth;
	uint32_t mips;
	uint32_t reserved1[11];

	struct {
		uint32_t size;
		uint32_t flags;
		uint32_t fourcc;
		uint32_t rgbbits;
		uint32_t rmask;
		uint32_t gmask;
		uint32_t bmask;
		uint32_t amask;
	} pxfmt;

	struct {
		uint32_t caps1;
		uint32_t caps2;
		uint32_t reserved[2];
	} caps;

	uint32_t reserved2;
};

void printCity(const xbc::Xbc* xbc)
{
	std::cout << "version:   " << xbc->version << std::endl;
	std::cout << "colCount:  " << xbc->colCount << std::endl;
	std::cout << "rowCount:  " << xbc->rowCount << std::endl;
	std::cout << "name:      " << xbc->name << std::endl;
	std::cout << "cellCount1: " << xbc->cellCount1 << std::endl;
	std::cout << "cellCount2: " << xbc->cellCount2 << std::endl;

	unsigned count = 0;
	for (unsigned i = 0; i < xbc->cellCount2; i++) {
		count += xbc->subfilesPerCell[i];
	}
	std::cout << "Sum subfiles: " << count << std::endl;
	std::cout << "matrixCount: " << xbc->matrixCount << std::endl;

	std::cout << "--- Roads ---" << std::endl;
	std::cout << "meshCount: " << xbc->roads.meshCount << std::endl;
	/*
	for (unsigned i = 0; i < xbc->roads.meshCount; i++) {
		std::cout << "  mesh " << i << ":  vertexCount: " << xbc->roads.meshes[i].vertexCount << "  indexCount: " << xbc->roads.meshes[i].indexCount << std::endl;
	}
	std::cout << "textureLength: " << xbc->roads.textureLength << std::endl;
	
	for (unsigned i = 0; i < xbc::RoadTextureCount; i++) {
		std::cout << "  texture " << i << ": \"" << xbc->roads.textures[i].name << "\"" << std::endl;
	}
	std::cout << "meshSectionCount: " << xbc->roads.meshSectionCount << std::endl;
	for (unsigned i = 0; i < xbc->roads.meshSectionCount; i++) {
		std::cout << "  meshSection " << i << ":  meshId: " << xbc->roads.meshSections[i].meshId << std::endl;
	}
	*/
	std::cout << "objectIndexCount: " << xbc->roads.objectIndexCount << std::endl;
	std::cout << "objectPositionCount: " << xbc->roads.objectPositionCount << std::endl;

	std::cout << "--- Facades ---" << std::endl;
	std::cout << "meshCount: " << xbc->facades.meshCount << std::endl;
	/*
	std::cout << "textureLength: " << xbc->facades.textureLength << std::endl;
	for (unsigned i = 0; i < xbc::FacadeTextureCount; i++) {
		std::cout << "  texture " << i << ": \"" << xbc->facades.textures[i].name << "\"" << std::endl;
	}
	std::cout << "meshSectionCount: " << xbc->facades.meshSectionCount << std::endl;
	for (unsigned i = 0; i < xbc->facades.meshSectionCount; i++) {
		std::cout << "  meshSection " << i << ":  meshId: " << xbc->facades.meshSections[i].meshId << std::endl;
	}
	*/
	std::cout << "objectIndexCount: " << xbc->facades.objectIndexCount << std::endl;
	std::cout << "objectPositionCount: " << xbc->facades.objectPositionCount << std::endl;

	std::cout << "--- Objects ---" << std::endl;
	std::cout << "unknown0Count: " << xbc->objects.unknown0Count << std::endl;
	std::cout << "unknown1Count: " << xbc->objects.unknown1Count << std::endl;
	/*
	for (unsigned i = 0; i < xbc->objects.unknown1Count; i++) {
		std::cout << "  object " << i << ":  unk0: " << xbc->objects.unknown1[i].id << std::endl;
	}
	std::cout << "nameCount: " << xbc->objects.nameCount << std::endl;
	for (unsigned i = 0; i < xbc->objects.nameCount; i++) {
		std::cout << "  object " << i << ":  name: \"" << xbc->objects.names[i] << "\"" << std::endl;
	}
	*/
	std::cout << "unknown2Count: " << xbc->objects.unknown2Count << std::endl;
	std::cout << "unknown3Count: " << xbc->objects.unknown3Count << std::endl;
	std::cout << "unknown4Count: " << xbc->objects.unknown4Count << std::endl;
	std::cout << "unknown5Count: " << xbc->objects.unknown5Count << std::endl;
	std::cout << "unknown6Count: " << xbc->objects.unknown6Count << std::endl;
	std::cout << "unknown7Count: " << xbc->objects.unknown7Count << std::endl;

	std::cout << "--- Trees ---" << std::endl;
	std::cout << "unknown0Count: " << xbc->trees.unknown0Count << std::endl;
	std::cout << "baseCount: " << xbc->trees.baseCount << std::endl;
	/*
	for (unsigned i = 0; i < xbc->trees.baseCount; i++) {
		std::cout << "  Base " << i << ":  name: \"" << xbc->trees.bases[i].name << "\"" << std::endl;
	}
	std::cout << "meshCount: " << xbc->trees.meshCount << std::endl;

	for (unsigned i = 0; i < xbc::SeasonTextureCount; i++) {
		std::cout << "  texture " << i << ": \"" << xbc->seasons[i].name << "\"" << std::endl;
	}
	*/
	std::cout << "--- Unknown ---" << std::endl;
	std::cout << "unknown0: " << xbc->unknown.unknown0 << std::endl;
	std::cout << "unknown1: " << xbc->unknown.unknown1 << std::endl;

	std::cout << "--- Textures ---" << std::endl;
	std::cout << "textureCount: " << xbc->textures.textureCount << std::endl;

	for (unsigned i = 0; i < xbc->textures.textureCount; i++) {
		//std::cout << "  texture " << std::setw(3) << i << " " << std::setw(2) << xbc->textures.textures[i].type << " \"" << xbc->textures.textures[i].name << "\"" << std::endl;
	}
	std::cout << "pakTextureCount: " << xbc->pakTextureCount << std::endl;
}

bool dumpTexture(const xbc::TextureHeader* tex, const char* data, const char* city, const char* prefix, unsigned num, unsigned variant)
{
	std::ostringstream name;
	name << city << '\\' << prefix << '_' << std::setfill('0') << std::setw(3) << num << '_' << variant << '_' << tex->name << ".dds";
	std::cout << "n: " << std::setw(64) << std::left << name.str() << std::right << " i: " << std::setw(3) << num << " t: " << std::setw(2) << tex->type << " u: " << std::setw(10) << tex->unknown << " f: " << tex->format << std::endl;

	DdsHdr hdr = { 0 };
	hdr.magic  = 0x20534444;
	hdr.size   = sizeof(DdsHdr) - 4;
	hdr.height = tex->height;
	hdr.width  = tex->width;
	hdr.mips   = tex->mips;
	hdr.pxfmt.size = 32;

	if (tex->format == xbc::TextureHeader::Format::L8) {
		hdr.flags  = 0x0002100F;
		hdr.plsize = tex->width;
		hdr.depth  = 1;

		hdr.pxfmt.flags   = 0x00020000;
		hdr.pxfmt.rgbbits = 8;
		hdr.pxfmt.rmask   = 0x000000FF;
		hdr.caps.caps1    = 0x00001000;
	}
	else {
		hdr.flags  = 0x00081007;
		hdr.plsize = 0;
		hdr.depth  = 0;

		hdr.pxfmt.flags  = 0x00000004;
		hdr.pxfmt.fourcc = 0x31545844;
		hdr.caps.caps1   = 0x00401008;
	}

	std::ofstream ofs;
	ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	try
	{
		ofs.open(name.str(), std::ifstream::out | std::ifstream::binary | std::ifstream::trunc);
		ofs.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
		ofs.write(data, tex->actualDataLength());
		ofs.close();

		return true;
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return false;
}

void dumpTextures(const xbc::Xbc* xbc, pak::Toc* toc)
{
	for (unsigned i = 0; i < xbc::RoadTextureCount; i++) {
		dumpTexture(&xbc->roads.textures[i], xbc->roads.textures[i].mainData, xbc->name.c_str(), "road", i, 1);

		if (xbc->roads.textures[i].isInterleaved()) {
			dumpTexture(&xbc->roads.textures[i], xbc->roads.textures[i].maskData, xbc->name.c_str(), "road", i, 2);
		}
	}

	for (unsigned i = 0; i < xbc::FacadeTextureCount; i++) {
		dumpTexture(&xbc->facades.textures[i], xbc->facades.textures[i].mainData, xbc->name.c_str(), "facade", i, 1);

		if (xbc->facades.textures[i].isInterleaved()) {
			dumpTexture(&xbc->facades.textures[i], xbc->facades.textures[i].maskData, xbc->name.c_str(), "facade", i, 2);
		}
	}

	for (unsigned i = 0; i < xbc::SeasonTextureCount; i++) {
		dumpTexture(&xbc->seasons[i], xbc->seasons[i].mainData, xbc->name.c_str(), "season", i, 1);

		if (xbc->seasons[i].isInterleaved()) {
			dumpTexture(&xbc->seasons[i], xbc->seasons[i].maskData, xbc->name.c_str(), "season", i, 2);
		}
	}

	unsigned pakTexIndex = 0;
	for (unsigned i = 0; i < xbc->textures.textureCount; i++) {
		dumpTexture(&xbc->textures.textures[i].texture, xbc->textures.textures[i].texture.mainData, xbc->name.c_str(), "texture_xbc", i, 1);

		if (xbc->textures.textures[i].texture.isInterleaved()) {
			dumpTexture(&xbc->textures.textures[i].texture, xbc->textures.textures[i].texture.maskData, xbc->name.c_str(), "texture_xbc", i, 2);
		}

		if (xbc->textures.textures[i].hasDataInPak()) {
			char* data = toc->getPakData(pakTexIndex);
			if (data) {
				dumpTexture(&xbc->textures.textures[i], data, xbc->name.c_str(), "texture_pak", i, 1);
				delete[] data;
			}
			pakTexIndex++;
		}
	}

	dumpTexture(&xbc->textures.noise, xbc->textures.noise.mainData, xbc->name.c_str(), "noise", 0, 1);

	if (xbc->textures.noise.isInterleaved()) {
		dumpTexture(&xbc->textures.noise, xbc->textures.noise.maskData, xbc->name.c_str(), "noise", 0, 2);
	}
}

void printToc(const pak::Toc* toc)
{
	for (unsigned i = 0; i < toc->entryCount; i++) {
		std::cout << std::setw(4) << i << " off: " << std::setw(10) << toc->entries[i].offset << "  len: " << std::setw(10) << toc->entries[i].length << "  unk: " << std::setw(10) << toc->entries[i].unknown << std::endl;
	}
}

bool dumpMap(const xbc::Xbc* xbc, const pak::Cell* cell)
{
	std::ostringstream name;
	name << xbc->name << '\\' << "Cell" << std::setfill('0') << std::setw(3) << cell->id << ".dds";

	DdsHdr hdr = { 0 };
	hdr.magic  = 0x20534444;
	hdr.size   = sizeof(DdsHdr) - 4;
	hdr.height = cell->heightMap.width;
	hdr.width  = cell->heightMap.width;
	hdr.mips   = 0;
	hdr.pxfmt.size = 32;

	hdr.flags  = 0x0002100F;
	hdr.plsize = cell->heightMap.width;
	hdr.depth  = 1;

	hdr.pxfmt.flags   = 0x00020000;
	hdr.pxfmt.rgbbits = 8;
	hdr.pxfmt.rmask   = 0x000000FF;
	hdr.caps.caps1    = 0x00001000;

	std::ofstream ofs;
	ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	try
	{
		ofs.open(name.str(), std::ifstream::out | std::ifstream::binary | std::ifstream::trunc);
		ofs.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
		ofs.write(cell->heightMap.data, cell->heightMap.width * cell->heightMap.width);
		ofs.close();

		return true;
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return false;
}

bool dumpMaps(const xbc::Xbc* xbc, pak::Toc* toc)
{
	unsigned pakIndex = xbc->unknown.unknown1 ? xbc->pakTextureCount : 0;
	unsigned width = 101;

	unsigned sizes[1024];

	for (unsigned i = 0; i < xbc->cellCount1; i++) {
		sizes[i] = toc->entries[pakIndex].length;
		pak::Cell* cell = toc->getCell(pakIndex);
		if (i == 0) {
			width = cell->heightMap.width;
		}
		if (!dumpMap(xbc, cell)) {
			delete cell;
			return false;
		}
		pakIndex += xbc->subfilesPerCell[i] + 8 + 1;
		delete cell;
	}

	std::ostringstream html;

	html << R"(<!DOCTYPE html>
<html>
	<head>
		<title>)" << xbc->name << R"(</title>
		<style>
			table {
				border: 1px solid #ccc;
				border-collapse: collapse;
			}
			td {
				border: 1px dotted #ccc;
				margin: 0;
				padding: 0;
				color: yellow;
				font-family: monospace, sans-serif;
				font-size: x-small;
				text-shadow: 0 0 1px #000;
			}
			td > div {
				position: relative;
				overflow: hidden;
				height: )" << width << R"(px;
				width:  )" << width << R"(px;
			}
			td > div > img{
				margin: 0;
				padding: 0;
				position: absolute;
				z-index: -1;
			}
		</style>
	</head>
	<body>
		<table>
			<tbody>
)";

	for (signed y = xbc->rowCount - 1; y >= 0; y--) {
		html << "<tr>\n";
		for (unsigned x = 0; x < xbc->colCount; x++) {
			unsigned i = y * xbc->colCount + x;
			html << R"(<td><div>
<img src="Cell)" << std::setfill('0') << std::setw(3) << i << R"(.png" width=")" << width << R"(" height=")" << width << R"(" />
#)" << i << "<br/>sub: " << xbc->subfilesPerCell[i] << "<br/>unk: " << xbc->unknownPerCell[i] << /*"<br/>siz: " << sizes[i] <<*/ R"(</div></td>
)";
		}
		html << "</tr>\n";
	}

	html << R"(			</tbody>
		</table>
	</body>
</html>)";

	std::ofstream ofs;
	ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	std::ostringstream name;
	name << xbc->name << '\\' << "Map\\index.html";

	try
	{
		ofs.open(name.str(), std::ifstream::out | std::ifstream::binary | std::ifstream::trunc);
		ofs << html.str();
		ofs.close();

		return true;
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return false;
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
		return 1;
	}

	std::string xbcFilename = argv[1], tocFilename = argv[1], pakFilename = argv[1];
	xbcFilename.append(".xbc");
	tocFilename.append(".toc");
	pakFilename.append(".pak");

	xbc::Xbc* xbc;
	pak::Toc* toc;

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	try {
		// XBC
		{
			std::cout << "Reading \"" << xbcFilename << "\"" << std::endl;
			ifs.open(xbcFilename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

			std::streampos length = ifs.tellg();
			ifs.seekg(0);

			xbc = xbc::Xbc::readFile(ifs);

			std::cout << "Finished reading with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

			ifs.close();
		}
		// TOC
		{
			std::cout << "Reading \"" << tocFilename << "\"" << std::endl;
			ifs.open(tocFilename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

			std::streampos length = ifs.tellg();
			ifs.seekg(0);

			toc = pak::Toc::readFile(ifs);

			std::cout << "Finished reading with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

			ifs.close();
		}
		// PAK
		{
			std::cout << "Opening \"" << pakFilename << "\"" << std::endl;
			ifs.open(pakFilename, std::ifstream::in | std::ifstream::binary);
			toc->setPakStream(&ifs);
		}

		//printCity(xbc);
		//printToc(toc);
		std::cout << std::endl;

		//dumpTextures(xbc, toc);
		dumpMaps(xbc, toc);
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
		return 2;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 3;
	}

	delete xbc;
	delete toc;

	ifs.close();

	return 0;
}