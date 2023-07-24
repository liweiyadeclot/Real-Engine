#pragma once

#include "MathHelper.h"
#include "FrameResource.h"
#include "d3dUtil.h"
#include "tiny_gltf.h"

struct textureInfo
{
	textureInfo() = default;

	int width;
	int height;
	int channel;

	unsigned char* data;
};

struct nodeInfo
{
	nodeInfo() = default;
	
	std::string name;
	DirectX::XMFLOAT4X4 world;

	std::vector<Vertex> verticles;
	std::vector<uint16_t> indices;

	textureInfo tex;
	textureInfo pbr;

	bool alphamode = false;
	
};

class Loader
{
public:
	Loader();
	~Loader();
	void loadfromadd(std::string add);

	std::vector<nodeInfo> nodeInfos;
private:
	void SceneParse();
	void NodeParse(int nodeid);
	void materialParse(nodeInfo& nd_info, int matid);
	void indicesParse(nodeInfo& nd_info, const tinygltf::Model& m_model, int indexid);
	void vertexParse(nodeInfo& nd_info, const tinygltf::Model& m_model,
		int poid, int tanid, int noid, int t0id);

	tinygltf::Model m_model;
};

inline int16_t doublebyteswitch(const std::vector<unsigned char>& data, size_t offset) 
{
	int16_t result = 0;
	memcpy(&result, &data[offset], 2);
	return result;
}

inline float quadbyteswitch(const std::vector<unsigned char>& data, size_t offset)
{
	float result = 0;
	memcpy(&result, &data[offset], 4);
	return result;
}