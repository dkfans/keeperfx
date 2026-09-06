#ifndef KFX_FILEFIND_H
#define KFX_FILEFIND_H

#include <string>
#include <utility>
#include <vector>

struct TbFileFind {
    std::vector<std::pair<std::string, std::string>> names;
    size_t index = 0;
};

#endif // KFX_FILEFIND_H
