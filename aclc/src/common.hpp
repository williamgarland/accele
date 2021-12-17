#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace acl {
    typedef std::string String;
    typedef std::stringstream StringBuffer;
    
    template <typename T>
    using List = std::vector<T>;

    template <typename K, typename V>
    using Map = std::unordered_map<K, V>;

    struct SourceMeta {
        String file;
        int line;
        int col;
    };
}