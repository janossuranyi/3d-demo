#ifndef JSE_H
#define JSE_H

#define RCAST(type,var) reinterpret_cast<type>(var)
#define SCAST(type,var) static_cast<type>(var)
#define CCAST(type,var) const_cast<type>(var)
#define JSE_INLINE inline

#include <cinttypes>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>
#include <fstream>
#include <istream>
#include <regex>
#include <variant>
#include <filesystem>
#include <SDL.h>
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "JSE_Core.h"
#include "JSE_Module.h"
#include "JSE_Log.h"
#include "JSE_Containers.h"
#include "JSE_Sys.h"
#include "JSE_Filesystem.h"
#include "JSE_Resources.h"
#include "JSE_Concurrent.h"
#include "JSE_Thread.h"
#include "JSE_Context.h"
#include "JSE_Handle.h"
#include "JSE_Heap.h"
#include "JSE_Memory.h"
#include "JSE_GfxCore.h"
#include "JSE_Input.h"
#include "JSE_Updatable.h"
#include "JSE_Texture.h"
#include "JSE_Material.h"
#include "JSE_PointLight.h"
#include "JSE_GfxDrawVert.h"
#include "JSE_GfxVertexBuffer.h"
#include "JSE_BoundingVolume.h"
//#include "JSE_GfxCoreGL46.h"
#include "JSE_GfxVertCache.h"
#include "JSE_MeshGeometry.h"
#include "JSE_Mesh.h"
#include "JSE_GfxRender.h"
#include "JSE_Node.h"
#include "JSE_Config.h"
#include "JSE_TextureManager.h"
#include "JSE_Engine.h"
#endif
