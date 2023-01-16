#ifndef JSE_H
#define JSE_H

#include <cinttypes>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <istream>
#include <regex>
#include <filesystem>
#include <SDL.h>
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "JSE_Core.h"
#include "JSE_Log.h"
#include "JSE_Containers.h"
#include "JSE_Sys.h"
#include "JSE_Filesystem.h"
#include "JSE_Resources.h"
#include "JSE_Concurrent.h"
#include "JSE_Thread.h"
#include "JSE_Handle.h"
#include "JSE_Heap.h"
#include "JSE_Memory.h"
#include "JSE_GfxCore.h"
#include "JSE_GfxRender.h"
#endif
