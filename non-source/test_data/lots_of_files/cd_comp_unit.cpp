
#include "cd_ids.h"
#include "cd_defines.h"
#include "cd_intrin.h"
#include "cd_standard_types.h"

#include "cd_system.h"
#include "cd.h"

#include "cd_partition.h"
#include "stb_truetype.h"
#include "cd_render_target.h"

#ifdef DEVELOPER
#include "cd_debug.h"
#endif

#include "cd_render_helper.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <windows.h>
#include <gl\gl.h>

#include "cd_render_target.cpp"
#include "cd_debug_renderer.cpp"
#include "win32_cd.cpp"

