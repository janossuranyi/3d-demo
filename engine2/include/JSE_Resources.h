#ifndef JSE_RESOURCES_H
#define JSE_RESOURCES_H


class JseFilesystem
{
public:
	static bool add_resource_path(const std::string& path);
	static std::string get_text_resource(const std::string& name);
	static std::string get_text_resource_with_includes(const std::string& name, int max_depth = 2);
	static std::vector<unsigned char> get_binary_resource(const std::string& name);
	static std::string get_resource(const std::string& name);

private:
	static std::unordered_map<std::string, std::string> resource_map_;
	static std::set<std::string> sources_;
};

#endif
