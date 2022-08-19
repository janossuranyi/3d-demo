#pragma once

#include <cinttypes>
#include <filesystem>
#include <functional>
#include <string>
#include <stb_image.h>

class FileSystem
{
public:
	FileSystem();
	void set_working_dir(const std::string&);
	std::string working_dir() const;
	std::string resolve(const std::string&) const;
	bool read_text_file(const std::string&, std::string&);
	bool read_text_file_base(const std::string&, std::string&);
	std::vector<uint8_t> read_binary_file(const std::string& aFileName);
	std::vector<std::string> get_directory_entries(const std::string& dirname, const char* filter = nullptr);
	void get_directory_entries(const std::string& dirname, const std::function<void(const std::string&)>& fn, const char* filter = nullptr);
	bool load_image_base(const std::string& filename, int& w, int& h, int& channels, unsigned char** data);
	void free_image(void* data);
private:
	std::filesystem::path m_working_dir;
};

extern FileSystem g_fileSystem;

