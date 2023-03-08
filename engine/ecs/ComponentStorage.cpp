#include "ComponentStorage.h"
#include <cstdio>

namespace nebula {

bool serializeBinary(const ComponentStorageHeader& header, const std::vector<u8>& data, const std::string& path) {
    FILE* file = nullptr;
    fopen_s(&file, path.c_str(), "wb");
    if (!file) return false;

    fwrite(&header, sizeof(header), 1, file);
    fwrite(data.data(), data.size(), 1, file);
    fclose(file);
    return true;
}

bool deserializeBinary(const std::string& path, ComponentStorageHeader& header, std::vector<u8>& data) {
    FILE* file = nullptr;
    fopen_s(&file, path.c_str(), "rb");
    if (!file) return false;

    fread(&header, sizeof(header), 1, file);
    data.resize(header.dataSize);
    fread(data.data(), data.size(), 1, file);
    fclose(file);
    return true;
}

}
