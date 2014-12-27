#include "omb.h"

#include <sstream>
#include <stdexcept>

using namespace omb;

void Material::read(std::ifstream& ifs)
{
	parse(ifs, name);
	parse(ifs, texture);
	parse(ifs, unknown0);
	parse(ifs, color);
	parse(ifs, mode);
}

void MaterialSet::read(std::ifstream& ifs)
{
	uint8_t unknown;
	parse(ifs, unknown);

	if (unknown != 0) {
		std::ostringstream msg;
		msg << "Unknown material set header. Expected 0, got " << unknown << ".";
		throw std::runtime_error(msg.str());
	}

	uint32_t materialCount;
	parse(ifs, materialCount);

	for (unsigned i = 0; i < materialCount; i++) {
		Material material;
		material.read(ifs);
		materials.push_back(material);
	}
}

MaterialSet* MaterialSet::readFile(std::ifstream& ifs)
{
	MaterialSet* set = new MaterialSet();

	set->read(ifs);

	return set;
}
