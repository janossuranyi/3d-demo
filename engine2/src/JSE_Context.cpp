#include "JSE.h"

js::Context appCtx;

namespace js {
	Context::~Context()
	{
	}

	Module* Context::addModule(JsUniquePtr<Module> m)
	{
		Module* ml = m.get();
		map_.emplace(m->typeIndex(), std::move(m));

		return ml;
	}
}

