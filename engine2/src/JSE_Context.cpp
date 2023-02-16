#include "JSE.h"

JseContext appCtx;

JseContext::~JseContext()
{
}

JseModule* JseContext::addModule(JseUniquePtr<JseModule> m)
{
	JseModule* ml = m.get();
	map_.emplace(m->typeIndex(), std::move(m));

	return ml;
}

