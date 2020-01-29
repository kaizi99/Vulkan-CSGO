//
// Created by kaizi99 on 1/25/20.
//

#include "bsp_loader.h"

#include <fstream>
#include <iostream>

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
#define HEADER_LUMPS 64

#pragma pack(push, 1)
struct lump_t
{
    int fileoffset;
    int filelength;
    int version;
    char fourCC[4];
};

struct dheader_t
{
    int ident;
    int version;
    lump_t lumps[HEADER_LUMPS];
    int mapRevision;
};

struct dface_t
{
    unsigned short	planenum;		// the plane number
    unsigned char		side;			// faces opposite to the node's plane direction
    unsigned char		onNode;			// 1 of on node, 0 if in leaf
    int		firstedge;		// index into surfedges
    short		numedges;		// number of surfedges
    short		texinfo;		// texture info
    short		dispinfo;		// displacement info
    short		surfaceFogVolumeID;	// ?
    unsigned char		styles[4];		// switchable lighting info
    int		lightofs;		// offset into lightmap lump
    float		area;			// face area in units^2
    int		LightmapTextureMinsInLuxels[2];	// texture lighting info
    int		LightmapTextureSizeInLuxels[2];	// texture lighting info
    int		origFace;		// original face this was split from
    unsigned short	numPrims;		// primitives
    unsigned short	firstPrimID;
    unsigned int	smoothingGroups;	// lightmap smoothing group
};
#pragma pack(pop)

bsp_parsed* load_bsp(const std::string& file) {
    std::ifstream fs(file, std::ios::binary);

    if (!fs.is_open()) {
        std::cout << "Could not open " << file << std::endl;
        return nullptr;
    }

    dheader_t bspheader;
    fs.read((char*)&bspheader, sizeof(bspheader));

    if (bspheader.ident != IDBSPHEADER || bspheader.version != 21) {
        std::cout << file << " is not a valid CS:GO map!" << std::endl;
        return nullptr;
    }

    // Read vertices
    lump_t vertexLump = bspheader.lumps[3];

    vertex* vertices = new vertex[vertexLump.filelength / 12];

    fs.seekg(0, std::ios::beg);
    fs.seekg(vertexLump.fileoffset);
    fs.read((char*)vertices, vertexLump.filelength);

    // Read edges
    lump_t edgesLump = bspheader.lumps[12];

    edge* edges = new edge[edgesLump.filelength / sizeof(edge)];

    fs.seekg(edgesLump.fileoffset, std::ios::beg);
    fs.read((char*)edges, edgesLump.filelength);

    // Read surfedges
    lump_t surfedgeLump = bspheader.lumps[13];

    int* surfedges = new int[surfedgeLump.filelength / sizeof(int)];

    fs.seekg(surfedgeLump.fileoffset, std::ios::beg);
    fs.read((char*)surfedges, surfedgeLump.filelength);

    // Read Faces
    lump_t facesLump = bspheader.lumps[7];

    dface_t* lfaces = new dface_t[facesLump.filelength / sizeof(dface_t)];
    face* faces = new face[facesLump.filelength / sizeof(dface_t)];

    fs.seekg(0, std::ios::beg);
    fs.seekg(facesLump.fileoffset);
    fs.read((char*)lfaces, facesLump.filelength);

    for (int i = 0; i < facesLump.filelength / sizeof(dface_t); i++) {
        faces[i].edgeCount = lfaces[i].numedges;
        faces[i].firstSurfedgeIndex = lfaces[i].firstedge;
    }

    delete[] lfaces;

    bsp_parsed* returnStruct = new bsp_parsed();
    returnStruct->vertices = vertices;
    returnStruct->verticesCount = vertexLump.filelength / 12;
    returnStruct->edges = edges;
    returnStruct->edgeCount = surfedgeLump.filelength / 4;
    returnStruct->surfedges = surfedges;
    returnStruct->surfedgeCount = surfedgeLump.filelength / 4;
    returnStruct->faces = faces;
    returnStruct->faceCount = facesLump.filelength / sizeof(dface_t);

    return returnStruct;
}

bsp_geometry_vulkan create_geometry_from_bsp(vulkan_renderer* renderer, bsp_parsed* bsp) {
    return {};
}