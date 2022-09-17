#pragma once

#include "common.h"

struct Entity {
	virtual void updateParentChild() = 0;
};