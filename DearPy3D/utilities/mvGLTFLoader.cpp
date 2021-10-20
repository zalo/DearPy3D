#include "mvGLTFLoader.h"
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <iterator>
#include <filesystem>
#include <assert.h>
#include "mvContext.h"

mv_internal std::vector<mvGLTFScene>
mvLoadScenes(nlohmann::json j)
{
	std::vector<mvGLTFScene> scenes;
	if (!j.contains("scenes"))
		return scenes;

	for (auto& jscene : j["scenes"])
	{
		mvGLTFScene scene{};

		if (jscene.contains("nodes"))
		{
			for (auto& node : jscene["nodes"])
				scene.nodes.push_back(node);
		}

		scenes.push_back(scene);
	}

	return scenes;
}

mv_internal std::vector<mvGLTFNode>
mvLoadNodes(nlohmann::json j)
{
	std::vector<mvGLTFNode> nodes;
	if (!j.contains("nodes"))
		return nodes;

	for (auto& jnode : j["nodes"])
	{
		mvGLTFNode node{};

		if (jnode.contains("name"))
			node.name = jnode["name"];

		if (jnode.contains("mesh"))
			node.mesh_index = jnode["mesh"];

		if (jnode.contains("children"))
		{
			for (auto& child : jnode["children"])
				node.children.push_back(child);
		}

		nodes.push_back(node);
	}

	return nodes;
}

mv_internal std::vector<mvGLTFMesh>
mvLoadMeshes(nlohmann::json j)
{
	std::vector<mvGLTFMesh> meshes;
	if (!j.contains("meshes"))
		return meshes;

	for (auto& jmesh : j["meshes"])
	{
		mvGLTFMesh mesh{};

		if (jmesh.contains("name"))
			mesh.name = jmesh["name"];

		if (jmesh.contains("primitives"))
		{
			nlohmann::json jprimitive = jmesh["primitives"][0];

			if (jprimitive.contains("indices"))
				mesh.indices_index = jprimitive["indices"];

			if (jprimitive.contains("material"))
				mesh.material_index = jprimitive["material"];

			if (jprimitive.contains("attributes"))
			{
				if (jprimitive["attributes"].contains("POSITION"))
					mesh.attributes.push_back({ MV_GLTF_POSITION , jprimitive["attributes"]["POSITION"] });

				if (j["attributes"].contains("TANGENT"))
					mesh.attributes.push_back({ MV_GLTF_TANGENT , jprimitive["attributes"]["TANGENT"] });

				if (j["attributes"].contains("NORMAL"))
					mesh.attributes.push_back({ MV_GLTF_NORMAL , jprimitive["attributes"]["NORMAL"] });

				if (j["attributes"].contains("TEXCOORD_0"))
					mesh.attributes.push_back({ MV_GLTF_TEXTCOORD , jprimitive["attributes"]["TEXCOORD_0"] });
			}
		}

		meshes.push_back(mesh);
	}

	return meshes;
}

mv_internal std::vector<mvGLTFMaterial>
mvLoadMaterials(nlohmann::json j)
{
	std::vector<mvGLTFMaterial> materials;
	if (!j.contains("materials"))
		return materials;

	for (auto& jmaterial : j["materials"])
	{
		mvGLTFMaterial material{};

		if (j.contains("name"))
			material.name = jmaterial["name"];

		if (jmaterial.contains("pbrMetallicRoughness"))
		{
			if (jmaterial["pbrMetallicRoughness"].contains("baseColorTexture"))
			{
				if (jmaterial["pbrMetallicRoughness"]["baseColorTexture"].contains("index"))
					material.base_color_texture = jmaterial["pbrMetallicRoughness"]["baseColorTexture"]["index"];
			}

			if (jmaterial["pbrMetallicRoughness"].contains("baseColorFactor"))
			{
				material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0];
				material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1];
				material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2];
				material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3];
			}

			if (jmaterial["pbrMetallicRoughness"].contains("metallicRoughnessTexture"))
			{
				if (jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].contains("index"))
					material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"];
			}

			if (jmaterial["pbrMetallicRoughness"].contains("metallicFactor"))
				material.metallic_factor = jmaterial["pbrMetallicRoughness"]["metallicFactor"];

			if (jmaterial["pbrMetallicRoughness"].contains("roughnessFactor"))
				material.roughness_factor = jmaterial["pbrMetallicRoughness"]["roughnessFactor"];
		}

		if (jmaterial.contains("normalTexture"))
		{
			if (jmaterial["normalTexture"].contains("index"))
				material.normal_texture = jmaterial["normalTexture"]["index"];

			if (jmaterial["normalTexture"].contains("scale"))
				material.normal_texture_scale = jmaterial["normalTexture"]["scale"];
		}

		if (jmaterial.contains("occlusionTexture"))
		{
			if (jmaterial["occlusionTexture"].contains("index"))
				material.occlusion_texture = jmaterial["occlusionTexture"]["index"];

			if (jmaterial["occlusionTexture"].contains("scale"))
				material.occlusion_texture_strength = jmaterial["occlusionTexture"]["strength"];
		}

		if (j.contains("emissiveTexture"))
		{
			if (j["emissiveTexture"].contains("index"))
				material.emissive_texture = jmaterial["emissiveTexture"]["index"];
		}

		if (j.contains("emissiveFactor"))
		{

			material.emissive_factor[0] = jmaterial["emissiveFactor"][0];
			material.emissive_factor[1] = jmaterial["emissiveFactor"][1];
			material.emissive_factor[2] = jmaterial["emissiveFactor"][2];
		}

		if (j.contains("doubleSided"))
			material.double_sided = jmaterial["doubleSided"];

		materials.push_back(material);
	}

	return materials;
}

mv_internal std::vector<mvGLTFTexture>
mvLoadTextures(nlohmann::json j)
{
	std::vector<mvGLTFTexture> textures;
	if (!j.contains("textures"))
		return textures;

	for (auto& jtexture : j["textures"])
	{
		mvGLTFTexture texture{};

		if (jtexture.contains("sampler"))
			texture.sampler_index = jtexture["sampler"];

		if (jtexture.contains("source"))
			texture.image_index = jtexture["source"];

		if (jtexture.contains("name"))
			texture.name = jtexture["name"];

		textures.push_back(texture);
	}

	return textures;
}

mv_internal std::vector<mvGLTFSampler>
mvLoadSamplers(nlohmann::json j)
{
	std::vector<mvGLTFSampler> samplers;
	if (!j.contains("samplers"))
		return samplers;

	for (auto& jsampler : j["samplers"])
	{
		mvGLTFSampler sampler{};

		if (jsampler.contains("magFilter"))
			sampler.mag_filter = jsampler["magFilter"];

		if (jsampler.contains("minFilter"))
			sampler.min_filter = jsampler["minFilter"];

		if (jsampler.contains("wrapS"))
			sampler.wrap_s = j["wrapS"];

		if (jsampler.contains("wrapT"))
			sampler.wrap_t = jsampler["wrapT"];

		samplers.push_back(sampler);
	}

	return samplers;
}

mv_internal std::vector<mvGLTFImage>
mvLoadImages(nlohmann::json j)
{
	std::vector<mvGLTFImage> images;
	if (!j.contains("images"))
		return images;

	for (auto& jimage : j["images"])
	{
		mvGLTFImage image{};

		if (jimage.contains("uri"))
			image.uri = jimage["uri"];

		images.push_back(image);
	}

	return images;
}

mv_internal std::vector<mvGLTFBuffer>
mvLoadBuffers(nlohmann::json j)
{
	std::vector<mvGLTFBuffer> buffers;
	if (!j.contains("buffers"))
		return buffers;

	for (auto& jbuffer : j["buffers"])
	{
		mvGLTFBuffer buffer{};

		if (jbuffer.contains("uri"))
			buffer.uri = jbuffer["uri"];

		if (jbuffer.contains("byteLength"))
			buffer.byte_length = jbuffer["byteLength"];

		buffers.push_back(buffer);
	}

	return buffers;
}

mv_internal std::vector<mvGLTFBufferView>
mvLoadBufferViews(nlohmann::json j)
{
	std::vector<mvGLTFBufferView> bufferviews;
	if (!j.contains("bufferViews"))
		return bufferviews;

	for (auto& jbufferview : j["buffers"])
	{
		mvGLTFBufferView bufferview{};

		if (jbufferview.contains("name"))
			bufferview.name = jbufferview["name"];

		if (jbufferview.contains("buffer"))
			bufferview.buffer_index = jbufferview["buffer"];

		if (jbufferview.contains("byteOffset"))
			bufferview.byte_offset = jbufferview["byteOffset"];

		if (jbufferview.contains("byteLength"))
			bufferview.byte_length = jbufferview["byteLength"];

		if (jbufferview.contains("byteStride"))
			bufferview.byte_stride = jbufferview["byteStride"];

		bufferviews.push_back(bufferview);
	}

	return bufferviews;
}

mv_internal std::vector<mvGLTFAccessor>
mvLoadAccessors(nlohmann::json j)
{
	std::vector<mvGLTFAccessor> accessors;
	if (!j.contains("accessors"))
		return accessors;

	for (auto& jaccessor : j["accessors"])
	{
		mvGLTFAccessor accessor{};

		if (jaccessor.contains("name"))
			accessor.name = jaccessor["name"];

		if (jaccessor.contains("count"))
			accessor.count = jaccessor["count"];

		if (jaccessor.contains("componentType"))
			accessor.component_type = jaccessor["componentType"];

		if (jaccessor.contains("bufferView"))
			accessor.buffer_view_index = jaccessor["bufferView"];

		if (jaccessor.contains("type"))
		{
			std::string strtype = jaccessor["type"];
			if (strtype == "SCALAR")
				accessor.type = MV_GLTF_SCALAR;

			else if (strtype == "VEC2")
				accessor.type = MV_GLTF_VEC2;

			else if (strtype == "VEC3")
				accessor.type = MV_GLTF_VEC3;

			else if (strtype == "VEC4")
				accessor.type = MV_GLTF_VEC4;
		}

		if (jaccessor.contains("max"))
		{
			for (auto& item : jaccessor["max"])
				accessor.maxes.push_back(item);
		}

		if (jaccessor.contains("min"))
		{
			for (auto& item : jaccessor["min"])
				accessor.mins.push_back(item);
		}

		accessors.push_back(accessor);
	}

	return accessors;
}

mvGLTFModel
mvLoadGLTF(const std::string& file)
{

	mvGLTFModel model{};

	std::filesystem::path path = file;
	model.rootDirectory = path.parent_path().string() + "\\";

	if (!std::filesystem::exists(path))
	{
		assert(false && "File not found.");
		return model;
	}

	std::ifstream inputStream(file);
	nlohmann::json j;
	inputStream >> j;
	
	model.scenes = mvLoadScenes(j);
	model.nodes = mvLoadNodes(j);
	model.materials = mvLoadMaterials(j);
	model.meshes = mvLoadMeshes(j);
	model.textures = mvLoadTextures(j);
	model.samplers = mvLoadSamplers(j);
	model.images = mvLoadImages(j);
	model.buffers = mvLoadBuffers(j);
	model.bufferviews = mvLoadBufferViews(j);
	model.accessors = mvLoadAccessors(j);

	for (auto& buffer : model.buffers)
	{
		std::ifstream inputStream(model.rootDirectory + buffer.uri, std::ios::binary);

		if (inputStream)
		{
			std::vector<std::uint8_t> ibuffer(std::istreambuf_iterator<char>(inputStream), {});
			buffer.data = ibuffer;
		}
		
		else
			buffer.data = {};
	}

	return model;
}