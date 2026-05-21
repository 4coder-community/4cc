# 4coder Community [![badge](https://github.com/4coder-community/4cc/actions/workflows/update_release.yml/badge.svg)](https://github.com/4coder-community/4cc/releases/tag/latest)

# Building

## Windows

> [!NOTE]
> 4coder needs the MSVC compiler and windows SDK from the "Desktop development with C++" component. If you don’t have this already, you can download the installer [here](https://visualstudio.microsoft.com/downloads/). Make sure to at least click on "Desktop development with C++" and tick the boxes for MSVC and the Windows SDK.

1. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```cmd
.\build.bat
```

## Linux

> tested on Ubuntu 22.04:

1. Get required libraries (apt names):

```sh
sudo apt install build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev`
```

2. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```sh
./build.sh
```

## Mac 

> 4coder targets x86_64. If you are using a M1+ ARM CPU you need to prefix the build scripts commands with: `arch -arch x86_64`

1. Execute this command from the root of the project (the resulting executable will be under the `build` folder in the root of the repo)

```sh
./build.sh
# For M1+ Macs
arch -arch x86_64 ./build.sh
```

## Build script parameter

Check available options by running:

Windows:
```cmd
.\build.bat --help
```

Linux/MacOS:
```sh
./build.sh --help
```
   
## API generators

4coder uses several small programs to generate some headers and source files. Those do not run automatically, you must build them and run them when needed (which shouldn't really happen).

- `code\4ed_font_api.cpp` creates, in `code\generated`, `font_api.h`, `font_api.cpp`, `font_api_constructor.cpp` (not used) and `font_api_master_list.h` (not used);
- `code\4ed_graphics_api.cpp` creates, in `code\generated`, `graphics_api.h` and `graphics_api.cpp`, `graphics_api_constructor.cpp` (not used) and `graphics_api_master_list.h` (not used);
- `code\4ed_system_api.cpp` creates, in `code\custom\generated`, `system_api.h`, `system_api.cpp`, `system_api_constructor.cpp`, `system_api_master_list.h`;
- `code\4ed_api_parser_main.cpp` is a little different, as it parses source files passed as parameters to search for functions and type preceded by `api(some_api_name)` and creates 4 files in the `code\custom\generated`. It is used to generate `custom_api.h`, `custom_api.cpp`, `custom_api_constructor.cpp` and `custom_api_master_list.h` by passing `code\4ed_api_implementation.cpp` as a parameter.

You need to compile one of those file and run it from the `code` directory.

There is also `code\4ed_api_check.cpp` to verify the generated file but it's not clear at the moment what to check against.

- `code\4ed_generate_keycodes.cpp` is also a bit appart as it generates `code\custom\generated\4coder_event_codes.h` which are keyboard key codes and some event hook ids.
