#include "./Entity3D.h"

namespace jsr {

	Entity3D::Entity3D(eEntityType type_) :
		type(type_),
		model(),
		light()
	{
	}
}