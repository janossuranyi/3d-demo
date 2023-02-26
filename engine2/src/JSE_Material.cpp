#include "JSE.h"

namespace js {

	void Material::CopyAttributes(int n, void* dest) const
	{
		size_t offset = 0;
		size_t bytes = 0;
		uint8_t* ptr = SCAST(uint8_t*, dest);

		for (const auto& it : attributes) {
			bytes = 0;
			if (std::holds_alternative<vec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<ivec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<uvec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<mat4>(it.second)) {
				bytes = 64;
			}

			assert(bytes > 0);

			if (bytes && offset < n)
				std::memcpy(ptr + offset, &it.second, bytes);

			offset += bytes;
		}
	}

	size_t Material::size() const
	{
		size_t size = 0;
		for (const auto& it : attributes) {
			size_t bytes = 0;
			if (std::holds_alternative<vec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<ivec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<uvec4>(it.second)) {
				bytes = 16;
			}
			else if (std::holds_alternative<mat4>(it.second)) {
				bytes = 64;
			}
			size += bytes;
		}

		return size;
	}

	void Material::AddAttribute(const JsString& name, const MaterialAttribute& value)
	{
		attributes.emplace_back(name, value);
	}

	JsString Material::GetUniformBlockDef(int binding, const JsString& name, const JsString& instanceName) const
	{
		JsString res{};
		res.append("layout(binding = ")
			.append(std::to_string(binding))
			.append(", std140) uniform ")
			.append(name).append(" {\n");

		for (const auto& attr : attributes)
		{
			JsString type;
			if (std::holds_alternative<vec4>(attr.second))
			{
				type = "\tvec4 ";
			}
			else if (std::holds_alternative<mat4>(attr.second)) {
				type = "\tmat4 ";
			}
			else if (std::holds_alternative<ivec4>(attr.second)) {
				type = "\tivec4 ";
			}
			else if (std::holds_alternative<uvec4>(attr.second)) {
				type = "\tuvec4 ";
			}

			if (type.empty() == false) {
				res.append(type).append(attr.first).append(";\n");
			}
		}

		res.append("} ").append(instanceName).append(";\n");

		return res;
	}

}

