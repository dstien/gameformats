#pragma once

#include <fstream>
#include <string>

namespace util
{
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
}