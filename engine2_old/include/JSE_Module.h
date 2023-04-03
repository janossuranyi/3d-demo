#ifndef JSE_MODULE_H
#define JSE_MODULE_H

#include <typeindex>

using JsType = std::type_index;
namespace js {

	class Module {
	public:
		virtual ~Module() {}
		virtual JsType typeIndex() const = 0;
	};

}
#endif