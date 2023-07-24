#pragma once
#include "../include/GLTFloader.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../include/tiny_gltf.h"

#define pngchannel 4;
Loader::Loader()
{
}

Loader::~Loader()
{
}

void Loader::loadfromadd(std::string add)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    //load model to m_model
    bool ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, add);
    assert(warn.empty());
    assert(err.empty());
    assert(ret);
    SceneParse();
}

void Loader::SceneParse()
{
    assert(!m_model.scenes.empty());
    int sceneid = m_model.defaultScene;
    for (int id : m_model.scenes[sceneid].nodes) {
        NodeParse(id);
    }
}

void Loader::NodeParse(int nodeid) 
{
    tinygltf::Node& ld_node= m_model.nodes[nodeid];
    nodeInfo nd_info;

    //cameraParse
    if (ld_node.camera != -1) {
        //setcamera
    }

    //name
    if (!ld_node.name.empty()) {
        nd_info.name = ld_node.name;
    }

    //world
    DirectX::XMFLOAT4X4 wd = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4* wdptr = &wd;
    DirectX::XMMATRIX world= DirectX::XMLoadFloat4x4(wdptr);

    if (!ld_node.matrix.empty()) {
        world = DirectX::XMMatrixSet(
            ld_node.matrix[0], ld_node.matrix[1], ld_node.matrix[2], ld_node.matrix[3],
            ld_node.matrix[4], ld_node.matrix[5], ld_node.matrix[6], ld_node.matrix[7],
            ld_node.matrix[8], ld_node.matrix[9], ld_node.matrix[10], ld_node.matrix[11],
            ld_node.matrix[12], ld_node.matrix[13], ld_node.matrix[14], ld_node.matrix[15]);
    }
    DirectX::XMStoreFloat4x4(&nd_info.world, world);

    //meshParse

    int meshid = ld_node.mesh;
    tinygltf::Mesh& ld_mesh = m_model.meshes[meshid];

    //name2
    if (!ld_mesh.name.empty()) {
        nd_info.name = ld_mesh.name;
    }

    tinygltf::Primitive& ld_pri = ld_mesh.primitives[0];

    //matParse
    if (ld_pri.material!=-1)
    {
        int matid = ld_pri.material;
        materialParse(nd_info, matid);
    }

    //index
    if (ld_pri.indices != -1) {
        int indexid = ld_pri.indices;
        indicesParse(nd_info,m_model,indexid);
    }

    //geometry
    std::map<std::string, int>& ld_attributes = ld_pri.attributes;

    int poid = ld_attributes["POSITION"];
    int tanid = ld_attributes["TANGENT"];
    int noid = ld_attributes["NORMAL"];
    int t0id = ld_attributes["TEXCOORD_0"];

    vertexParse(nd_info, m_model, poid, tanid, noid, t0id);

    nodeInfos.push_back(nd_info);
}

void Loader::indicesParse(nodeInfo& nd_info, const tinygltf::Model& m_model, int indexid) 
{
    int bfvid = m_model.accessors[indexid].bufferView;
    int bfid = m_model.bufferViews[bfvid].buffer;
    size_t count = m_model.accessors[indexid].count;

    const std::vector<unsigned char>& dt = m_model.buffers[bfid].data;

    size_t btoffset = 0;
    btoffset += m_model.accessors[indexid].byteOffset;
    btoffset += m_model.bufferViews[bfvid].byteOffset; 
    
    for (size_t i = 0; i < count; ++i) {
        nd_info.indices.push_back(doublebyteswitch(dt, btoffset + 2 * i));
    }       
}

void Loader::vertexParse(nodeInfo& nd_info,const tinygltf::Model& m_model,
    int poid,int tanid, int noid, int t0id)
{
    size_t count = m_model.accessors[poid].count;
    const std::vector<unsigned char>& buffer = m_model.buffers[0].data;
    //position
    int po_bfvid = m_model.accessors[poid].bufferView;
    size_t po_offset = 0;
    po_offset += m_model.accessors[poid].byteOffset;
    po_offset += m_model.bufferViews[po_bfvid].byteOffset;
    //tangent
    int tan_bfvid = m_model.accessors[tanid].bufferView;
    size_t tan_offset = 0;
    tan_offset += m_model.accessors[tanid].byteOffset;
    tan_offset += m_model.bufferViews[tan_bfvid].byteOffset;
    //normal
    int no_bfvid = m_model.accessors[noid].bufferView;
    size_t no_offset = 0;
    no_offset += m_model.accessors[noid].byteOffset;
    no_offset += m_model.bufferViews[no_bfvid].byteOffset;
    //texcoord_0
    int t0_bfvid = m_model.accessors[t0id].bufferView;
    size_t t0_offset = 0;
    t0_offset += m_model.accessors[t0id].byteOffset;
    t0_offset += m_model.bufferViews[t0_bfvid].byteOffset;

    for (size_t i = 0; i < count; ++i) {

        float x = quadbyteswitch(buffer, po_offset + i * 12);
        float y = quadbyteswitch(buffer, po_offset + 4 + i * 12);
        float z = quadbyteswitch(buffer, po_offset + 8 + i * 12);

        float a = quadbyteswitch(buffer, tan_offset + i * 16);
        float b = quadbyteswitch(buffer, tan_offset + 4 + i * 16);
        float c = quadbyteswitch(buffer, tan_offset + 8 + i * 16);
        float d = quadbyteswitch(buffer, tan_offset + 12 + i * 16);

        float nx = quadbyteswitch(buffer, no_offset + i * 12);
        float ny = quadbyteswitch(buffer, no_offset + 4 + i * 12);
        float nz = quadbyteswitch(buffer, no_offset + 8 + i * 12);

        float u = quadbyteswitch(buffer, t0_offset + i * 8);
        float v = quadbyteswitch(buffer, t0_offset + 4 + i * 8);
         
        nd_info.verticles.push_back(Vertex(
            x, y, z, 
            a, b, c, d,
            nx, ny, nz,
            u, v));
    }
}

void Loader::materialParse(nodeInfo& nd_info, int matid)
{
    tinygltf::Material& ld_mat = m_model.materials[matid];
    //basecolor
    int basecolortexid = ld_mat.values["baseColorTexture"].json_double_value["index"];
    int basecolorimgid = m_model.textures[basecolortexid].source;

    nd_info.tex.height = m_model.images[basecolorimgid].height;
    nd_info.tex.width = m_model.images[basecolorimgid].width;
    nd_info.tex.channel = pngchannel;
    nd_info.tex.data = &(m_model.images[basecolorimgid].image[0]);

    //alphamode
    if (!(ld_mat.additionalValues.find("alphaMode") == ld_mat.additionalValues.end())) {
        nd_info.alphamode = true;
    }

    //pbr
    int pbrtexid = ld_mat.values["metallicRoughnessTexture"].json_double_value["index"];
    int pbrimgid = m_model.textures[pbrtexid].source;

    nd_info.pbr.height = m_model.images[pbrtexid].height;
    nd_info.pbr.width = m_model.images[pbrtexid].width;
    nd_info.pbr.channel = pngchannel;
    nd_info.pbr.data = &(m_model.images[pbrtexid].image[0]);
}
