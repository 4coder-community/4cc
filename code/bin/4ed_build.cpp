/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.????
 *
 * 4coder development build rule.
 *
 */

// TOP

//#define FM_PRINT_COMMANDS

#include "4coder_base_types.h"
#include "4coder_version.h"

#include "4coder_base_types.cpp"
#include "4coder_malloc_allocator.cpp"

#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4coder_file_moving.h"


//
// OS and compiler index
//

typedef u32 Platform_Code;
enum{
    Platform_Windows,
    Platform_Linux,
    Platform_Mac,
    //
    Platform_COUNT,
    Platform_None = Platform_COUNT,
};

char *platform_names[] = {
    "win",
    "linux",
    "mac",
};

typedef u32 Compiler_Code;
enum{
    Compiler_CL,
    Compiler_GCC,
    Compiler_Clang,
    //
    Compiler_COUNT,
    Compiler_None = Compiler_COUNT,
};

char *compiler_names[] = {
    "cl",
    "gcc",
    "clang",
};

typedef u32 Arch_Code;
enum{
    Arch_X64,
    Arch_X86,
    Arch_arm64,
    
    //
    Arch_COUNT,
    Arch_None = Arch_COUNT,
};

char *arch_names[] = {
    "x64",
    "x86",
    "arm64",
};

#if OS_WINDOWS
# define This_OS Platform_Windows
#elif OS_LINUX
# define This_OS Platform_Linux
#elif OS_MAC
# define This_OS Platform_Mac
#else
# error This platform is not enumerated.
#endif

#if COMPILER_CL
# define This_Compiler Compiler_CL
#elif COMPILER_GCC
# define This_Compiler Compiler_GCC
#elif COMPILER_CLANG
# define This_Compiler Compiler_Clang
#else
# error This compilers is not enumerated.
#endif

//
// Universal directories
//

#define BUILD_DIR ".." SLASH "build"
#define SITE_DIR ".." SLASH "site"

#define FOREIGN ".." SLASH "non-source" SLASH "foreign"

char *includes[] = { "custom", FOREIGN SLASH "freetype2", 0, };

//
// Platform layer file tables
//

char *windows_platform_layer[] = { "platform_win32/win32_4ed.cpp", 0 };
char *linux_platform_layer[] = { "platform_linux/linux_4ed.cpp", 0 };
char *mac_platform_layer[] = { "platform_mac/mac_4ed.mm", 0 };

char **platform_layers[Platform_COUNT] = {
    windows_platform_layer,
    linux_platform_layer  ,
    mac_platform_layer    ,
};

char *windows_cl_platform_inc[] = { "platform_all", 0 };
char *linux_gcc_platform_inc[] = { "platform_all", "platform_unix", 0 };

char *mac_clang_platform_inc[] = { "platform_all", "platform_unix", 0 };

char **platform_includes[Platform_COUNT][Compiler_COUNT] = {
    {windows_cl_platform_inc, 0                     , 0},
    {0                      , linux_gcc_platform_inc, 0},
    {0                      , 0                     , mac_clang_platform_inc},
};

char *default_custom_target = ".." SLASH "code" SLASH "custom" SLASH "4coder_default_bindings.cpp";

// NOTE(allen): Build flags

enum{
    OPTS = 0x1,
    LIBS = 0x2,
    ICON = 0x4,
    SHARED_CODE = 0x8,
    DEBUG_INFO = 0x10,
    OPTIMIZATION = 0x20,
    SUPER = 0x40,
    INTERNAL = 0x80,
    SHIP = 0x100,
    OPENGL = 0x200,
    DX11 = 0x400,
};

internal char**
get_defines_from_flags(Arena *arena, u32 flags){
    char **result = 0;
    if (HasFlag(flags, SHIP)){
        result = fm_list(arena, fm_list_one_item(arena, "SHIP_MODE"), result);
    }
    if (HasFlag(flags, INTERNAL)){
        result = fm_list(arena, fm_list_one_item(arena, "FRED_INTERNAL"), result);
    }
    if (HasFlag(flags, SUPER)){
        result = fm_list(arena, fm_list_one_item(arena, "FRED_SUPER"), result);
    }
    if (HasFlag(flags, OPENGL)) {
        result = fm_list(arena, fm_list_one_item(arena, "WIN32_OPENGL"), result);
    } else if (HasFlag(flags, DX11)) {
        result = fm_list(arena, fm_list_one_item(arena, "WIN32_DX11"), result);
    }
    return(result);
}

//
// build implementation: cl
//

#if COMPILER_CL

#define CL_OPTS                                  \
"-W4 -wd4310 -wd4100 -wd4201 -wd4505 -wd4996 "  \
"-wd4127 -wd4510 -wd4512 -wd4610 -wd4390 "      \
"-wd4611 -wd4189 -WX -GR- -EHa- -nologo -FC"

#define CL_LIBS_COMMON \
"user32.lib winmm.lib gdi32.lib comdlg32.lib userenv.lib "

#if defined( WIN32_DX11 )
#define CL_LIBS_BACKEND " d3d11.lib dxgi.lib d3dcompiler.lib"
#else
#define CL_LIBS_BACKEND " OpenGL32.lib"
#endif

#define CL_LIBS_X64 CL_LIBS_COMMON FOREIGN "\\x64\\freetype.lib" CL_LIBS_BACKEND
#define CL_LIBS_X86 CL_LIBS_COMMON FOREIGN "\\x86\\freetype.lib" CL_LIBS_BACKEND

#define CL_ICON "..\\non-source\\res\\icon.res"

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Temp_Dir temp = fm_pushdir(out_path);
    
    Build_Line line;
    fm_init_build_line(&line);
    
    if (arch == Arch_X86){
        fm_add_to_line(line, "%s\\custom\\bin\\setup_cl_x86.bat &", code_path);
    }
    
    fm_add_to_line(line, "cl");
    
    if (flags & OPTS){
        fm_add_to_line(line, CL_OPTS);
    }
    
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        case Arch_X86: fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        default: InvalidPath;
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
        }
    }
    
    if (flags & LIBS){
        switch (arch){
            case Arch_X64: fm_add_to_line(line, CL_LIBS_X64); break;
            case Arch_X86: fm_add_to_line(line, CL_LIBS_X86); break;
            default: InvalidPath;
        }
    }
    
    if (flags & ICON){
        fm_add_to_line(line, CL_ICON);
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-Zi");
        fm_add_to_line(line, "-DDO_CRAZY_EXPENSIVE_ASSERTS");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O2");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-LD");
    }
    
    if (defines != 0){
        for (u32 i = 0; defines[i] != 0; ++i){
            char *define_flag = fm_str(arena, "-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    for (u32 i = 0; code_files[i]; ++i){
        fm_add_to_line(line, "\"%s\\%s\"", code_path, code_files[i]);
    }
    
    fm_add_to_line(line, "-Fe%s", out_file);
    
    fm_add_to_line(line, "-link -INCREMENTAL:NO -RELEASE -PDBALTPATH:%%_PDB%%");
    switch (arch){
        case Arch_X64: fm_add_to_line(line, "-MACHINE:X64"); break;
        case Arch_X86: fm_add_to_line(line, "-MACHINE:X86"); break;
        default: InvalidPath;
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-DEBUG");
    }
    
    if (flags & SHARED_CODE){
        Assert(exports != 0);
        fm_add_to_line(line, "-OPT:REF");
        for (u32 i = 0; exports[i] != 0; ++i){
            char *str = fm_str(arena, "-EXPORT:", exports[i]);
            fm_add_to_line(line, "%s", str);
        }
    }
    else{
        fm_add_to_line(line, "-NODEFAULTLIB:library");
    }
    
    fm_finish_build_line(&line);
    
    //printf("%s\n", line.build_options);
    systemf("%s", line.build_options);
    fm_popdir(temp);
    
    fflush(stdout);
}

//
// build implementation: gcc
//

#elif COMPILER_GCC

#if OS_LINUX

# define GCC_OPTS                      \
"-Wno-write-strings "                 \
"-D_GNU_SOURCE -fPIC "                \
"-fno-threadsafe-statics -pthread "   \
"-Wno-unused-result "                 \
"-std=c++11"

# define GCC_LIBS_COMMON       \
"-lX11 -lpthread -lm -lrt "   \
"-lGL -ldl -lXfixes -lfreetype -lfontconfig"

# define GCC_LIBS_X64 GCC_LIBS_COMMON
# define GCC_LIBS_X86 GCC_LIBS_COMMON

#else
# error gcc options not set for this platform
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Build_Line line;
    fm_init_build_line(&line);
    
    switch (arch){
        case Arch_X64:
        fm_add_to_line(line, "-m64");
        fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        
        case Arch_X86:
        fm_add_to_line(line, "-m32");
        fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        
        default: InvalidPath;
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, GCC_OPTS);
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
        }
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-g -O0");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O3");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-shared");
    }
    
    if (defines != 0){
        for (u32 i = 0; defines[i]; ++i){
            char *define_flag = fm_str(arena, "-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    fm_add_to_line(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        fm_add_to_line(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        if (arch == Arch_X64){
            fm_add_to_line(line, GCC_LIBS_X64);
        }
        else if (arch == Arch_X86)
        {
            fm_add_to_line(line, GCC_LIBS_X86);
        }
    }
    
    fm_finish_build_line(&line);
    
    Temp_Dir temp = fm_pushdir(out_path);
    systemf("g++ %s -o %s", line.build_options, out_file);
    fm_popdir(temp);
}

#elif COMPILER_CLANG

#if OS_MAC

# define CLANG_OPTS \
"-Wno-write-strings -Wno-deprecated-declarations " \
"-Wno-comment -Wno-switch -Wno-null-dereference " \
"-Wno-tautological-compare -Wno-unused-result " \
"-Wno-missing-declarations -Wno-nullability-completeness " \
"-std=c++11 "

# define CLANG_LIBS_COMMON \
"-framework Cocoa -framework QuartzCore " \
"-framework CoreServices " \
"-framework OpenGL -framework IOKit -framework Metal -framework MetalKit "

# define CLANG_LIBS_X64 CLANG_LIBS_COMMON \
FOREIGN "/x64/libfreetype-mac.a"

# define CLANG_LIBS_X86 CLANG_LIBS_COMMON \
FOREIGN "/x86/libfreetype-mac.a"

#else
# error clang options not set for this platform
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char **code_files, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    Build_Line line;
    fm_init_build_line(&line);
    
    switch (arch){
        case Arch_X64:
        fm_add_to_line(line, "-m64");
        fm_add_to_line(line, "-DFTECH_64_BIT"); break;
        
        case Arch_X86:
        fm_add_to_line(line, "-m32");
        fm_add_to_line(line, "-DFTECH_32_BIT"); break;
        
        default: InvalidPath;
    }
    
    if (flags & OPTS){
        fm_add_to_line(line, CLANG_OPTS);
    }
    
    fm_add_to_line(line, "-I%s", code_path);
    if (inc_folders != 0){
        for (u32 i = 0; inc_folders[i] != 0; ++i){
            char *str = fm_str(arena, code_path, "/", inc_folders[i]);
            fm_add_to_line(line, "-I%s", str);
        }
    }
    
    if (flags & DEBUG_INFO){
        fm_add_to_line(line, "-g -O0");
    }
    
    if (flags & OPTIMIZATION){
        fm_add_to_line(line, "-O3");
    }
    
    if (flags & SHARED_CODE){
        fm_add_to_line(line, "-shared");
    }
    
    if (defines != 0){
        for (u32 i = 0; defines[i]; ++i){
            char *define_flag = fm_str(arena, "-D", defines[i]);
            fm_add_to_line(line, "%s", define_flag);
        }
    }
    
    fm_add_to_line(line, "-I\"%s\"", code_path);
    for (u32 i = 0; code_files[i] != 0; ++i){
        fm_add_to_line(line, "\"%s/%s\"", code_path, code_files[i]);
    }
    
    if (flags & LIBS){
        if (arch == Arch_X64){
            fm_add_to_line(line, CLANG_LIBS_X64);
        }
        else if (arch == Arch_X86)
        {
            fm_add_to_line(line, CLANG_LIBS_X86);
        }
    }
    
    fm_finish_build_line(&line);
    
    Temp_Dir temp = fm_pushdir(out_path);
    
    // systemf("clang++ %s -E -o %s", line.build_options, "4ed.i");
    systemf("clang++ %s -o %s", line.build_options, out_file);
    fm_popdir(temp);
}

#else
# error build function not defined for this compiler
#endif

internal void
build(Arena *arena, u32 flags, u32 arch, char *code_path, char *code_file, char *out_path, char *out_file, char **defines, char **exports, char **inc_folders){
    char **code_files = fm_list_one_item(arena, code_file);
    build(arena, flags, arch, code_path, code_files, out_path, out_file, defines, exports, inc_folders);
}

function void
dispatch_build(Arena *arena, u32 arch, char *cwd, u32 flags, char** dist_files, i32 dist_file_count){
    Temp_Dir temp = fm_pushdir(fm_str(arena, BUILD_DIR));
    
    char *build_script_postfix = "";
    switch (This_OS){
        case Platform_Windows:
        {
            build_script_postfix = "-win";
        }break;
        case Platform_Linux:
        {
            build_script_postfix = "-linux";
        }break;
        case Platform_Mac:
        {
            build_script_postfix = "-mac";
        }break;
    }
    char *build_script = fm_str(arena, "custom/bin/buildsuper_", arch_names[arch], build_script_postfix, BAT);
    
    char *build_command = fm_str(arena, "\"", cwd, SLASH, build_script, "\" \"", default_custom_target, "\"");
    if (This_OS == Platform_Windows){
        build_command = fm_str(arena, "call ", build_command);
    }

    if (flags & OPTIMIZATION) {
        build_command = fm_str(arena, build_command, " release");
    }

    systemf("%s", build_command);
    
    fm_popdir(temp);
    fflush(stdout);

    {
        char *file = fm_str(arena, "4ed_app_target.cpp");
        char **exports = fm_list_one_item(arena, "app_get_functions");
        build(arena, OPTS | SHARED_CODE | flags, arch, cwd, file, BUILD_DIR, "4ed_app" DLL, get_defines_from_flags(arena, flags), exports, includes);
    }
    
    {
        char **inc = (char**)fm_list(arena, includes, platform_includes[This_OS][This_Compiler]);
        build(arena, OPTS | LIBS | ICON | flags, arch, cwd, platform_layers[This_OS], BUILD_DIR, "4ed", get_defines_from_flags(arena, flags), 0, inc);
    }
    
    fflush(stdout);
    
    for (i32 j = 0; j < dist_file_count; j += 1){
        fm_copy_all(dist_files[j], BUILD_DIR);
    }
    
    char *custom_src_dir = fm_str(arena, cwd, SLASH, "custom");
    char *custom_dst_dir = fm_str(arena, BUILD_DIR, SLASH, "custom");
    fm_make_folder_if_missing(arena, custom_dst_dir);
    fm_copy_all(custom_src_dir, custom_dst_dir);
}

int main(int argc, char **argv){
    Arena arena = fm_init_system(DetailLevel_FileOperations);
    
    char cwd[256];
    i32 n = fm_get_current_directory(cwd, sizeof(cwd));
    Assert(n < sizeof(cwd));
    
    u32 flags = 0;

#if ARCH_X86 
    u32 arch = Arch_X86;
#elif ARCH_X64
    u32 arch = Arch_X64;
#elif ARCH_ARM64
    u32 arch = Arch_arm64;
#endif

#if defined(DEV_BUILD)
    flags |= DEBUG_INFO | INTERNAL;
#endif

#if defined(OPT_BUILD)
    flags |= OPTIMIZATION | SHIP;
#endif

#if OS_WINDOWS
    #if defined(WIN32_DX11)
        flags |= DX11;
    #else
        flags |= OPENGL;
    #endif
#endif

    // NOTE(allen): meta
    char *dist_files[] = {
        fm_str(&arena, "../non-source/dist_files"),
        fm_str(&arena, "ship_files"),
    };
    
    printf("cwd: %s\n", cwd);
    printf("BUILD: 4coder\n");
    printf(" arch: %s\n", arch_names[arch]);
    printf(" build dir: %s\n", BUILD_DIR);
    fflush(stdout);
    
    Temp_Memory temp = begin_temp(&arena);
    dispatch_build(&arena, arch, cwd, flags, dist_files, ArrayCount(dist_files));
    end_temp(temp);
    
    return(error_state);
}

// BOTTOM
