#pragma once

#include <string>
#include <utility>
#include <vector>

#include "renderer.h"

namespace gfx {

	using ShaderDefs = Vector<std::pair<String, String>>;

	struct RenderProgram {
		String name;
		String vertex;
		String fragment;
		String geomety;
		ShaderDefs defs;
	};
	struct ComputeProgram {
		String name;
		String compute;
		ShaderDefs defs;
	};

	class ShaderManager {
	public:

		ProgramHandle createProgram(const RenderProgram& prg);
		ProgramHandle createProgram(const ComputeProgram& prg);

		ProgramHandle get(const String& name) const;

		ShaderManager(Renderer* r);
		void setVersionString(const String& version);
		String getVersionString() const;

	private:
		Renderer* R_;
		String version_string_;
		HashMap<String, ProgramHandle> program_map_;
	};
}