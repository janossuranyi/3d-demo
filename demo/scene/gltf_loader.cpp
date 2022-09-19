
#include <glm/gtc/type_ptr.hpp>
#include "gpu_utils.h"
#include "logger.h"
#include "gltf_loader.h"


void GltfLoader::setNodeTransform(xNode* node, const tinygltf::Node& glnode)
{
	if (glnode.matrix.size())
	{
		node->applyMatrix(glm::make_mat4(glnode.matrix.data()));
	}
	else
	{
		if (glnode.translation.size())
		{
			node->translation = vec3(glm::make_vec3(glnode.translation.data()));
		}
		if (glnode.rotation.size())
		{
			node->rotation = quat(float(glnode.rotation[3]), float(glnode.rotation[0]), float(glnode.rotation[1]), float(glnode.rotation[2]));
		}
		if (glnode.scale.size())
		{
			node->scale = vec3(glm::make_vec3(glnode.scale.data()));
		}
		node->changed = true;
	}
}

void GltfLoader::parseNodes()
{
	_scene.nodes.clear();

	for (uint i = 0; i < _glmodel.nodes.size(); ++i)
	{
		uint xnode = _scene.addNode();
		xNode* pnode = _scene.nodes[xnode].get();

		const tinygltf::Node& node = _glmodel.nodes[i];
		pnode->name = node.name;
		pnode->children.assign(node.children.begin(), node.children.end());

		Info("Processing node '%s'", node.name.c_str());
		if (node.camera > -1)
		{
			pnode->type = xNode::eType::Camera;
			pnode->value = node.camera;
		}
		else if (node.mesh > -1)
		{
			pnode->type = xNode::eType::Mesh;
			pnode->value = node.mesh;
		}
		else
		{
			for (auto& ext : node.extensions)
			{
				if (ext.first == "KHR_lights_punctual")
				{
					pnode->type = xNode::eType::Light;
					pnode->value = ext.second.GetNumberAsInt();
				}
			}
		}

		setNodeTransform(pnode, node);
	}
}

void GltfLoader::parseLights()
{
	_scene.lights.clear();

	for (uint i = 0; i < _glmodel.lights.size(); ++i)
	{
		const tinygltf::Light& inp = _glmodel.lights[i];
		xLight* light = _scene.lights[_scene.addLight()].get();
		
		if (inp.type == "point")			light->type = xLight::eType::POINT;
		else if (inp.type == "spot")		light->type = xLight::eType::SPOT;
		else if (inp.type == "directional")	light->type = xLight::eType::DIRECTIONAL;

		light->color		= glm::make_vec3(inp.color.data());
		light->intensity	= float(inp.intensity);

		if (light->type == xLight::eType::POINT || light->type == xLight::eType::SPOT)
		{
			light->range = float(inp.range);
		}
		if (light->type == xLight::eType::SPOT)
		{
			light->setInnerConeAngle(inp.spot.innerConeAngle);
			light->setOuterConeAngle(inp.spot.outerConeAngle);
		}
	}
}

void GltfLoader::parseCameras()
{
	_scene.cameras.clear();
	for (uint i = 0; i < _glmodel.cameras.size(); ++i)
	{
		const tinygltf::Camera& inp = _glmodel.cameras[i];
		xCamera* cam = _scene.cameras[_scene.addCamera()].get();

		if (inp.type == "perspective")
		{
			cam->type			= xCamera::eType::PERSPECTIVE;
			cam->aspectRatio	= static_cast<float>(inp.perspective.aspectRatio);
			cam->yfov			= static_cast<float>(inp.perspective.yfov);
			cam->zfar			= static_cast<float>(inp.perspective.zfar);
			cam->znear			= static_cast<float>(inp.perspective.znear);
		}
		else
		{
			cam->type			= xCamera::eType::ORTHOGRAPHIC;
			cam->xmag			= static_cast<float>(inp.orthographic.xmag);
			cam->ymag			= static_cast<float>(inp.orthographic.ymag);
			cam->zfar			= static_cast<float>(inp.orthographic.zfar);
			cam->znear			= static_cast<float>(inp.orthographic.znear);
		}

		cam->update();
	}
}

void GltfLoader::parseMaterials()
{
	_scene.materials.clear();

	for (uint i = 0; i < _glmodel.materials.size(); ++i)
	{
		const tinygltf::Material& inp = _glmodel.materials[i];
		xMaterial* mat = _scene.materials[_scene.addMaterial()].get();
		mat->name = inp.name;

		if (inp.extensions.size())
		{
			for (auto& ext : inp.extensions)
			{
				if (ext.first == "KHR_materials_pbrSpecularGlossiness")
				{
					mat->specularGlossiness = true;
					for (const auto& key : ext.second.Keys())
					{
						auto& val = ext.second.Get(key);
						if (key == "diffuseFactor")
						{
							for (uint j = 0; j < 4; ++j) {
								mat->diffuseFactor[j] = static_cast<float>(val.Get(j).GetNumberAsDouble());
							}
						}
						else if (key == "specularFactor")
						{
							for (uint j = 0; j < 4; ++j) {
								mat->specularFactor[j] = static_cast<float>(val.Get(j).GetNumberAsDouble());
							}
						}
						else if (key == "glossinessFactor")
						{
							mat->glossinessFactor = static_cast<float>(val.GetNumberAsDouble());
						}
						else if (key == "diffuseTexture")
						{
							mat->diffuseTexture = val.Get("index").GetNumberAsInt();
						}
						else if (key == "specularGlossinessTexture")
						{
							mat->specularGlossinessTexture = val.Get("index").GetNumberAsInt();
						}

					}
				}
			}
		} 
		else
		{
			// metallic-roughness
			mat->baseColorFactor			= glm::make_vec4(inp.pbrMetallicRoughness.baseColorFactor.data());
			mat->metallicFactor				= float(inp.pbrMetallicRoughness.metallicFactor);
			mat->roughnessFactor			= float(inp.pbrMetallicRoughness.roughnessFactor);
			mat->baseColorTexture			= inp.pbrMetallicRoughness.baseColorTexture.index;
			mat->metallicRoughnessTexture	= inp.pbrMetallicRoughness.metallicRoughnessTexture.index;
		}

		mat->normalTexture	= inp.normalTexture.index;
		mat->normalScale	= float(inp.normalTexture.scale);
		mat->alphaCutoff	= float(inp.alphaCutoff);
		mat->doubleSided	= inp.doubleSided;

		if (inp.alphaMode == "OPAQUE")		mat->alphaMode = xMaterial::eAlphaMode::OPAQUE;
		else if (inp.alphaMode == "MASK")	mat->alphaMode = xMaterial::eAlphaMode::MASK;
		else if (inp.alphaMode == "BLEND")	mat->alphaMode = xMaterial::eAlphaMode::BLEND;

		mat->occlusionTexture	= inp.occlusionTexture.index;
		mat->occlusionStrength	= float(inp.occlusionTexture.strength);
		mat->emissiveFactor		= glm::make_vec3(inp.emissiveFactor.data());
		mat->emissiveTexture	= inp.emissiveTexture.index;

	}
}

void GltfLoader::parseImages()
{
	_scene.images.clear();

	for (uint i = 0; i < _glmodel.images.size(); ++i)
	{
		tinygltf::Image& inp = _glmodel.images[i];
		Image* img = _scene.images[_scene.addImage()].get();

		img->name		= inp.name;
		img->width		= inp.width;
		img->height		= inp.height;
		img->component	= inp.component;
		img->bits		= inp.bits;

		switch (inp.pixel_type)
		{
		case TINYGLTF_COMPONENT_TYPE_BYTE:
			img->pixelType = eDataType::BYTE;
			break;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
			img->pixelType = eDataType::UNSIGNED_SHORT;
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			img->pixelType = eDataType::UNSIGNED_INT32;
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			img->pixelType = eDataType::FLOAT;
			break;
		}

		img->image = inp.image;
	}
}

void GltfLoader::parseSamplers()
{
	_scene.samplers.clear();

	for (uint i = 0; i < _glmodel.samplers.size(); ++i)
	{
		const tinygltf::Sampler& inp = _glmodel.samplers[i];
		Sampler& sam = _scene.samplers[_scene.addSampler()];

		if (inp.magFilter == -1 || inp.magFilter == 9729) sam.magFilter = eTexMagFilter::LINEAR;
		else if (inp.magFilter == 9728) sam.magFilter = eTexMagFilter::NEAREST;

		if (inp.minFilter == -1 || inp.minFilter == 9729) sam.minFilter = eTexMinFilter::LINEAR;
		else if (inp.minFilter == 9728) sam.minFilter = eTexMinFilter::NEAREST;
		else if (inp.minFilter == 9984) sam.minFilter = eTexMinFilter::NEAREST_MIPMAP_NEAREST;
		else if (inp.minFilter == 9985) sam.minFilter = eTexMinFilter::LINEAR_MIPMAP_NEAREST;
		else if (inp.minFilter == 9986) sam.minFilter = eTexMinFilter::NEAREST_MIPMAP_LINEAR;
		else sam.minFilter = eTexMinFilter::LINEAR_MIPMAP_LINEAR;

		switch (inp.wrapS)
		{
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			sam.wrapS = eTexWrap::CLAMP_TO_EDGE;
			break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			sam.wrapS = eTexWrap::MIRRORED_REPEAT;
			break;
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			sam.wrapS = eTexWrap::REPEAT;
			break;
		}
		switch (inp.wrapT)
		{
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			sam.wrapT = eTexWrap::CLAMP_TO_EDGE;
			break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			sam.wrapT = eTexWrap::MIRRORED_REPEAT;
			break;
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			sam.wrapT = eTexWrap::REPEAT;
			break;
		}
	}
}


void GltfLoader::parseTextures()
{
	_scene.textures.clear();

	for (uint i = 0; i < _glmodel.textures.size(); ++i)
	{
		const tinygltf::Texture& inp = _glmodel.textures[i];
		Texture& tex = _scene.textures[_scene.addTexture()];

		tex.type	= Texture::eType::D2;
		tex.image	= inp.source;
		tex.sampler = inp.sampler;
	}
}

void GltfLoader::parseAnimations()
{
	_scene.animations.clear();

	std::unordered_map<int, int> input_cache;

	for (uint i = 0; i < _glmodel.animations.size(); ++i)
	{
		const tinygltf::Animation& inp = _glmodel.animations[i];
		Animation* anim = _scene.animations[_scene.addAnimation()].get();

		anim->name = inp.name;

		/* channels */
		for (int j = 0; j < inp.channels.size(); ++j)
		{
			Animation::Channel& ch = anim->channels[anim->addChannel()];
			ch.sampler = inp.channels[j].sampler;
			ch.target = inp.channels[j].target_node;

			const std::string path = inp.channels[j].target_path;

			if (path == "rotation") ch.path = Animation::ePath::Rotation;
			else if (path == "translation") ch.path = Animation::ePath::Translation;
			else if (path == "scale") ch.path = Animation::ePath::Scale;
			else ch.path = Animation::ePath::Weights;
		}

		/* samplers */
		for (int j = 0; j < inp.samplers.size(); ++j)
		{
			const tinygltf::AnimationSampler& sinp = inp.samplers[j];

			uint input_idx = 0;
			auto& res = input_cache.find(sinp.input);
			if (res == input_cache.end())
			{
				// cache miss
				input_idx = anim->addInput();
				input_cache[sinp.input] = input_idx;
				arrayToFloatArray(_glmodel.accessors[sinp.input], anim->inputs[input_idx]);
			}
			else
			{
				input_idx = res->second;
			}

			Animation::Sampler* samp = anim->samplers[anim->addSampler()].get();
			samp->input = input_idx;

			const tinygltf::Accessor& access = _glmodel.accessors[sinp.output];
			arrayToFloatArray(access, samp->output);
		}
	}
}

void GltfLoader::parseMeshes()
{
	_scene.meshes.clear();

	for (uint i = 0; i < _glmodel.meshes.size(); ++i)
	{
		const auto& inp = _glmodel.meshes[i];
		Mesh* mesh = _scene.meshes[_scene.addMesh()].get();

		mesh->name = inp.name;
		std::vector<float> temp;

		for (uint j = 0; j < inp.primitives.size(); ++j)
		{
			const auto& pinp = inp.primitives[j];
			const uint pidx = mesh->addPrimitive();
			Primitive* p = mesh->primitives[pidx].get();

			p->material = pinp.material;

			switch (pinp.mode)
			{
			case TINYGLTF_MODE_LINE:
				p->mode = eDrawMode::LINES;
				break;
			case TINYGLTF_MODE_LINE_LOOP:
				p->mode = eDrawMode::LINE_LOOP;
				break;
			case TINYGLTF_MODE_LINE_STRIP:
				p->mode = eDrawMode::LINE_STRIP;
				break;
			case TINYGLTF_MODE_POINTS:
				p->mode = eDrawMode::POINTS;
				break;
			case TINYGLTF_MODE_TRIANGLES:
				p->mode = eDrawMode::TRIANGLES;
				break;
			case TINYGLTF_MODE_TRIANGLE_FAN:
				p->mode = eDrawMode::TRIANGLE_FAN;
				break;
			case TINYGLTF_MODE_TRIANGLE_STRIP:
				p->mode = eDrawMode::TRIANGLE_STRIP;
				break;
			}

			for (auto& attr : pinp.attributes)
			{
				const auto& access = _glmodel.accessors[attr.second];

				temp.clear();
				arrayToFloatArray(access, temp);
				const size_t count = _glmodel.accessors[attr.second].count;

				if (attr.first == "POSITION")
				{
					const glm::vec3* ptr = reinterpret_cast<const glm::vec3*>(temp.data());
					p->positions.assign(ptr, ptr + count);
					p->aabb.merge(glm::make_vec3(access.minValues.data()));
					p->aabb.merge(glm::make_vec3(access.maxValues.data()));
					mesh->aabb.merge(p->aabb);
				}
				else if (attr.first == "NORMAL")
				{
					const glm::vec3* ptr = reinterpret_cast<const glm::vec3*>(temp.data());
					p->normals.assign(ptr, ptr + count);
				}
				else if (attr.first == "TANGENT")
				{
					const glm::vec4* ptr = reinterpret_cast<const glm::vec4*>(temp.data());
					p->tangents.assign(ptr, ptr + count);
				}
				else if (attr.first == "TEXCOORD_0")
				{
					const glm::vec2* ptr = reinterpret_cast<const glm::vec2*>(temp.data());
					p->texcoords0.assign(ptr, ptr + count);
				}
				else if (attr.first == "COLOR_0" && access.type == TINYGLTF_TYPE_VEC4)
				{
					const glm::vec4* ptr = reinterpret_cast<const glm::vec4*>(temp.data());
					p->colors0.assign(ptr, ptr + count);
				}
				else if (attr.first == "COLOR_0" && access.type == TINYGLTF_TYPE_VEC3)
				{
					const glm::vec3* ptr = reinterpret_cast<const glm::vec3*>(temp.data());
					for (size_t k = 0; k < count; ++k)
					{
						p->colors0.push_back(glm::vec4(ptr[k], 1.0f));
					}
				}
			}
			// indices
			const auto& indices = _glmodel.accessors[pinp.indices];
			const auto& iview = _glmodel.bufferViews[indices.bufferView];
			const auto& ibuff = _glmodel.buffers[iview.buffer];
			size_t idx_size = indices.count;

			if (indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				idx_size *= 2;
				p->indexSize = 2;
			}
			else if (indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			{
				idx_size *= 4;
				p->indexSize = 4;
			}

			const uchar* ptr = reinterpret_cast<const uchar*>(ibuff.data.data() + iview.byteOffset + indices.byteOffset);

			p->indices.resize(idx_size, 0);
			std::memcpy(p->indices.data(), ptr, idx_size);
		}
	}
}

void GltfLoader::arrayToFloatArray(const tinygltf::Accessor& access, std::vector<float>& dst)
{
	const tinygltf::BufferView& view = _glmodel.bufferViews[access.bufferView];
	const tinygltf::Buffer& buffer = _glmodel.buffers[view.buffer];

	uint elemCount = 1;
	switch (access.type)
	{
	case TINYGLTF_TYPE_SCALAR:
		elemCount = 1;
		break;
	case TINYGLTF_TYPE_VEC2:
		elemCount = 2;
		break;
	case TINYGLTF_TYPE_VEC3:
		elemCount = 3;
		break;
	case TINYGLTF_TYPE_VEC4:
		elemCount = 4;
		break;
	}

	if (access.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		const float* tmp = reinterpret_cast<const float*>(buffer.data.data() + view.byteOffset + access.byteOffset);
		dst.assign(tmp, tmp + access.count * elemCount);
	}
	else if (access.componentType == TINYGLTF_COMPONENT_TYPE_BYTE)
	{
		const char* tmp = reinterpret_cast<const char*>(buffer.data.data() + view.byteOffset + access.byteOffset);
		for (int k = 0; k < elemCount * access.count; ++k)
		{
			float c = static_cast<float>(*tmp++);
			dst.push_back(glm::max(c / 127.0f, -1.0f));
		}
	}
	else if (access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		const unsigned char* tmp = reinterpret_cast<const unsigned char*>(buffer.data.data() + view.byteOffset + access.byteOffset);
		for (int k = 0; k < elemCount * access.count; ++k)
		{
			float c = static_cast<float>(*tmp++);
			dst.push_back(c / 255.0f);
		}
	}
	else if (access.componentType == TINYGLTF_COMPONENT_TYPE_SHORT)
	{
		const short* tmp = reinterpret_cast<const short*>(buffer.data.data() + view.byteOffset + access.byteOffset);
		for (int k = 0; k < elemCount * access.count; ++k)
		{
			float c = static_cast<float>(*tmp++);
			dst.push_back(glm::max(c / 32767.0f, -1.0f));

		}
	}
	else if (access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		const unsigned short* tmp = reinterpret_cast<const unsigned short*>(buffer.data.data() + view.byteOffset + access.byteOffset);
		for (int k = 0; k < elemCount * access.count; ++k)
		{
			float c = static_cast<float>(*tmp++);
			dst.push_back(c / 65535.0f);
		}
	}
}

bool GltfLoader::load()
{
    //using namespace tinygltf;

    tinygltf::TinyGLTF loader;
    std::string warn, err;
	// try binary first

	if (!loader.LoadBinaryFromFile(&_glmodel, &err, &warn, _filename))
	{
		Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
		// then ascii
		if (!loader.LoadASCIIFromFile(&_glmodel, &err, &warn, _filename))
		{
			Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
			return false;
		}
	}

	Info(" Gltf scene statistics");
	Info("******************************************************************");
	Info("Filename:  %s", _filename.c_str());
	Info("#scene     %d", _glmodel.scenes.size());
	Info("#node      %d", _glmodel.nodes.size());
	Info("#mesh      %d", _glmodel.meshes.size());
	Info("#material  %d", _glmodel.materials.size());
	Info("#texture   %d", _glmodel.textures.size());
	Info("#image     %d", _glmodel.images.size());
	Info("#camera    %d", _glmodel.cameras.size());
	Info("#light     %d", _glmodel.lights.size());
	Info("#skins     %d", _glmodel.skins.size());
	Info("#animations %d", _glmodel.animations.size());
	Info("*****************************************************************");

	_glscene = &_glmodel.scenes[_glmodel.defaultScene];

	for (auto x : _glscene->nodes)
	{
		_scene.root.push_back(x);
	}
	
	parseNodes();
	parseLights();
	parseCameras();
	parseMaterials();
	parseImages();
	parseMeshes();
	parseTextures();
	parseSamplers();
	parseAnimations();

    return true;
}
