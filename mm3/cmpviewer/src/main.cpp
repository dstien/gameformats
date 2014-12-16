#include <cstring>
#include <iomanip>
#include <iostream>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>

#include "cmp.h"

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

void printNode(cmp::Node* node)
{
	static int level = 0;

	int indent = 4 * level;
	
	std::cout << std::setw(indent) << "" << node->type << " node \"" << node->name << "\"" << std::endl;

	switch (node->type) {
		case cmp::Node::Root:
			break;
		case cmp::Node::Transform:
			{
				cmp::TransformNode* transformNode = dynamic_cast<cmp::TransformNode*>(node);
				if (transformNode && transformNode->meshIndex != -1) {
					std::cout << std::setw(indent + 4) << "" << "Mesh index: " << transformNode->meshIndex << std::endl;
				}
			}
			break;
		case cmp::Node::Axis:
		case cmp::Node::Light:
		case cmp::Node::Smoke:
			break;
		case cmp::Node::Mesh1:
		case cmp::Node::Mesh2:
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);
			if (meshNode) {
				for (cmp::Mesh* mesh : meshNode->meshes) {
					std::cout << std::setw(indent + 4) << "" << "Mesh \"" << mesh->name << "\" (" << mesh->length << " bytes)" << std::endl;
					if (mesh->length) {
						std::cout << std::setw(indent + 8) << "" << mesh->vertexCount2 << " vertices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->indexCount << " indices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->attributeCount << " attributes" << std::endl;
						for (cmp::Attribute* attr : mesh->attributes) {
							std::cout << std::setw(indent + 12) << "" << "Type: " << (int)attr->type << " Subtype: " << (int)attr->subtype << std::endl;
						}
						std::cout << std::setw(indent + 8) << "" << mesh->unparsedLength << " unparsed bytes" << std::endl;
					}
					else {
						std::cout << std::setw(indent + 8) << "" << "Reference " << (mesh->reference ? "resolved" : "not resolved") << std::endl;
					}
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

osg::ref_ptr<osg::Geode> drawBoundBox(cmp::BoundBox* aabb)
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

	return geode.get();
}

osg::ref_ptr<osg::Geode> drawMesh(cmp::Mesh* mesh)
{
	if (!mesh->length) {
		if (mesh->reference) {
			// TODO: Re-use geode.
			return drawMesh(mesh->reference);
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

	for (cmp::Attribute* attr : mesh->attributes) {
		cmp::TrianglesAttribute* triattr = dynamic_cast<cmp::TrianglesAttribute*>(attr);
		if (triattr) {
			osg::ref_ptr<osg::DrawElementsUInt> triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
			for (unsigned i = 0; i < (triattr->length * 3) + 3; i++) {
				triangles->insert(triangles->begin(), mesh->indices[triattr->offset + i]);
				//triangles->push_back(mesh->indices[triattr->offset + i]);
			}

			geometry->addPrimitiveSet(triangles.get());
			continue;
		}

		cmp::TriangleStripAttribute* stripattr = dynamic_cast<cmp::TriangleStripAttribute*>(attr);
		if (stripattr) {
			osg::ref_ptr<osg::DrawElementsUInt> strip = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP);

			for (unsigned i = 0; i < stripattr->length + 3; i++) {
				strip->insert(strip->begin(), stripattr->offset + i);
				//strip->push_back(stripattr->offset + i);
			}

			geometry->addPrimitiveSet(strip.get());
			continue;
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

	return geode.get();
}

osg::Matrix cmpMatrix2osgMatrix(cmp::Mat4x3* m)
{
	return osg::Matrix(
			m->a[0][0], m->a[1][0],  m->a[2][0], 0.0f,
			m->a[0][1], m->a[1][1],  m->a[2][1], 0.0f,
			m->a[0][2], m->a[1][2],  m->a[2][2], 0.0f,
			m->a[3][0], m->a[3][1], -m->a[3][2], 1.0f);
}

osg::ref_ptr<osg::Node> drawNode(cmp::Node* node, osg::Group* parent)
{
	switch (node->type) {
		case cmp::Node::Root:
		case cmp::Node::Transform:
		{
			cmp::GroupNode* groupNode = dynamic_cast<cmp::GroupNode*>(node);

			osg::ref_ptr<osg::MatrixTransform> group = new osg::MatrixTransform();

			// Group bound is relative to parent.
			parent->addChild(drawBoundBox(&groupNode->aabb));

			cmp::TransformNode* transNode = dynamic_cast<cmp::TransformNode*>(node);
			if (transNode) {
				group->setMatrix(cmpMatrix2osgMatrix(&transNode->transformation.relative));
			}

			for (cmp::Node* node : groupNode->children) {
				osg::ref_ptr<osg::Node> child = drawNode(node, group);
				if (child) {
					group->addChild(child.get());
				}
			}

			return group.get();
		}
		case cmp::Node::Mesh1:
		case cmp::Node::Mesh2:
		{
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);

			osg::ref_ptr<osg::Group> group = new osg::Group();

			if (meshNode->hasBound()) {
				//group->addChild(drawBoundBox(&meshNode->aabb));
			}

			for (cmp::Mesh* mesh : meshNode->meshes) {
				//group->addChild(drawBoundBox(&mesh->aabb));
				group->addChild(drawMesh(mesh));
				break;
			}

			return group.get();
		}
		case cmp::Node::Axis:
		case cmp::Node::Light:
		case cmp::Node::Smoke:
		default:
			return 0;
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
	catch (const std::ios_base::failure& e) {
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
	viewer.setUpViewInWindow(0, 0, 800, 600);
	viewer.getCamera()->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// Wireframe/light toggling.
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.setSceneData(world.get());
	viewer.realize();

	return viewer.run();
}
