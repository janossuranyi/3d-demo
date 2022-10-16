#include "resource_manager.h"
#include "logger.h"

#include <iostream>
#include <filesystem>
#include <utility>

std::unordered_map<std::string, std::string> ResourceManager::resource_map_;
std::set<std::string> ResourceManager::sources_;

bool ResourceManager::add_resource_path(const std::string& path)
{
    if (sources_.count(path) > 0) return false;
    sources_.insert(path);

    auto files = FileSystem::get_directory_entries(path, true);
    for (auto& e : files)
    {
        if (std::filesystem::exists(e))
        {
            auto const abs_path = std::filesystem::absolute(e);
            auto const short_name = std::filesystem::path(abs_path).filename();
            resource_map_.insert(std::make_pair(short_name.string(), abs_path.string()));

            Info("[ResourceManager]: %s - (%s) added", short_name.string().c_str(), abs_path.string().c_str());
        }
    }

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

std::string ResourceManager::get_text_resource_with_includes(const std::string& name)
{
    return get_text_resource(name);
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