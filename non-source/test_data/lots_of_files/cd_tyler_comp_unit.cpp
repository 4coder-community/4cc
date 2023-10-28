
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



#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <windows.h>
#include <gl\gl.h>

#define FTECH_STRING_INC
#include "4tech_string.h"
#include "cd_partition.h"

#include "cd_defines.h"
#include "cd_standard_types.h"
#include "cd_render_target.h"

// NOTE(allen): Not sure what the "r/*" files were
// but I don't seem to have those files after doing
// a git clone so I'm just getting rid of them for now.+

#include "cd_render_helper.cpp"
#include "cd_render_target.cpp"

#include "cd_game_renderer.cpp"



