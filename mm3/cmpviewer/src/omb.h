#pragma once

#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

namespace omb
{
	enum TexMode : uint32_t
	{
		Decal        = 0,
		Transparency = 1,
		Replace      = 2,
		Modulate     = 3,
	};

	struct Color4b
	{
		uint8_t b, g, r, a;
	};

	class Element
	{
		public:
			virtual ~Element() {}
			virtual void read(std::ifstream& ifs) = 0;

		protected:
			template<class T>
			static void parse(std::istream& in, T& var) { in.read(reinterpret_cast<char*>(&var), sizeof(var)); }
			static void parse(std::istream& in, std::string& var) { std::getline(in, var, '\0'); }
	};

	class Material : public Element
	{
		public:
			virtual void read(std::ifstream& ifs);

			std::string name;
			std::string texture;
			uint8_t     unknown0;
			Color4b     color;
			TexMode     mode;
	};

	class MaterialSet : public Element
	{
		public:
			virtual void read(std::ifstream& ifs);
			static MaterialSet* readFile(std::ifstream& ifs);

			std::vector<Material> materials;
	};
}
