//
// Created by Kai-Uwe Zimdars on 30.01.20.
//

#ifndef VULKAN_TEST_VPK_H
#define VULKAN_TEST_VPK_H

#include <vector>
#include <string>
#include <unordered_map>

struct vpk_directory_entry {
    std::string filename;
    std::vector<unsigned char> preload;
    short archiveIndex;
    int archiveOffset;
    int archiveLength;
};

struct vpk_directory {
    std::string pakname;
    std::unordered_map<std::string, vpk_directory_entry> entries;
};

vpk_directory* load_vpk(std::string folder, std::string packname);

#endif //VULKAN_TEST_VPK_H
