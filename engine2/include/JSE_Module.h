#ifndef JSE_MODULE_H
#define JSE_MODULE_H

#include <typeindex>

using JseType = std::type_index;

class JseModule {
public:
	virtual ~JseModule() {}
	virtual JseType typeIndex() const = 0;
};

#endif