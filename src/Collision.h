#pragma once

#include "PhysObject.h"
#include "PhysTypes.h"
#include <glm/glm.hpp>
#include <vector>



bool genManifold(PhysObject& obj1, PhysObject& obj2, Manifold& out);
