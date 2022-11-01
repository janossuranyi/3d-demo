#include "resource_manager.h"
#include "logger.h"


#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <functional>
#include <array>
#include <regex>

namespace rc {

    std::unordered_map<std::string, std::string> ResourceManager::resource_map_;
    std::set<std::string> ResourceManager::sources_;

    namespace fs = std::filesystem;

    static void dir_traversal(const fs::path& start, std::function<void(const fs::path&)> fn)
    {
        auto it = fs::directory_iterator{ start };
        for (auto const& e : it)
        {
            if (e.is_regular_file())
            {
                fn(e);
            }
            else if(e.is_directory())
            {
                dir_traversal(e, fn);
            }
        }
    }

    bool ResourceManager::add_resource_path(const std::string& path)
    {
        const fs::path src = fs::absolute(fs::path(path));
        const fs::path rel = src.filename();
        const auto src_len = src.generic_string().length();

        if (!fs::exists(src)) return false;

        if (sources_.count(src.generic_string()) > 0) return false;
        sources_.insert(src.generic_string());
        
        dir_traversal(src, [&](const fs::path& elem) {
            auto subs = fs::path(elem.generic_string().substr(src_len));
            const std::string key = rel.generic_string() + subs.generic_string();
            const std::string val = elem.generic_string();
            resource_map_.insert(std::make_pair(key, val));
            Info("%s --> %s", key.c_str(), val.c_str());
        });

        Info("[ResourceManager]: dir %s added", src.generic_string().c_str());
        auto it = src.begin();
        

        return true;
    }

    std::string ResourceManager::get_text_resource(const std::string& name)
    {
        if (resource_map_.count(name) == 0) return "";

        auto const& fn = resource_map_.at(name);

        std::string result;
        FileSystem::read_text_file(fn, result);

        return result;
    }

    std::string ResourceManager::get_text_resource_with_includes(const std::string& name, int depth)
    {
        static const std::regex include_rx("^#include[ ]*\"(.+)\"");
        std::string output;

        if (depth < 0 || resource_map_.count(name) == 0)
        {
            return output;
        }

        auto const& fileName = resource_map_.at(name);
        auto const fn = fs::path(name).filename().generic_string();
 
        auto const dir = name.substr(0, name.find(fn));


        std::ifstream ifs(fileName, std::ios::in);
        std::string line;
        if (ifs.is_open())
        {
            while (std::getline(ifs, line))
            {                
                std::smatch m;
                if (std::regex_search(line, m, include_rx))
                {
                    const auto realname = dir + m[1].str();
                    if (realname != name) {
                        output += get_text_resource_with_includes(realname, depth - 1) + "\n";
                    }
                }
                else
                {
                    output += line + "\n";
                }
            }
            ifs.close();
        }

        return output;
    }

    std::vector<unsigned char> ResourceManager::get_binary_resource(const std::string& name)
    {
        if (resource_map_.count(name) == 0)
            return std::vector<unsigned char>{};

        auto const& fn = resource_map_.at(name);

        return FileSystem::read_binary_file(fn);
    }

    std::string ResourceManager::get_resource(const std::string& name)
    {
        if (resource_map_.count(name) == 0) return "";

        return resource_map_.at(name);
    }

}