#include "RenderProgs.h"
#include "RenderSystem.h"
#include "Logger.h"

namespace jsr {

	ProgramManager::ProgramManager() :
		initialized(false),
		g_commonData(),
		g_commonData_h(),
		currentProgram(0)
	{
	}

	ProgramManager::~ProgramManager()
	{
		Shutdown();
	}

	void ProgramManager::BindUniformBlock(eUboBufferBinding binding, vertCacheHandle_t handle)
	{
		UniformBuffer buffer;
		if (renderSystem.vertexCache->GetUniformBuffer(handle, buffer))
		{
			BindUniformBlock(binding, buffer);
		}
	}

	void ProgramManager::BindCommonUniform()
	{
		BindUniformBlock(UBB_COMMON_DATA, g_commonData_h);
	}

	bool ProgramManager::Init()
	{
		if (LowLevelInit())
		{
			initialized = true;

			memset(&g_commonData, 0, sizeof(g_commonData));
			g_commonData_h = renderSystem.vertexCache->AllocStaticUniform(&g_commonData, sizeof(g_commonData));
			BindCommonUniform();

			return true;
		}


		return false;
	}

	ShaderStage::~ShaderStage()
	{
		if (_code) MemFree(_code);
	}

	ShaderStage::ShaderStage(eShaderStage stage, const std::string& code)
	{
		_stage = stage;
		_size = code.size();
		_code = (uint8_t*)MemAlloc((15 + _size) & ~15);
		memcpy(_code, code.c_str(), code.size());

	}

	ShaderStage::ShaderStage(eShaderStage stage, const std::vector<uint8_t>& code) :
		_stage(stage),
		_size(code.size())
	{
		_code = (uint8_t*)MemAlloc(code.size());
		memcpy(_code, code.data(), _size);
	}
	ShaderStage::ShaderStage(eShaderStage stage, const uint8_t* code, size_t size) :
		_stage(stage),
		_size(size)
	{
		_code = (uint8_t*)MemAlloc(size);
		memcpy(_code, code, size);
	}
	const uint8_t* ShaderStage::GetCode() const
	{
		return _code;
	}
	size_t ShaderStage::GetSize() const
	{
		return _size;
	}

}
