#include <cstring>
#include <iomanip>
#include <iostream>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include "cmp.h"
#include "omb.h"

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

#define ATTRIB_NO_INTENSITIES  10
#define ATTRIB_NM_INTENSITIES  "intensities"
#define ATTRIB_NO_IDS          11
#define ATTRIB_NM_IDS          "ids"
#define UNIFORM_MATRICES       "matrices"
#define UNIFORM_TEX_MODE       "texMode"
#define UNIFORM_DEMOLITION_MAP "demolitionMap"

typedef std::vector<osg::ref_ptr<osg::StateSet>> StateSetList;

const char* vertexShader = R"(
#version 330

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform mat3 osg_NormalMatrix;
uniform mat4 matrices[37];

in vec4 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_Color;
in vec2 osg_MultiTexCoord0;
in vec2 osg_MultiTexCoord1;
in vec4 intensities;
in vec4 ids;

out vec3 N;
out vec3 L;
out vec3 E;

out vec4 vcolor;
out vec2 uv0;
out vec2 uv1;
out float ambientIntensity;
out float specularIntensity;
out float specularPower;
out float envMapIntensity;

void main()
{
	vec4 vert;
	vec3 norm;
	if (bool(ids.w)) {
		vert = matrices[int(ids.y)] * osg_Vertex;
		norm = transpose(inverse(mat3(matrices[int(ids.y)]))) * osg_Normal;
	}
	else {
		vert = osg_Vertex;
		norm = osg_Normal;
	}

	N = osg_NormalMatrix * norm;
	E = -(osg_ModelViewMatrix * vert).xyz;
	L = (osg_ViewMatrix * vec4(osg_ViewMatrixInverse[3].xyz, 1)).xyz + E;

	vcolor = osg_Color;
	uv0 = osg_MultiTexCoord0;
	uv1 = osg_MultiTexCoord1;
	ambientIntensity  = intensities.x;
	specularIntensity = intensities.y;
	specularPower     = intensities.z;
	envMapIntensity   = intensities.w;

	gl_Position = osg_ModelViewProjectionMatrix * vert;
}
)";

const char* fragmentShader = R"(
#version 330

uniform int texMode;
uniform sampler2D osg_Sampler0;
uniform sampler2D demolitionMap;

in vec3 N;
in vec3 L;
in vec3 E;

in vec4 vcolor;
in vec2 uv0;
in vec2 uv1;
in float ambientIntensity;
in float specularIntensity;
in float specularPower;
in float envMapIntensity;

out vec4 color;

void main(void)
{
	vec3 materialColor;
	float alpha;
	vec4 t0 = texture(osg_Sampler0, uv0);
	vec4 t1 = texture(demolitionMap, uv1);

	float spec;

	switch (texMode) {
		case -1: // Solid color
			materialColor = vcolor.rgb;
			alpha = vcolor.a;
			break;
		case 0: // Decal
			materialColor = (vcolor.rgb * (vec3(1, 1, 1) - vec3(t0.a, t0.a, t0.a))) + (t0.rgb * vec3(t0.a, t0.a, t0.a));
			alpha = vcolor.a;
			break;
		case 1: // Transparency
		case 2: // Replace
			materialColor = t0.rgb;
			alpha = t0.a;
			break;
		case 3: // Modulate
			if (t0.a == 1.0) {
				materialColor = t0.rgb;
			}
			else {
				materialColor = vcolor.rgb * t0.rgb;
			}
			alpha = vcolor.a;
			break;
		default: // Unknown
			materialColor = vec3(0, 1, 0);
			alpha = 1.0;
	}

	/*
	if (t1.a > 0.0) {
		if (t1.a == 1.0) {
			materialColor = t1.rgb;
			alpha = 1.0;
		}
		else {
			materialColor = mix(materialColor, t1.rgb, 0.5);
		}

		spec = 0.0;
	}
	else*/ {
		spec = specularIntensity;
	}

	vec3 n = normalize(N);
	vec3 l = normalize(L);
	vec3 e = normalize(E);
	vec3 r = reflect(-l, n);

	vec3 ambient  = materialColor * vec3(ambientIntensity, ambientIntensity, ambientIntensity);
	vec3 diffuse  = materialColor * clamp(dot(n, l), 0, 1);
	vec3 specular = vec3(spec, spec, spec) * pow(clamp(dot(e, r), 0, 1), specularPower * 5.0);

	color.rgb = ambient + diffuse + specular;
	color.a = alpha;
}
)";

std::ostream& operator<<(std::ostream& lhs, cmp::Node::Type type)
{
	switch (type) {
		case cmp::Node::Root:      lhs << "Root";      break;
		case cmp::Node::Transform: lhs << "Transform"; break;
		case cmp::Node::Mesh:      lhs << "Mesh";      break;
		case cmp::Node::Axis:      lhs << "Axis";      break;
		case cmp::Node::Light:     lhs << "Light";     break;
		case cmp::Node::Smoke:     lhs << "Smoke";     break;
		case cmp::Node::MultiMesh: lhs << "MultiMesh"; break;
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

void printNode(cmp::Node* node, omb::MaterialSet* materials)
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
				std::cout << std::setw(indent + 4) << "" << "Matrices: " << rootNode->matrixCount << std::endl;
				}
			break;
		}
		case cmp::Node::Transform:
		{
			cmp::TransformNode* transformNode = dynamic_cast<cmp::TransformNode*>(node);
			if (transformNode && transformNode->matrixId != -1) {
				std::cout << std::setw(indent + 4) << "" << "Matrix id: " << transformNode->matrixId << std::endl;
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
		case cmp::Node::Mesh:
		case cmp::Node::MultiMesh:
		{
			cmp::MeshNode* meshNode = dynamic_cast<cmp::MeshNode*>(node);
			if (meshNode) {
				for (cmp::MeshData* mesh : meshNode->meshes) {
					std::cout << std::setw(indent + 4) << "" << "Mesh \"" << mesh->name << "\" (" << mesh->length << " bytes)" << std::endl;
					if (mesh->length) {
						std::cout << std::setw(indent + 8) << "" << mesh->vertexCount2 << " vertices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->indexCount << " indices" << std::endl;
						std::cout << std::setw(indent + 8) << "" << mesh->primitives.size() << " primitives" << std::endl;
						for (int i = 0; i < mesh->primitives.size(); i++) {
							omb::Material material = materials->materials.at(mesh->materials[i]->material);
							std::cout << std::setw(indent + 12) << "" << "Type: " << std::left << std::setw(13) << mesh->primitives[i]->type << " Material: \"" << material.name << "\" Texture: \"" << material.texture << "\"" << std::endl;
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
			printNode(node, materials);
		}
		level--;
	}
}

StateSetList generateOSGMaterials(omb::MaterialSet* materials, std::string basepath, osg::ref_ptr<osg::Uniform> matricesUniform)
{
	StateSetList states;

	osg::ref_ptr<osg::Shader> vert = new osg::Shader(osg::Shader::Type::VERTEX, vertexShader);
	vert->setName("vertexShader");

	osg::ref_ptr<osg::Shader> frag = new osg::Shader(osg::Shader::Type::FRAGMENT, fragmentShader);
	frag->setName("fragmentShader");

	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->setName("program");
	program->addShader(vert.get());
	program->addShader(frag.get());
	program->addBindAttribLocation(ATTRIB_NM_INTENSITIES, ATTRIB_NO_INTENSITIES);
	program->addBindAttribLocation(ATTRIB_NM_IDS, ATTRIB_NO_IDS);

	// Load demolition texture.
	osg::ref_ptr<osg::Uniform> demolitionUniform = 0;
	osg::ref_ptr<osg::Texture2D> demolitionTex = 0;
	{
		std::string texpath = osgDB::findFileInDirectory("../demolition_texture.dds", basepath, osgDB::CaseSensitivity::CASE_INSENSITIVE);

		osg::ref_ptr<osg::Image> img = 0;

		if (texpath != "") {
			img = osgDB::readImageFile(texpath);
		}

		if (!img) {
			std::cerr << "Error: Couldn't load demolition texture \"" << basepath << osgDB::getNativePathSeparator() << ".." << osgDB::getNativePathSeparator() << "demolition_texture.dds\"" << std::endl;
		}
		else {
			demolitionTex = new osg::Texture2D();
			demolitionTex->setImage(img.get());
			demolitionUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, UNIFORM_DEMOLITION_MAP); 
			demolitionUniform->set(1);
		}
	}

	for (omb::Material mat : materials->materials) {
		osg::ref_ptr<osg::StateSet> state = new osg::StateSet();
		osg::ref_ptr<osg::Material> material = new osg::Material();

		material->setName(mat.name);

		material->setDiffuse(osg::Material::FRONT, osg::Vec4(mat.color.r / 255.0, mat.color.g / 255.0f, mat.color.b / 255.0, mat.color.a / 255.0));
		state->setAttributeAndModes(program.get(), osg::StateAttribute::ON);

		if (mat.texture == "No") {
			state->addUniform(new osg::Uniform(UNIFORM_TEX_MODE, -1));
		}
		else {
			std::string texpath = osgDB::findFileInDirectory(osgDB::getSimpleFileName(mat.texture), basepath, osgDB::CaseSensitivity::CASE_INSENSITIVE);

			osg::ref_ptr<osg::Image> img = 0;

			if (texpath != "") {
				img = osgDB::readImageFile(texpath);
			}

			if (!img) {
				std::cerr << "Error: Couldn't load texture \"" << basepath << osgDB::getNativePathSeparator() << osgDB::getSimpleFileName(mat.texture) << "\"" << std::endl;
				state->addUniform(new osg::Uniform(UNIFORM_TEX_MODE, -1));
			}
			else {
				osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D();
				tex->setImage(img.get());
				state->setTextureAttributeAndModes(0, tex.get(), osg::StateAttribute::ON);
				state->addUniform(new osg::Uniform(UNIFORM_TEX_MODE, (int)mat.mode));
			}
		}

		if (demolitionTex) {
			state->setTextureAttributeAndModes(1, demolitionTex.get(), osg::StateAttribute::ON);
			state->addUniform(demolitionUniform.get());
		}
		else {
			std::cerr << "Demolition texture loading failed" << std::endl;
		}

		if (mat.mode == omb::TexMode::Transparency || mat.color.a != 0xFF) {
			osg::ref_ptr<osg::BlendFunc> blend = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
			state->setAttributeAndModes(blend.get(), osg::StateAttribute::ON);
			state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}
		else {
			state->setMode(GL_BLEND, osg::StateAttribute::OFF);
		}

		state->addUniform(matricesUniform.get());

		state->setAttributeAndModes(material.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		states.push_back(state);
	}

	return states;
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

osg::ref_ptr<osg::Geode> drawMesh(cmp::MeshData* mesh, StateSetList* states, bool isMultiMesh, osg::Node::NodeMask mask)
{
	if (!mesh->length) {
		if (mesh->reference) {
			// TODO: Re-use geode.
			return drawMesh(mesh->reference, states, isMultiMesh, mask);
		}

		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		return geode.get();
	}

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	cmp::BoundBox* b = &mesh->aabb;
	float maxX = (b->min.x - b->max.x) * -1;
	float maxY = (b->min.y - b->max.y) * -1;
	float maxZ = (b->min.z - b->max.z) * -1;

	osg::ref_ptr<osg::Vec3Array> vertices    = new osg::Vec3Array(mesh->vertexCount2);
	osg::ref_ptr<osg::Vec3Array> normals     = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);
	osg::ref_ptr<osg::Vec2Array> uvs0        = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);
	osg::ref_ptr<osg::Vec2Array> uvs1        = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);
	osg::ref_ptr<osg::Vec4Array> colors      = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);
	osg::ref_ptr<osg::Vec4Array> intensities = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);
	osg::ref_ptr<osg::Vec4bArray> ids        = new osg::Vec4bArray(osg::Array::BIND_PER_VERTEX, mesh->vertexCount2);

	// TODO: Apply deformations in vertex shader.
	float damage = 0.0f;

	if (mesh->hasNumberPlate) {
		damage = 0.0f;
	}

	for (unsigned i = 0; i < mesh->vertexCount2; i++) {
		cmp::Vertex* v = &mesh->vertices[i];
		vertices->at(i) = osg::Vec3(v->scaleX(maxX) + (v->scaleDX() * damage), v->scaleY(maxY) + (v->scaleDY() * damage), -v->scaleZ(maxZ) + (-v->scaleDZ() * damage));
		normals->at(i) = osg::Vec3(v->scaleNX(), v->scaleNY(), -v->scaleNZ());
		uvs0->at(i) = osg::Vec2(v->scaleU0(), v->scaleV0());
		uvs1->at(i) = osg::Vec2(v->scaleU1(), v->scaleV1());

		if (v->actualMaterialId() < states->size()) {
			osg::ref_ptr<osg::Material> material = (osg::Material*)states->at(v->actualMaterialId())->getAttribute(osg::StateAttribute::MATERIAL);

			if (material) {
				colors->at(i) = material->getDiffuse(osg::Material::FRONT);
			}
		}

		intensities->at(i) = osg::Vec4(v->actualAmbientIntensity(), v->actualSpecularIntensity(), v->actualSpecularPower(), v->actualEnvMapIntensity());
		ids->at(i) = osg::Vec4b(v->actualMaterialId(), v->actualMatrixId(), v->actualDemolitionId(), isMultiMesh);
	}

	osg::ref_ptr<osg::VertexBufferObject> vbo = new osg::VertexBufferObject();
	vbo->addArray(vertices.get());
	vbo->addArray(normals.get());
	vbo->addArray(uvs0.get());
	vbo->addArray(uvs1.get());
	vbo->addArray(colors.get());
	vbo->addArray(intensities.get());
	vbo->addArray(ids.get());

	// One geometry per material.
	osg::ref_ptr<osg::Geometry>* matgeo = new osg::ref_ptr<osg::Geometry>[states->size()];
	::memset(matgeo, 0, sizeof(osg::ref_ptr<osg::Geometry>) * states->size());

	unsigned int i = 0;
	for (cmp::Primitive* primitive : mesh->primitives) {
		unsigned materialId = mesh->materials.at(i)->material;

		// Create new geometry node once per material.
		if (!matgeo[materialId]) {
			matgeo[materialId] = new osg::Geometry();

			matgeo[materialId]->setUseVertexBufferObjects(true);
			matgeo[materialId]->setVertexArray(vertices.get());
			matgeo[materialId]->setNormalArray(normals.get());
			matgeo[materialId]->setTexCoordArray(0, uvs0.get());
			matgeo[materialId]->setTexCoordArray(1, uvs1.get());
			matgeo[materialId]->setColorArray(colors.get());

			matgeo[materialId]->setVertexAttribArray(ATTRIB_NO_INTENSITIES, intensities.get());
			matgeo[materialId]->setVertexAttribBinding(ATTRIB_NO_INTENSITIES, osg::Geometry::BIND_PER_VERTEX);

			matgeo[materialId]->setVertexAttribArray(ATTRIB_NO_IDS, ids.get());
			matgeo[materialId]->setVertexAttribBinding(ATTRIB_NO_IDS, osg::Geometry::BIND_PER_VERTEX);

			matgeo[materialId]->setStateSet(states->at(materialId).get());

			geode->addDrawable(matgeo[materialId].get());
		}

		switch (primitive->type) {
			case cmp::Primitive::Type::TriangleList:
			{
				cmp::TriangleList* list = dynamic_cast<cmp::TriangleList*>(primitive);
				if (list) {
					unsigned length = (list->count + 1) * 3;
					osg::ref_ptr<osg::DrawElementsUInt> triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, length);
					for (unsigned i = 0; i < length; i++) {
						triangles->at(i) = mesh->indices[list->offset + i];
					}

					matgeo[materialId]->addPrimitiveSet(triangles.get());
				}
				break;
			}
			case cmp::Primitive::Type::TriangleStrip:
			{
				cmp::TriangleStrip* strip = dynamic_cast<cmp::TriangleStrip*>(primitive);
				if (strip) {
					unsigned length = strip->count + 3;
					osg::ref_ptr<osg::DrawElementsUInt> tristrip = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP, length);

					for (unsigned i = 0; i < length; i++) {
						tristrip->at(i) = strip->offset + i;
					}

					matgeo[materialId]->addPrimitiveSet(tristrip.get());
				}
				break;
			}
		}

		i++;
	}

	delete[] matgeo;

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

osg::ref_ptr<osg::Node> drawNode(cmp::Node* node, osg::Group* parent, StateSetList* states, osg::Uniform* matricesUniform, osg::Node::NodeMask mask = 0)
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

				if (transNode->matrixId >= 0) {
					matricesUniform->setElement(transNode->matrixId, cmpMatrix2osgMatrix(&transNode->transformation.world));
				}
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
				osg::ref_ptr<osg::Node> child = drawNode(node, group, states, matricesUniform, mask);
				if (child) {
					group->addChild(child.get());
				}
			}

			return group;
		}
		case cmp::Node::Mesh:
		case cmp::Node::MultiMesh:
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
			for (cmp::MeshData* mesh : meshNode->meshes) {
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

				group->addChild(drawMesh(mesh, states, node->type == cmp::Node::MultiMesh && lod == 0, mask));
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
	if (argc < 2 || argc > 3) {
		std::cerr << "Usage: " << argv[0] << " filename.cmp [materialSetNo]" << std::endl;
		return 1;
	}

	std::string materialSetNo;
	if (argc == 3) {
		materialSetNo = argv[2];
		if (materialSetNo.length() == 1) {
			materialSetNo = "0" + materialSetNo;
		}
	}
	else {
		materialSetNo = "00";
	}

	std::string cmppath = argv[1];
	std::string basepath = osgDB::getFilePath(cmppath);
	std::string ombpath = basepath + osgDB::getNativePathSeparator() + "materialSet" + materialSetNo + ".omb";

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

	std::cout << "Reading \"" << cmppath << "\"" << std::endl;

	cmp::RootNode* root;
	omb::MaterialSet* materials;

	try {
		// CMP
		{
			ifs.open(cmppath, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

			std::streampos length = ifs.tellg();
			ifs.seekg(0);

			root = cmp::RootNode::readFile(ifs);

			std::cout << "Finished reading CMP with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

			ifs.close();
		}
		// OMB
		{
			std::cout << "Reading \"" << ombpath << "\"" << std::endl;
			ifs.open(ombpath, std::ifstream::in | std::ifstream::binary | std::ifstream::ate);

			std::streampos length = ifs.tellg();
			ifs.seekg(0);

			materials = omb::MaterialSet::readFile(ifs);

			std::cout << "Finished reading OMB with " << length - ifs.tellg() << " bytes left in file" << std::endl << std::endl;;

			ifs.close();
		}
	}
	catch (const std::ios_base::failure&) {
		std::cerr << "Exception: " << ::strerror(errno) << std::endl;
		return 2;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 3;
	}

	printNode(root, materials);
	std::cout << std::endl;

	osg::ref_ptr<osg::Uniform> matricesUniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, UNIFORM_MATRICES, cmp::MaxMatrices);

	StateSetList states = generateOSGMaterials(materials, basepath, matricesUniform);

	osg::ref_ptr<osg::Group> world = new osg::Group();
	osg::ref_ptr<osg::Node> model = drawNode(root, world, &states, matricesUniform);
	world->addChild(model.get());

	// Flip and cull back faces.
	world->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::Mode::CLOCKWISE));
	world->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::Mode::BACK));

	delete root;
	delete materials;

	osgViewer::Viewer viewer;
	viewer.setUpViewInWindow(100, 50, 800, 600);
	viewer.getCamera()->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	viewer.getCamera()->setCullMask(CULL_BODY | CULL_TIRES | CULL_WINDOWS | CULL_INTERIOR | CULL_LOOSEPARTS | CULL_OTHER |  CULL_LOD1);

	viewer.addEventHandler(new KeyHandler(viewer.getCamera()));

	// Wireframe/light toggling.
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	osg::ref_ptr<osgGA::TrackballManipulator> trackball = new osgGA::TrackballManipulator();
	viewer.setCameraManipulator(trackball.get());

	viewer.setSceneData(world.get());

	osg::Vec3d eye, center, up;
	viewer.getCameraManipulator()->getHomePosition(eye, center, up);

	// Set default rotation.
	eye.set(-eye.y() * 0.5, -eye.y() * 0.3, eye.y() * 0.5);
	center.set(0.0, center.y() * 0.5, 0.0);
	up.set(0.0, 1.0, 0.0);
	viewer.getCameraManipulator()->setHomePosition(eye, center, up);
	viewer.home();

	viewer.realize();

	// VBO helpers.
	osg::State* state = viewer.getCamera()->getGraphicsContext()->getState();
	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	return viewer.run();
}
