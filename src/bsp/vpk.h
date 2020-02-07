// Copyright (C) 2020 Kai-Uwe Zimdars
/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef VULKAN_TEST_VPK_H
#define VULKAN_TEST_VPK_H

#include <vector>
#include <string>
#include <unordered_map>

struct vpk_directory_entry {
    std::string extension;
    std::string path;
    std::string filename;
    std::vector<unsigned char> preload;
    short archiveIndex;
    int archiveOffset;
    int archiveLength;
};

struct vpk_directory {
    std::string pakname;
    //std::unordered_map < std::string, std::unordered_map<std::string, std::unordered_map<std::string, vpk_directory_entry>>> entries;
    std::unordered_map < std::string, std::unordered_map < std::string, vpk_directory_entry >> entries;
};

vpk_directory* load_vpk(std::string folder, std::string packname);

#endif //VULKAN_TEST_VPK_H
