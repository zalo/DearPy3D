#pragma once

#include <vector>
#include <string>

// forward declarations
struct mvGLTFNode;
struct mvGLTFScene;
struct mvGLTFMesh;
struct mvGLTFMaterial;
struct mvGLTFTexture;
struct mvGLTFSampler;
struct mvGLTFImage;
struct mvGLTFBuffer;
struct mvGLTFBufferView;
struct mvGLTFAccessor;
struct mvGLTFAttribute;

enum mvGLTFPrimitiveMode
{
	MV_GLTF_POINTS    = 0,
	MV_GLTF_LINES     = 1,
	MV_GLTF_TRIANGLES = 4
};

enum mvGLTFAttributeType
{
	MV_GLTF_POSITION = 0,
	MV_GLTF_TANGENT,
	MV_GLTF_NORMAL,
	MV_GLTF_TEXTCOORD,
};

enum mvGLTFAccessorType
{
	MV_GLTF_SCALAR,
	MV_GLTF_VEC2,
	MV_GLTF_VEC3,
	MV_GLTF_VEC4,
};

enum mvGLTFComponentType
{
	MV_GLTF_BYTE           = 0x1400,
	MV_GLTF_UNSIGNED_BYTE  = 0x1401,
	MV_GLTF_SHORT          = 0x1402,
	MV_GLTF_UNSIGNED_SHORT = 0x1403,
	MV_GLTF_INT            = 0x1404,
	MV_GLTF_UNSIGNED_INT   = 0x1405,
	MV_GLTF_FLOAT          = 0x1406,
	MV_GLTF_DOUBLE         = 0x140A
};

struct mvGLTFAttribute
{
	mvGLTFAttributeType type;
	int                 index = -1; // accessor index
};

struct mvGLTFAccessor
{
	std::string         name;
	mvGLTFAccessorType  type = MV_GLTF_SCALAR;
	int                 buffer_view_index = -1;
	mvGLTFComponentType component_type = MV_GLTF_FLOAT;
	int                 count = -1;
	std::vector<float>  maxes;
	std::vector<float>  mins;
};

struct mvGLTFTexture
{
	std::string name;
	int         image_index = -1;
	int         sampler_index = -1;
};

struct mvGLTFSampler
{
	int mag_filter = -1;
	int min_filter = -1;
	int wrap_s = -1;
	int wrap_t = -1;
};

struct mvGLTFImage
{
	std::string uri;
};

struct mvGLTFBuffer
{
	int                       byte_length = -1;
	std::string               uri;
	std::vector<std::uint8_t> data;
};

struct mvGLTFBufferView
{
	std::string name;
	int         buffer_index = -1;
	int         byte_offset = 0;
	int         byte_length = -1;
	int         byte_stride = -1;
};

struct mvGLTFMesh
{
	std::string                  name;
	int                          indices_index = -1; // accessor index
	int                          material_index = -1;
	mvGLTFPrimitiveMode          mode = MV_GLTF_TRIANGLES;
	std::vector<mvGLTFAttribute> attributes;
};

struct mvGLTFMaterial
{
	std::string name;
	int         base_color_texture = -1;
	int         metallic_roughness_texture = -1;
	int         normal_texture = -1;
	int         occlusion_texture = -1;
	int         emissive_texture = -1;
	float       normal_texture_scale = 0.8f;
	float       occlusion_texture_strength = 0.9f;
	float       metallic_factor = 1.0f;
	float       roughness_factor = 0.0f;
	float       base_color_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float       emissive_factor[3] = { 0.0f, 0.0f, 0.0f };
	bool        double_sided = false;
};

struct mvGLTFNode
{
	std::string      name;
	int              mesh_index = -1;
	std::vector<int> children;
};

struct mvGLTFScene
{
	std::vector<int> nodes;
};

struct mvGLTFModel
{
	std::string                   rootDirectory;
	std::vector<mvGLTFScene>      scenes;
	std::vector<mvGLTFNode>       nodes;
	std::vector<mvGLTFMesh>       meshes;
	std::vector<mvGLTFMaterial>   materials;
	std::vector<mvGLTFTexture>    textures;
	std::vector<mvGLTFSampler>    samplers;
	std::vector<mvGLTFImage>      images;
	std::vector<mvGLTFBuffer>     buffers;
	std::vector<mvGLTFBufferView> bufferviews;
	std::vector<mvGLTFAccessor>   accessors;
};

mvGLTFModel mvLoadGLTF(const std::string& file);