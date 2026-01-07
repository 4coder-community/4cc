# 4coder Community [![badge](https://github.com/4coder-community/4cc/actions/workflows/update_release.yml/badge.svg)](https://github.com/4coder-community/4cc/releases/tag/latest)

# Building

## Windows

> [!NOTE]
> 4coder needs the MSVC compiler and windows SDK from the "Desktop development with C++" component. If you don’t have this already, you can download the installer [here](https://visualstudio.microsoft.com/downloads/). Make sure to at least click on "Desktop development with C++" and tick the boxes for MSVC and the Windows SDK.

1. Setup the MSVC toolchain in your environment, this can be done either with the `code/custom/bin/setup_cl_x64.bat` script or by using the development shell in windows terminal
2. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```batch
cd code && .\bin\build.bat
```

In addition to the parameter listed below, you can specify which backend to use by passing one of those parameters to the build scripts:
- `/DWIN32_OPENGL` (default) to use the OpenGL backend.
- `/DWIN32_DX11` to use the Direct3D 11 backend.

```batch
cd code && .\bin\build.bat /DWIN32_DX11
```

## Linux

> tested on Ubuntu 22.04:

1. Get required libraries (apt names):

```sh
sudo apt install build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev`
```

2. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```sh
cd code && ./bin/build-linux.sh
```

## Mac 

> 4coder targets x86_64. If you are using a M1+ ARM CPU you need to prefix the build scripts commands with: `arch -arch x86_64`

1. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```sh
cd code && ./bin/build-mac.sh
```

### Older Macs, 10.15.7 Catalina

If you are using an older version of mac, such as 10.15.7 Catalina you need to install the realpath command:

1. `$ sudo port install coreutils`
2. macports names the `realpath` command `grealpath`, so make a symbolic link in order to use build-mac.sh:  
   `$ sudo ln -s /opt/local/bin/grealpath /opt/local/bin/realpath`

## Build script parameter

The build script accepts a parameter (mutually exclusive):
- `/DDEV_BUILD` (default value) : build without optimizations.
   Produces debug symbols.
   Defines: `FRED_INTERNAL`, `FRED_SUPER`, `DO_CRAZY_EXPENSIVE_ASSERTS` (on Windows) macros.
- `/DOPT_BUILD` (similar to `build_optimized` script): build with optimizations.
   Doesn't produce debug symbols.
   Defines `FRED_SUPER` macro.
   
## API generators

4coder uses several small programs to generate some headers and source files. Those do not run automatically, you must build them and run them when needed (which shouldn't really happen).

- `code\4ed_font_api.cpp` creates, in `code\generated`, `font_api.h`, `font_api.cpp`, `font_api_constructor.cpp` (not used) and `font_api_master_list.h` (not used);
- `code\4ed_graphics_api.cpp` creates, in `code\generated`, `graphics_api.h` and `graphics_api.cpp`, `graphics_api_constructor.cpp` (not used) and `graphics_api_master_list.h` (not used);
- `code\4ed_system_api.cpp` creates, in `code\custom\generated`, `system_api.h`, `system_api.cpp`, `system_api_constructor.cpp`, `system_api_master_list.h`;
- `code\4ed_api_parser_main.cpp` is a little different, as it parses source files passed as parameters to search for functions and type preceded by `api(some_api_name)` and creates 4 files in the `code\custom\generated`. It is used to generate `custom_api.h`, `custom_api.cpp`, `custom_api_constructor.cpp` and `custom_api_master_list.h` by passing `code\4ed_api_implementation.cpp` as a parameter.

You need to compile one of those file and run it from the `code` directory.

There is also `code\4ed_api_check.cpp` to verify the generated file but it's not clear at the moment what to check against.

- `code\4ed_generate_keycodes.cpp` is also a bit appart as it generates `code\custom\generated\4coder_event_codes.h` which are keyboard key codes and some event hook ids.
