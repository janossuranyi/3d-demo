#ifndef JSE_MODULE_H
#define JSE_MODULE_H

#include <typeindex>

using Type = std::type_index;

class JseModule {
public:
	virtual ~JseModule() {}
	virtual Type typeIndex() const = 0;
};

#endif