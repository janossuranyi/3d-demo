#include "shader_manager.h"
#include "renderer.h"
#include "resource/resource_manager.h"

namespace gfx {

    ProgramHandle ShaderManager::createProgram(const RenderProgram& p)
    {
        if (program_map_.count(p.name) > 0)
        {
            return program_map_.at(p.name);
        }

        auto prg = R_->createProgram();
        ShaderHandle v, f;

        {
            String output(version_string_);

            for (auto& def : p.defs)
            {
                 output += "#define " + def.first + " " + def.second + "\n";
            }

            output += rc::ResourceManager::get_text_resource_with_includes(p.vertex);

            v = R_->createShader(ShaderStage::Vertex, output);
        }

        {
            String output(version_string_);
            for (auto& def : p.defs)
            {
                output += "#define " + def.first + " " + def.second + "\n";
            }

            output += rc::ResourceManager::get_text_resource_with_includes(p.fragment);

            f = R_->createShader(ShaderStage::Fragment, output);
        }

        R_->linkProgram(prg, Vector<ShaderHandle>{ v,f });
        R_->deleteShader(v);
        R_->deleteShader(f);

        program_map_.insert(std::make_pair(p.name, prg));

        return prg;

    }
    ProgramHandle ShaderManager::createProgram(const ComputeProgram& p)
    {
        if (program_map_.count(p.name) > 0)
        {
            return program_map_.at(p.name);
        }

        auto prg = R_->createProgram();
        ShaderHandle c;

        {
            String output(version_string_);

            for (auto& def : p.defs)
            {
                output += "#define " + def.first + " " + def.second + "\n";
            }

            output += rc::ResourceManager::get_text_resource_with_includes(p.compute);

            c = R_->createShader(ShaderStage::Compute, output);
        }

        R_->linkProgram(prg, Vector<ShaderHandle>{ c });
        R_->deleteShader(c);

        return prg;
    }
   
    ProgramHandle ShaderManager::get(const String& name) const
    {
        auto& it = program_map_.find(name);

        return it != program_map_.end() ? it->second : ProgramHandle{};
    }

    ShaderManager::ShaderManager(Renderer* r) : R_(r)
    {
    }
    void ShaderManager::setVersionString(const String& version)
    {
        version_string_ = version;
    }
    String ShaderManager::getVersionString() const
    {
        return version_string_;
    }
}