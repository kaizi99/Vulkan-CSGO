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

#include "vpk.h"

#include <fstream>
#include <iostream>

#pragma pack(push, 1)
struct VPKHeader_v2
{
    unsigned int Signature;
    unsigned int Version;

    // The size, in bytes, of the directory tree
    unsigned int TreeSize;

    // How many bytes of file content are stored in this VPK file (0 in CSGO)
    unsigned int FileDataSectionSize;

    // The size, in bytes, of the section containing MD5 checksums for external archive content
    unsigned int ArchiveMD5SectionSize;

    // The size, in bytes, of the section containing MD5 checksums for content in this file (should always be 48)
    unsigned int OtherMD5SectionSize;

    // The size, in bytes, of the section containing the public key and signature. This is either 0 (CSGO & The Ship) or 296 (HL2, HL2:DM, HL2:EP1, HL2:EP2, HL2:LC, TF2, DOD:S & CS:S)
    unsigned int SignatureSectionSize;
};

struct VPKDirectoryEntry
{
    unsigned int CRC; // A 32bit CRC of the file's data.
    unsigned short PreloadBytes; // The number of bytes contained in the index file.

    // A zero based index of the archive this file's data is contained in.
    // If 0x7fff, the data follows the directory.
    unsigned short ArchiveIndex;

    // If ArchiveIndex is 0x7fff, the offset of the file data relative to the end of the directory (see the header for more details).
    // Otherwise, the offset of the data from the start of the specified archive.
    unsigned int EntryOffset;

    // If zero, the entire file is stored in the preload data.
    // Otherwise, the number of bytes stored starting at EntryOffset.
    unsigned int EntryLength;

    const unsigned short Terminator = 0xffff;
};
#pragma pack(pop)

std::string vpkdir_readstring(char* tree, unsigned int* p) {
    std::string str = tree + *p;
    *p += str.length() + 1;
    return str;
}

vpk_directory* load_vpk(std::string folder, std::string packname) {
    vpk_directory* dir = new vpk_directory();

    std::ifstream fs(folder + packname + "_dir.vpk", std::ios::binary);

    if (!fs.is_open()) {
        throw std::runtime_error("cannot read vpk file " + folder + packname + "_dir.vpk");
    }

    VPKHeader_v2 header;
    fs.read((char*)&header, sizeof(VPKHeader_v2));

    if (header.Signature != 0x55aa1234 || header.Version != 2) {
        throw std::runtime_error(folder + packname + "_dir.vpk" + " is not a valid vpk v2 directory!");
    }

    char* tree = new char[header.TreeSize];
    unsigned int read = 0;

    fs.seekg(0, std::ios::end);
    int size = fs.tellg();
    int headersize = sizeof(header);
    fs.seekg(sizeof(header), std::ios::beg);

    fs.read(tree, header.TreeSize);
    
    unsigned int p = 0;

    while (true) {
        std::string extension = vpkdir_readstring(tree, &p);
        
        if (extension.empty()) {
            break;
        }

        std::unordered_map<std::string, vpk_directory_entry>& entries = dir->entries[extension];

        while (true) {
            std::string path = vpkdir_readstring(tree, &p);

            if (path.empty()) {
                break;
            }

            while (true) {
                std::string filename = vpkdir_readstring(tree, &p);

                if (filename.empty()) {
                    break;
                }

                VPKDirectoryEntry entry;
                memcpy(&entry, tree + p, sizeof(entry));
                p += sizeof(entry);

                vpk_directory_entry centry = {};
                centry.filename = filename;
                centry.path = path;
                centry.extension = extension;

                if (entry.PreloadBytes != 0) {
                    centry.preload.reserve(entry.PreloadBytes);
                    memcpy(centry.preload.data(), tree + p, entry.PreloadBytes);
                    p += entry.PreloadBytes;
                }
                
                centry.archiveIndex = entry.ArchiveIndex;
                centry.archiveLength = entry.EntryLength;
                centry.archiveOffset = entry.EntryOffset;

                entries[path + "/" + filename] = centry;
            }
        }
    }

    delete[] tree;

    return dir;
}