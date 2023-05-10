#include "./Entity3D.h"

namespace jsr {

	Entity3D::Entity3D(eEntityType type_) :
		type(type_),
		value()
	{
	}
	void Entity3D::SetValue(void* ptr)
	{
		value = ptr;
	}
	void Entity3D::SetType(eEntityType type)
	{
		this->type = type;
	}
}
