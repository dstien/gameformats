#include <cstring>
#include <iomanip>
#include <iostream>

#include "cmp.h"

std::ostream& operator<<(std::ostream& lhs, cmp::Node::Type type)
{
	switch (type) {
		case cmp::Node::Root:      lhs << "Root";      break;
		case cmp::Node::Transform: lhs << "Transform"; break;
		case cmp::Node::Mesh1:	   lhs << "Mesh1";     break;
		case cmp::Node::Axis:	   lhs << "Axis";      break;
		case cmp::Node::Light:	   lhs << "Light";     break;
		case cmp::Node::Smoke:	   lhs << "Smoke";     break;
		case cmp::Node::Mesh2:	   lhs << "Mesh2";     break;
		default: lhs << "Unknown (" << (uint32_t)type << ")";
	}

	return lhs;
}

void printNode(cmp::Node* node)
{
	static int level = 0;

	int indent = 4 * level;
	
	std::cout << std::setw(indent) << "" << node->type << " node (\"" << node->name << "\")" << std::endl;

	switch (node->type) {
		case cmp::Node::Root:
		case cmp::Node::Transform:
		case cmp::Node::Axis:
		case cmp::Node::Light:
		case cmp::Node::Smoke:
			break;
		case cmp::Node::Mesh1:
		case cmp::Node::Mesh2:
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);
			if (meshNode) {
				for (cmp::Mesh* mesh : meshNode->meshes) {
					std::cout << std::setw(indent + 4) << "" << "Mesh \"" << mesh->name << "\" (" << mesh->length << ") bytes" << std::endl;
					std::cout << std::setw(indent + 8) << "" << mesh->vertexCount2 << " vertices" << std::endl;
					std::cout << std::setw(indent + 8) << "" << mesh->indexCount << " indices" << std::endl;
					std::cout << std::setw(indent + 8) << "" << mesh->unparsedLength << " unparsed bytes" << std::endl;
				}
			}
			break;
	}

	cmp::GroupNode* group = dynamic_cast<cmp::GroupNode*>(node);
	if (group) {
		level++;
		for (cmp::Node* node : group->children) {
			printNode(node);
		}
		level--;
	}
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " filename.cmp" << std::endl;
		return 1;
	}

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	std::cout << "Reading \"" << argv[1] << "\"" << std::endl;

	try {
		ifs.open(argv[1], std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

		std::streampos length = ifs.tellg();
		ifs.seekg(0);

		cmp::RootNode* root = cmp::RootNode::readFile(ifs);

		std::cout << "Finished reading with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

		ifs.close();

		printNode(root);
		std::cout << std::endl;

		delete root;
	}
	catch (const std::ios_base::failure& e) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
