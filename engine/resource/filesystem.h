#pragma once

#include <cinttypes>
#include <filesystem>
#include <functional>
#include <string>
#include <stb_image.h>

namespace rc {
	class FileSystem
	{
	public:
		//FileSystem();
		static void set_working_dir(const std::string&);
		static std::string working_dir();
		static std::string resolve(const std::string&);
		static bool read_text_file(const std::string&, std::string&);
		static bool read_text_file_base(const std::string&, std::string&);
		static std::vector<uint8_t> read_binary_file(const std::string& aFileName);
		static std::vector<std::string> get_directory_entries(const std::string& dirname, bool recursive = false, const char* filter = nullptr);
		static void get_directory_entries(const std::string& dirname, const std::function<void(const std::string&)>& fn, const char* filter = nullptr);
		static bool load_image_base(const std::string& filename, int& w, int& h, int& channels, unsigned char** data);
		static void free_image(void* data);
	private:
		static std::filesystem::path m_working_dir;
	};

}
