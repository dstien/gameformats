#include <cstring>
#include <iomanip>
#include <iostream>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>

#include "cmp.h"

#define CULL_GROUP_AABB 1 <<  0
#define CULL_MESH_AABB  1 <<  1
#define CULL_BODY       1 <<  2
#define CULL_TIRES      1 <<  3
#define CULL_WINDOWS    1 <<  4
#define CULL_INTERIOR   1 <<  5
#define CULL_LOOSEPARTS 1 <<  6
#define CULL_OTHER      1 <<  7
#define CULL_LOD1       1 <<  8
#define CULL_LOD2       1 <<  9
#define CULL_LOD3       1 << 10

std::ostream& operator<<(std::ostream& lhs, cmp::Node::Type type)
{
	switch (type) {
		case cmp::Node::Root:      lhs << "Root";      break;
		case cmp::Node::Transform: lhs << "Transform"; break;
		case cmp::Node::Mesh1:     lhs << "Mesh1";     break;
		case cmp::Node::Axis:      lhs << "Axis";      break;
		case cmp::Node::Light:     lhs << "Light";     break;
		case cmp::Node::Smoke:     lhs << "Smoke";     break;
		case cmp::Node::Mesh2:     lhs << "Mesh2";     break;
		default: lhs << "Unknown (" << (uint32_t)type << ")";
	}

	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, cmp::Primitive::Type type)
{
	switch (type) {
		case cmp::Primitive::TriangleList:  lhs << "TriangleList";  break;
		case cmp::Primitive::TriangleStrip: lhs << "TriangleStrip"; break;
		default: lhs << "Unknown (" << (uint16_t)type << ")";
	}

	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, cmp::LightNode::LightType type)
{
	switch (type) {
	case cmp::LightNode::HeadLight:    lhs << "HeadLight";    break;
	case cmp::LightNode::BackLight:    lhs << "BackLight";    break;
	case cmp::LightNode::BrakeLight:   lhs << "BrakeLight";   break;
	case cmp::LightNode::ReverseLight: lhs << "ReverseLight"; break;
	case cmp::LightNode::Siren:        lhs << "Siren";        break;
	case cmp::LightNode::SignalLeft:   lhs << "SignalLeft";   break;
	case cmp::LightNode::SignalRight:  lhs << "SignalRight";  break;
	case cmp::LightNode::HeadLightEnv: lhs << "HeadLightEnv"; break;
	case cmp::LightNode::SirenEnv:     lhs << "SirenEnv";     break;
	default: lhs << "Unknown (" << (uint32_t)type << ")";
	}

	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, cmp::Color4b color)
{
	return lhs << "(R: " << (int)color.r << " G: " << (int)color.g << " B: " << (int)color.b << " A: " << (int)color.a << ")";
}

void printNode(cmp::Node* node)
{
	static int level = 0;

	int indent = 4 * level;
	
	std::cout << std::setw(indent) << "" << node->type << " node \"" << node->name << "\"" << std::endl;

	switch (node->type) {
		case cmp::Node::Root:
		{
			cmp::RootNode* rootNode = dynamic_cast<cmp::RootNode*>(node);
			if (rootNode) {
				std::cout << std::setw(indent + 4) << "" << "Version: " << rootNode->version << std::endl;
				std::cout << std::setw(indent + 4) << "" << "Mesh nodes: " << rootNode->meshNodeCount << std::endl;
			}
			break;
		}
		case cmp::Node::Transform:
		{
			cmp::TransformNode* transformNode = dynamic_cast<cmp::TransformNode*>(node);
			if (transformNode && transformNode->meshIndex != -1) {
				std::cout << std::setw(indent + 4) << "" << "Mesh index: " << transformNode->meshIndex << std::endl;
			}
			break;
		}
		case cmp::Node::Axis:
			break;
		case cmp::Node::Light:
		{
			cmp::LightNode* lightNode = dynamic_cast<cmp::LightNode*>(node);
			if (lightNode) {
				std::cout << std::setw(indent + 4) << "" << "Type: " << lightNode->lightType << std::endl;
				std::cout << std::setw(indent + 4) << "" << "Color: " << lightNode->color << std::endl;
			}
			break;
		}
		case cmp::Node::Smoke:
			break;
		case cmp::Node::Mesh1:
		case cmp::Node::Mesh2:
		{
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);
			if (meshNode) {
				for (cmp::Mesh* mesh : meshNode->meshes) {
					std::cout << std::setw(indent + 4) << "" << "Mesh \"" << mesh->name << "\" (" << mesh->length << " bytes)" << std::endl;
					if (mesh->length) {
						std::cout << std::setw(indent + 8) << "" << mesh->vertexCount2 << " vertices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->indexCount << " indices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->primitives.size() << " primitives" << std::endl;
						for (int i = 0; i < mesh->primitives.size(); i++) {
							std::cout << std::setw(indent + 12) << "" << "Type: " << std::left << std::setw(13) << mesh->primitives[i]->type << " Material: " << mesh->materials[i]->material << std::endl;
						}
						if (mesh->hasNumberPlate) {
							std::cout << std::setw(indent + 8) << "" << mesh->numberPlateVertexCount << " number plate vertices" << std::endl;
						}
					}
					else {
						std::cout << std::setw(indent + 8) << "" << "Reference: ";
						if (!mesh->reference) {
							std::cout << "Not resolved" << std::endl;
						}
						else {
							std::cout << "\"" << mesh->reference->name << "\"" << std::endl;
						}
					}
				}
			}
			break;
		}
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

osg::ref_ptr<osg::Geode> drawBoundBox(cmp::BoundBox* aabb, osg::Node::NodeMask mask)
{
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();

	vertices->push_back(osg::Vec3(aabb->max.x, aabb->max.y, -aabb->max.z)); // 1 0
	vertices->push_back(osg::Vec3(aabb->min.x, aabb->max.y, -aabb->max.z)); // 2 1
	vertices->push_back(osg::Vec3(aabb->max.x, aabb->min.y, -aabb->max.z)); // 3 2
	vertices->push_back(osg::Vec3(aabb->min.x, aabb->min.y, -aabb->max.z)); // 4 3

	vertices->push_back(osg::Vec3(aabb->max.x, aabb->max.y, -aabb->min.z)); // 5 4
	vertices->push_back(osg::Vec3(aabb->min.x, aabb->max.y, -aabb->min.z)); // 6 5
	vertices->push_back(osg::Vec3(aabb->max.x, aabb->min.y, -aabb->min.z)); // 7 6
	vertices->push_back(osg::Vec3(aabb->min.x, aabb->min.y, -aabb->min.z)); // 8 7

	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);

	indices->push_back(0);
	indices->push_back(1);
	indices->push_back(1);
	indices->push_back(3);
	indices->push_back(3);
	indices->push_back(2);
	indices->push_back(2);
	indices->push_back(0);

	indices->push_back(4);
	indices->push_back(5);
	indices->push_back(5);
	indices->push_back(7);
	indices->push_back(7);
	indices->push_back(6);
	indices->push_back(6);
	indices->push_back(4);

	indices->push_back(0);
	indices->push_back(4);
	indices->push_back(1);
	indices->push_back(5);
	indices->push_back(2);
	indices->push_back(6);
	indices->push_back(3);
	indices->push_back(7);

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	geometry->setVertexArray(vertices.get());
	geometry->addPrimitiveSet(indices.get());

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(geometry.get());

	geode->setNodeMask(mask);

	return geode;
}

osg::ref_ptr<osg::Geode> drawMesh(cmp::Mesh* mesh, osg::Node::NodeMask mask)
{
	if (!mesh->length) {
		if (mesh->reference) {
			// TODO: Re-use geode.
			return drawMesh(mesh->reference, mask);
		}

		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		return geode.get();
	}

	cmp::BoundBox* b = &mesh->aabb;
	float maxX = (b->min.x - b->max.x) * -1;
	float maxY = (b->min.y - b->max.y) * -1;
	float maxZ = (b->min.z - b->max.z) * -1;

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);

	for (unsigned i = 0; i < mesh->vertexCount2; i++) {
		cmp::Vertex* v = &mesh->vertices[i];
		vertices->push_back(osg::Vec3(v->scaleX(maxX), v->scaleY(maxY), -v->scaleZ(maxZ)));
		normals->push_back(osg::Vec3(v->scaleNX(), v->scaleNY(), -v->scaleNZ()));
	}

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	geometry->setVertexArray(vertices.get());
	geometry->setNormalArray(normals.get());

	for (cmp::Primitive* primitive : mesh->primitives) {
		switch (primitive->type) {
			case cmp::Primitive::Type::TriangleList:
			{
				cmp::TriangleList* list = dynamic_cast<cmp::TriangleList*>(primitive);
				if (list) {
					osg::ref_ptr<osg::DrawElementsUInt> triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
					for (unsigned i = 0; i < (list->count * 3) + 3; i++) {
						triangles->insert(triangles->begin(), mesh->indices[list->offset + i]);
						//triangles->push_back(mesh->indices[list->offset + i]);
					}

					geometry->addPrimitiveSet(triangles.get());
				}
				break;
			}
			case cmp::Primitive::Type::TriangleStrip:
			{
				cmp::TriangleStrip* strip = dynamic_cast<cmp::TriangleStrip*>(primitive);
				if (primitive) {
					osg::ref_ptr<osg::DrawElementsUInt> tristrip = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP);

					for (unsigned i = 0; i < strip->count + 3; i++) {
						tristrip->insert(tristrip->begin(), strip->offset + i);
						//tristrip->push_back(strip->offset + i);
					}

					geometry->addPrimitiveSet(tristrip.get());
					continue;
				}
				break;
			}
		}
	}

	if (mesh->color.a == 1.0f) {
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
		colors->push_back(osg::Vec4(mesh->color.r, mesh->color.g, mesh->color.b, mesh->color.a));
		geometry->setColorArray(colors.get());
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	}
	else {
		osg::ref_ptr<osg::StateSet> state = geometry->getOrCreateStateSet();
		state->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::ref_ptr<osg::Material> material = new osg::Material();
		material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(mesh->color.r, mesh->color.g, mesh->color.b, mesh->color.a));
		material->setAlpha(osg::Material::FRONT_AND_BACK, mesh->color.a);
		state->setAttributeAndModes(material.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	}

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(geometry.get());

	geode->setNodeMask(mask);

	return geode;
}

osg::Matrix cmpMatrix2osgMatrix(cmp::Mat4x3* m)
{
	return osg::Matrix(
			m->a[0][0], m->a[1][0],  m->a[2][0], 0.0f,
			m->a[0][1], m->a[1][1],  m->a[2][1], 0.0f,
			m->a[0][2], m->a[1][2],  m->a[2][2], 0.0f,
			m->a[3][0], m->a[3][1], -m->a[3][2], 1.0f);
}

osg::ref_ptr<osg::Node> drawNode(cmp::Node* node, osg::Group* parent, osg::Node::NodeMask mask = 0)
{
	switch (node->type) {
		case cmp::Node::Root:
		case cmp::Node::Transform:
		{
			cmp::GroupNode* groupNode = dynamic_cast<cmp::GroupNode*>(node);

			osg::ref_ptr<osg::MatrixTransform> group = new osg::MatrixTransform();

			// Group bound is relative to parent.
			parent->addChild(drawBoundBox(&groupNode->aabb, CULL_GROUP_AABB));

			cmp::TransformNode* transNode = dynamic_cast<cmp::TransformNode*>(node);
			if (transNode) {
				group->setMatrix(cmpMatrix2osgMatrix(&transNode->transformation.relative));
			}

			if (mask == 0) {
				if (node->name == "body") {
					mask = CULL_BODY;
				}
				else if (node->name == "looseparts") {
					mask = CULL_LOOSEPARTS;
				}
				else if (node->name == "lowdash") {
					mask = CULL_INTERIOR;
				}
				else if (node->name == "glass_windows") {
					mask = CULL_WINDOWS;
				}
				else if (!node->name.compare(0, 4, "tire")) {
					mask = CULL_TIRES;
				}
			}

			for (cmp::Node* node : groupNode->children) {
				osg::ref_ptr<osg::Node> child = drawNode(node, group, mask);
				if (child) {
					group->addChild(child.get());
				}
			}

			return group;
		}
		case cmp::Node::Mesh1:
		case cmp::Node::Mesh2:
		{
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);

			osg::ref_ptr<osg::Group> group = new osg::Group();

			if (meshNode->hasBound()) {
				group->addChild(drawBoundBox(&meshNode->aabb, CULL_MESH_AABB));
			}

			if (!mask) {
				mask = CULL_OTHER;
			}

			int lod = 0;
			for (cmp::Mesh* mesh : meshNode->meshes) {
				switch (lod) {
					case 0:
						//mask |= CULL_LOD1;
						break;
					case 1:
						mask = CULL_LOD2;
						break;
					default:
						mask = CULL_LOD3;
				}

				group->addChild(drawBoundBox(&mesh->aabb, CULL_MESH_AABB));

				group->addChild(drawMesh(mesh, mask));
				lod++;
			}

			return group;
		}
		case cmp::Node::Axis:
		case cmp::Node::Light:
		case cmp::Node::Smoke:
		default:
			return 0;
	}
}

class KeyHandler : public osgGA::GUIEventHandler
{
	public:
		KeyHandler(osg::Camera* camera) : camera(camera) {}

		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
		{
			if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
					switch (ea.getKey()) {
						//case '1':
						//	camera->setCullMask(camera->getCullMask() ^ CULL_LOD1);
						//	return true;
						case '2':
							camera->setCullMask(camera->getCullMask() ^ CULL_LOD2);
							return true;
						case '3':
							camera->setCullMask(camera->getCullMask() ^ CULL_LOD3);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F1:
							camera->setCullMask(camera->getCullMask() ^ CULL_GROUP_AABB);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F2:
							camera->setCullMask(camera->getCullMask() ^ CULL_MESH_AABB);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F3:
							camera->setCullMask(camera->getCullMask() ^ CULL_BODY);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F4:
							camera->setCullMask(camera->getCullMask() ^ CULL_TIRES);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F5:
							camera->setCullMask(camera->getCullMask() ^ CULL_WINDOWS);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F6:
							camera->setCullMask(camera->getCullMask() ^ CULL_INTERIOR);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F7:
							camera->setCullMask(camera->getCullMask() ^ CULL_LOOSEPARTS);
							return true;
						case osgGA::GUIEventAdapter::KeySymbol::KEY_F8:
							camera->setCullMask(camera->getCullMask() ^ CULL_OTHER);
							return true;
					}
			}

			return false;
		}

	private:
		osg::Camera* camera;
};

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " filename.cmp" << std::endl;
		return 1;
	}

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	std::cout << "Reading \"" << argv[1] << "\"" << std::endl;

	cmp::RootNode* root;

	try {
		ifs.open(argv[1], std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

		std::streampos length = ifs.tellg();
		ifs.seekg(0);

		root = cmp::RootNode::readFile(ifs);

		std::cout << "Finished reading with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

		ifs.close();

		printNode(root);
		std::cout << std::endl;
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
		return 2;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 3;
	}

	osg::ref_ptr<osg::Group> world = new osg::Group();
	osg::ref_ptr<osg::Node> model = drawNode(root, world);
	world->addChild(model.get());

	delete root;

	osgViewer::Viewer viewer;
	viewer.setUpViewInWindow(100, 50, 800, 600);
	viewer.getCamera()->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	viewer.getCamera()->setCullMask(CULL_BODY | CULL_TIRES | CULL_WINDOWS | CULL_INTERIOR | CULL_LOOSEPARTS | CULL_OTHER |  CULL_LOD1);

	viewer.addEventHandler(new KeyHandler(viewer.getCamera()));

	// Wireframe/light toggling.
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.setSceneData(world.get());
	viewer.realize();

	return viewer.run();
}
