/*
 * 4coder development build rule.
 */

#include "4coder_base_types.h"

#include "4coder_base_types.cpp"
#include "4coder_malloc_allocator.cpp"

//'prev_error' and 'error_state' come from "4coder_file_moving.h" and indicate
// an error happened with a file moving operation.
#define FTECH_FILE_MOVING_IMPLEMENTATION
#include "4coder_file_moving.h"

// Utilities

#define ForEachNode(NAME, LIST) \
    for (auto* NAME = (LIST)->first; NAME != NULL; NAME = NAME->next)

#define smchi(X, Y) string_match_insensitive(X, Y)

// Constants

static char* BUILD_ROOT;
static char* SRC_ROOT;
static char* NON_SRC_ROOT;
static char* CUSTOM_ROOT;
static char* FOREIGN_ROOT;

// Build Options

enum {
    LIBS = bit_1,
    ICON = bit_2,
    SHARED_CODE = bit_3,
    DEBUG_INFO = bit_4,
    OPTIMIZATION = bit_5,
    SHIP = bit_6,
    VERBOSE = bit_7,
};

enum GLAPI {
    GLAPI_OPENGL,
    GLAPI_DX11,
};

enum BuildMode {
    MODE_DEBUG = DEBUG_INFO,
    MODE_RELEASE = OPTIMIZATION | SHIP,
};

struct BuildFlags {
    BuildMode type = MODE_DEBUG;
    GLAPI backend = GLAPI_OPENGL;
    u32 opts = 0;
};

#if COMPILER_CL && OS_WINDOWS /////////////////////////////////////////////////////////////////////

void build(
    Arena *arena,
    const char* out_file,
    List_String_Const_char files,
    List_String_Const_char defines,
    List_String_Const_char exports,
    List_String_Const_char includes,
    BuildFlags flags
) {
    Temp_Dir temp = fm_pushdir(BUILD_ROOT);
    
    Build_Line line;
    fm_init_build_line(&line);
    
    fm_add_to_line(line, "cl");
    
    fm_add_to_line(line, "-Fe%s", out_file);
    
    const char* opts[] = {
        "-W4",     // Enables all warnings
        "-WX",     // Raise warnings to errors
        "-nologo", // Suppress copyright message
        "-FC",     // Use full pathnames in diagnostics 
        "-GR-",    // Disable Runtime Type informations
        "-EHa-",   // Disable C++ EH (w/ SEH exceptions)
        "-wd4310", // Disable "cast truncates constant value"
        "-wd4100", // Disable "'identifier' : unreferenced formal parameter"
        "-wd4201", // Disable "nonstandard extension used : nameless struct/union "
        "-wd4505", // Disable "'function' : unreferenced local function has been removed"
        "-wd4996", // Disable "deprecation warnings"
        "-wd4127", // Disable "conditional expression is constant"
        "-wd4510", // Disable "'class': default constructor was implicitly defined as deleted"
        "-wd4512", // Disable "'class' : assignment operator could not be generated "
        "-wd4610", // Disable "object 'class' can never be instantiated - user-defined constructor required"
        "-wd4390", // Disable "';' : empty controlled statement found; is this the intent?"
        "-wd4611", // Disable "interaction between 'function' and C++ object destruction is non-portable"
        "-wd4189", // Disable "'identifier' : local variable is initialized but not referenced"
    };

    for (int i = 0; i < ArrayCount(opts); ++i) {
        fm_add_to_line(line, "%s", opts[i]);
    }
    
    if (flags.type == MODE_DEBUG && !(flags.opts & OPTIMIZATION)) {
        fm_add_to_line(line, "-Od");
    }
    
    if (flags.opts & DEBUG_INFO) {
        fm_add_to_line(line, "-Zi");
    }

    if (flags.opts & OPTIMIZATION) {
        fm_add_to_line(line, "-O2");
    }
    
    if (flags.opts & SHARED_CODE) {
        fm_add_to_line(line, "-LD");
    }
    
    ForEachNode(iter, &includes) {
        fm_add_to_line(line, "-I%.*s", string_expand(iter->string));
    }
    
    ForEachNode(iter, &defines) {
        fm_add_to_line(line, "-D%.*s", string_expand(iter->string));
    }
    
    ForEachNode(iter, &files) {
        fm_add_to_line(line, "\"%.*s\"", string_expand(iter->string));
    }

    if (flags.opts & ICON) {
        fm_add_to_line(line, "..\\non-source\\res\\icon.res");
    }

    if (flags.opts & LIBS) {
        fm_add_to_line(line, "%s", "user32.lib winmm.lib gdi32.lib comdlg32.lib userenv.lib");

        if (ARCH_X64) {
            fm_add_to_line(line, "%s", fm_str(arena, FOREIGN_ROOT, "\\x64\\freetype.lib"));
        } else if (ARCH_X86) {
            fm_add_to_line(line, "%s", fm_str(arena, FOREIGN_ROOT, "\\x86\\freetype.lib"));
        } else {
            InvalidPath;
        }

        switch (flags.backend) {
            case GLAPI_OPENGL: {
                fm_add_to_line(line, "OpenGL32.lib");
            } break;
            case GLAPI_DX11: {
                fm_add_to_line(line, "d3d11.lib dxgi.lib d3dcompiler.lib");
            } break;
            default: {
                InvalidPath;
            }break;
        }
    }

    
    fm_add_to_line(line, "-link -INCREMENTAL:NO -RELEASE -PDBALTPATH:%%_PDB%%");

    if (ARCH_X64) {
        fm_add_to_line(line, "-MACHINE:X64");
    } else if (ARCH_X86) {
        fm_add_to_line(line, "-MACHINE:X86");
    } else {
        InvalidPath;
    }
    
    if (flags.opts & DEBUG_INFO){
        fm_add_to_line(line, "-DEBUG");
    }
    
    if (flags.opts & SHARED_CODE) {
        fm_add_to_line(line, "-OPT:REF");
        ForEachNode(iter, &exports) {
            fm_add_to_line(line, "-EXPORT:%.*s", string_expand(iter->string));
        }
    } else {
        fm_add_to_line(line, "-NODEFAULTLIB:library");
    }
    
    fm_finish_build_line(&line);
    
    if (flags.opts & VERBOSE) {
        printf("%s\n", line.build_options);
    }

    systemf("%s", line.build_options);
    fm_popdir(temp);
}

#elif COMPILER_GCC && OS_LINUX ////////////////////////////////////////////////////////////////////

void build(
    Arena *arena,
    const char* out_file,
    List_String_Const_char files,
    List_String_Const_char defines,
    List_String_Const_char exports,
    List_String_Const_char includes,
    BuildFlags flags
) {
    Temp_Dir temp = fm_pushdir(BUILD_ROOT);

    Build_Line line;
    fm_init_build_line(&line);

    fm_add_to_line(line, "g++");

    fm_add_to_line(line, "-o %s", out_file);
        
    const char* opts[] = {
        "-D_GNU_SOURCE",                    // Enable GNU extentions
        "-fPIC",                            // Enable Position Independent Code
        "-fno-threadsafe-statics -pthread", // Don't use C++ thread safety routines
        "-Wno-write-strings",               // Disable "warning: deprecated conversion from string constant to 'char*'"
        "-Wno-unused-result",               // Disable "warning: ignoring return value of 'function_name'"
        "-std=c++11",
    };

    for (int i = 0; i < ArrayCount(opts); ++i) {
        fm_add_to_line(line, "%s", opts[i]);
    }

    if (flags.type == MODE_DEBUG && !(flags.opts & OPTIMIZATION)) {
        fm_add_to_line(line, "-O0");
    }

    if (flags.opts & DEBUG_INFO) {
        fm_add_to_line(line, "-g");
    }
    
    if (flags.opts & OPTIMIZATION) {
        fm_add_to_line(line, "-O3");
    }
    
    if (flags.opts & SHARED_CODE) {
        fm_add_to_line(line, "-shared");
    }
    
    ForEachNode(iter, &defines) {
        fm_add_to_line(line, "-D%.*s", string_expand(iter->string));
    }

    ForEachNode(iter, &includes) {
        fm_add_to_line(line, "-I%.*s", string_expand(iter->string));
    }

    ForEachNode(iter, &files) {
        fm_add_to_line(line, "'%.*s'", string_expand(iter->string));
    }
    
    if (flags.opts & LIBS) {
        fm_add_to_line(line, "-lX11 -lpthread -lm -lrt -lGL -ldl -lXfixes -lfreetype -lfontconfig");
    }

    fm_finish_build_line(&line);

    if (flags.opts & VERBOSE) {
        printf("%s\n", line.build_options);
    }
    
    systemf("%s", line.build_options);
    fm_popdir(temp);
}

#elif COMPILER_CLANG && OS_MAC ////////////////////////////////////////////////////////////////////

void build(
    Arena *arena,
    const char* out_file,
    List_String_Const_char files,
    List_String_Const_char defines,
    List_String_Const_char exports,
    List_String_Const_char includes,
    BuildFlags flags
) {
    Temp_Dir temp = fm_pushdir(BUILD_ROOT);

    Build_Line line;
    fm_init_build_line(&line);

    fm_add_to_line(line, "clang++");

    fm_add_to_line(line, "-o %s", out_file);
    
    const char* opts[] = {
        "-Wno-write-strings",            // Disable "warning: deprecated conversion from string constant to 'char*'"
        "-Wno-deprecated-declarations",  // Disable deprecation warnings
        "-Wno-switch",                   // Disable "enumeration value B not handled in switch"
        "-Wno-null-dereference",         // Disable "warning: binding dereferenced null pointer to reference has undefined behavior"
        "-Wno-unused-result",            // Disable "warning: ignoring return value of 'function_name'"
        "-Wno-missing-declarations",     // Disable "warning: declaration does not declare anything"
        "-Wno-nullability-completeness", // Disable "warning: pointer is missing a nullability type specifier (_Nonnull, _Nullable, or _Null_unspecified)"
        "-Wno-tautological-compare",
        "-std=c++11 ",
    };

    for (int i = 0; i < ArrayCount(opts); ++i) {
        fm_add_to_line(line, "%s", opts[i]);
    }

    if (flags.type == MODE_DEBUG && !(flags.opts & OPTIMIZATION)) {
        fm_add_to_line(line, "-O0");
    }
    
    if (flags.opts & DEBUG_INFO) {
        fm_add_to_line(line, "-g");
    }
    
    if (flags.opts & OPTIMIZATION) {
        fm_add_to_line(line, "-O3");
    }
    
    if (flags.opts & SHARED_CODE) {
        fm_add_to_line(line, "-shared");
    }

    ForEachNode(iter, &includes) {
        fm_add_to_line(line, "-I%.*s", string_expand(iter->string));
    }

    ForEachNode(iter, &defines) {
        fm_add_to_line(line, "-D%.*s", string_expand(iter->string));
    }

    ForEachNode(iter, &files) {
        fm_add_to_line(line, "'%.*s'", string_expand(iter->string));
    }

    if (flags.opts & LIBS) {
        fm_add_to_line(line, "-framework Cocoa -framework QuartzCore "
                             "-framework CoreServices -framework OpenGL "
                             "-framework IOKit -framework Metal "
                             "-framework MetalKit");

        if (ARCH_X64) {
            fm_add_to_line(line, "%s", fm_str(arena, FOREIGN_ROOT, "/x64/libfreetype-mac.a"));
        } else if (ARCH_X86) {
            fm_add_to_line(line, "%s", fm_str(arena, FOREIGN_ROOT, "/x86/libfreetype-mac.a"));
        } else {
            InvalidPath;
        }
    }
    
    fm_finish_build_line(&line);

    if (flags.opts & VERBOSE) {
        printf("%s\n", line.build_options);
    }
    
    systemf("%s", line.build_options);
    fm_popdir(temp);
}

#else
# error build function not defined for this compiler
#endif ////////////////////////////////////////////////////////////////////////////////////////////

function void
dispatch_build(Arena* arena, BuildFlags flags) {
    // Build Super
    {
        Temp_Dir dir = fm_pushdir(BUILD_ROOT);

        const char* build_script = fm_str(arena, CUSTOM_ROOT, "/bin/buildsuper_", ARCH_NAME, "-" OS_NAME, BAT);
        char* default_custom_target = fm_str(arena, CUSTOM_ROOT, "/4coder_default_bindings.cpp");
        char* build_cmd = fm_str(arena, "\"", build_script, "\" \"", default_custom_target, "\"");

        if (OS_WINDOWS) {
            build_cmd = fm_str(arena, "call ", build_cmd);
        }

        if (flags.type == MODE_RELEASE) {
            build_cmd = fm_str(arena, build_cmd, " release");
        }

        if (flags.opts & VERBOSE) {
            printf("%s\n", build_cmd);
        }

        systemf("%s", build_cmd);

        fm_popdir(dir);

        if (prev_error != 0) {
            return;
        }
    }

    // Setup prequesites for build

    List_String_Const_char includes = {};
    string_list_push(arena, &includes, SCchar(SRC_ROOT));
    string_list_push(arena, &includes, SCchar(CUSTOM_ROOT));
    string_list_push(arena, &includes, SCchar(fm_str(arena, FOREIGN_ROOT, "/freetype2")));

    if (OS_MAC || OS_LINUX) {
        string_list_push(arena, &includes, SCchar("platform_unix"));
    }

    List_String_Const_char defines = {};
    switch (flags.type) {
        case MODE_DEBUG: {
            string_list_push(arena, &defines, SCchar("FRED_INTERNAL"));
        } break;
        case MODE_RELEASE: {
            string_list_push(arena, &defines, SCchar("SHIP_MODE"));
            string_list_push(arena, &defines, SCchar("NDEBUG"));
        } break;
        default: {
            InvalidPath;
        } break;
    }

    switch (flags.backend) {
        case GLAPI_OPENGL: {
            string_list_push(arena, &defines, SCchar("WIN32_OPENGL"));
        } break;
        case GLAPI_DX11: {
            string_list_push(arena, &defines, SCchar("WIN32_DX11"));
        } break;
        default: {
            InvalidPath;
        } break;
    }

    // Build 4ed_app
    {
        List_String_Const_char files = {};
        string_list_push(arena, &files, SCchar(fm_str(arena, SRC_ROOT, "/4ed_app_target.cpp")));

        List_String_Const_char exports = {};
        string_list_push(arena, &exports, SCchar("app_get_functions"));

        BuildFlags opts = flags;
        opts.opts = flags.type | flags.opts | SHARED_CODE;
        build(arena, "4ed_app" DLL, files, defines, exports, includes, opts);

        if (prev_error != 0) {
            return;
        }
    }
    
    // Build Platform Layer
    {
        List_String_Const_char files = {};
        if (OS_WINDOWS) {
            string_list_push(arena, &files, SCchar(fm_str(arena, SRC_ROOT, "/platform_win32/win32_4ed.cpp")));
        } else if (OS_LINUX) {
            string_list_push(arena, &files, SCchar(fm_str(arena, SRC_ROOT, "/platform_linux/linux_4ed.cpp")));
        } else if (OS_MAC) {
            string_list_push(arena, &files, SCchar(fm_str(arena, SRC_ROOT, "/platform_mac/mac_4ed.mm")));
        }

        BuildFlags opts = flags;
        opts.opts = flags.type | flags.opts | LIBS | ICON;
        build(arena, "4ed", files, defines, {}, includes, opts);

        if (prev_error != 0) {
            return;
        }
    }

    // Copy distribution files into build dir
    {
        char *files[] = {
            fm_str(arena, NON_SRC_ROOT, "/dist_files"),
            fm_str(arena, SRC_ROOT, "/ship_files"),
        };
        
        for (int i = 0; i < ArrayCount(files); i += 1) {
            fm_copy_all(files[i], BUILD_ROOT);
        }

        char* custom_dst = fm_str(arena, BUILD_ROOT, "/custom");
        fm_make_folder_if_missing(arena, custom_dst);
        fm_copy_all(CUSTOM_ROOT, custom_dst);
    }
}

void usage(FILE* fd) {
    fprintf(fd, R"(Usage: build [options]

Options:
    -m=<string>, --mode=<string>
        Sets build mode 
        Available modes:
            - debug
            - release
        Default: debug

    -gl=<string> --gl-api=<string>
        Sets the graphics API to use
        Available backends:
            - OpenGL
            - DX11 (Windows only)
        Default: OpenGL

    -s, --symbols
        Adds debug symbols. Already set by --mode=debug.

    -o, --optimizations
        Adds optimizations. Already set by --mode=release.

    -h, --help
        Display this help text and exit

    -v, --verbose
        Display build debug information like command currentely executing
    )");
}

String_Const_char get_flag_value(String_Const_char flag) {
    for (int i = 0; i < flag.size; ++i) {
        if (flag.str[i] == '=') {
            return SCchar(flag.str + i + 1, flag.size - i - 1);
        }
    }
    return SCchar("");
}

BuildFlags parse_flags(int argc, const char** argv) {
    // Default flags
    BuildFlags flags = {};

    for (int i = 1; i < argc; ++i) {
        String_Const_char arg = SCchar((char*)argv[i]);

        if (smchi(arg, SCchar("-h")) || smchi(arg, SCchar("--help"))) {
            usage(stdout);
            exit(0);
        } else if (smchi(arg, SCchar("-v")) || smchi(arg, SCchar("--verbose"))) {
            flags.opts |= VERBOSE;
        } else if (smchi(arg, SCchar("-s")) || smchi(arg, SCchar("--symbols"))) {
            flags.opts |= DEBUG_INFO;
        } else if (smchi(arg, SCchar("-o")) || smchi(arg, SCchar("--optimiziations"))) {
            flags.opts |= OPTIMIZATION;
        } else if (string_has_prefix(arg, SCchar("-m=")) || string_has_prefix(arg, SCchar("--mode="))) {
            String_Const_char param = get_flag_value(arg);
            if (smchi(param, SCchar("release"))) {
                flags.type = MODE_RELEASE;
            } else if (smchi(param, SCchar("debug"))) {
                flags.type = MODE_DEBUG;
            } else {
                fprintf(stderr, "build: invalid mode '%.*s'\n", string_expand(param));
                usage(stderr);
                exit(1);
            }
        } else if (string_has_prefix(arg, SCchar("--gl-api=")) || string_has_prefix(arg, SCchar("-gl="))) {
            String_Const_char param = get_flag_value(arg);

            if (smchi(param, SCchar("DX11")) && OS_WINDOWS) {
                flags.backend = GLAPI_DX11;
            } else if (smchi(param, SCchar("OpenGL"))) {
                flags.backend = GLAPI_OPENGL;
            } else {
                fprintf(stderr, "build: invalid backend '%.*s'\n", string_expand(param));
                usage(stderr);
                exit(1);
            }
        } else {
            fprintf(stderr, "build: unrecognised option '%.*s'\n", string_expand(arg));
            usage(stderr);
            exit(1);
        }
    }

    return flags;
}

bool set_common_dirs(Arena* arena) {
    char cwd[256];
    int cwd_size = fm_get_current_directory(cwd, sizeof(cwd));
    if (cwd_size >= sizeof(cwd)) {
        fprintf(stderr, "CWD length exceed %zu characters. Aborting.", sizeof(cwd));
        return false;
    }

    BUILD_ROOT = fm_str(arena, cwd, "/build");
    SRC_ROOT = fm_str(arena, cwd, "/code");
    NON_SRC_ROOT = fm_str(arena, cwd, "/non-source");
    CUSTOM_ROOT = fm_str(arena, cwd, "/code/custom");
    FOREIGN_ROOT = fm_str(arena, cwd, "/non-source/foreign");

    return true;
}

int main(int argc, const char **argv){
    Arena arena = fm_init_system(DetailLevel_FileOperations);

    BuildFlags flags = parse_flags(argc, argv);

    if (!set_common_dirs(&arena)) {
        return 1;
    }

    dispatch_build(&arena, flags);
    
    return error_state;
}

// BOTTOM