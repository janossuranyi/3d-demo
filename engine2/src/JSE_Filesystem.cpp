#include "JSE.h"

#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include "logger.h"

//FileSystem g_fileSystem;

namespace fs = std::filesystem;

fs::path JSE_FileSystem::m_working_dir{ fs::current_path() };

void JSE_FileSystem::set_working_dir(const std::string& p0)
{
	m_working_dir = fs::path{ p0 };// .make_preferred();
}

std::string JSE_FileSystem::working_dir()
{
	return m_working_dir.generic_string();
}

std::string JSE_FileSystem::resolve(const std::string& aPath)
{
	const fs::path p = m_working_dir / aPath;
	return p.string();
}

bool JSE_FileSystem::read_text_file(const std::string& fileName, std::string& output)
{

	std::ifstream ifs(fileName.c_str(), std::ios::in);
	if (ifs.is_open())
	{
		std::stringstream sstr;
		sstr << ifs.rdbuf();
		output = sstr.str();
		ifs.close();
	}
	else
	{
		Error("Cannot load file %s", fileName.c_str());

		return false;
	}

	return true;
}

bool JSE_FileSystem::read_text_file_base(const std::string& filename, std::string& output)
{
	return read_text_file(resolve(filename), output);
}

std::vector<uint8_t> JSE_FileSystem::read_binary_file(const std::string& filename)
{
	std::vector<uint8_t> result;

	std::ifstream input{ filename, std::ios::binary };

	if (input.good())
	{
		// copies all data into buffer
		result = std::vector<uint8_t>{ std::istreambuf_iterator<char>(input), {} };

		input.close();
	}

	return result;
}

std::vector<std::string> JSE_FileSystem::get_directory_entries(const std::string& dirname, bool recursive, const char* filter)
{
	std::vector<std::string> result;
	const fs::path path = fs::absolute(fs::path(dirname));

	std::filesystem::directory_iterator start;
	try {
		start = fs::directory_iterator{ path };
	}
	catch (std::exception e)
	{
		Error("%s", e.what());
		return result;
	}

	if (!filter)
	{
		for (auto const& e : start)
		{
			if (e.is_regular_file()) {
				result.push_back(e.path().generic_string());
			}
			else if (e.is_directory() && recursive)
			{
				auto a = get_directory_entries(e.path().generic_string(), recursive);
				if (!a.empty()) result.insert(result.end(), a.begin(), a.end());
			}
		}
	}
	else
	{
		try
		{
			std::regex fr{ filter, std::regex::icase };
			for (auto const& e : fs::directory_iterator{ path })
			{
				if (e.is_regular_file() && std::regex_match(e.path().generic_string(), fr)) {
					result.push_back(e.path().generic_string());
				}
				else if (e.is_directory() && recursive)
				{
					auto a = get_directory_entries(e.path().generic_string(), recursive, filter);
					if (!a.empty()) result.insert(result.end(), a.begin(), a.end());
				}
			}
		}
		catch (const std::exception& ex)
		{
			Error("Invalid regexp (%s) %s", filter, ex.what());
		}

	}
	return result;
}

void JSE_FileSystem::get_directory_entries(const std::string& dirname, const std::function<void(const std::string&)>& fn, const char* filter)
{
	for (auto e : get_directory_entries(dirname, filter)) {
		fn(e);
	}
}
bool JSE_FileSystem::load_image_base(const std::string& filename, int& w, int& h, int& channels, unsigned char** data)
{
	std::string abspath = resolve(filename);
	int _w, _h, _n;
	stbi_set_flip_vertically_on_load(true);

	*data = stbi_load(abspath.c_str(), &_w, &_h, &_n, 0);
	if (*data != nullptr)
	{
		w = _w;
		h = _h;
		channels = _n;
		return true;
	}

	return false;
}
void JSE_FileSystem::free_image(void* data)
{
	stbi_image_free(data);
}
