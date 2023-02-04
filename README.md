# 4Coder Community

Welcome to the 4coder community repository.

# Building

## Windows
1. Setup the MSVC toolchain in your environment, this can be done with the `code/custom/bin/setup_cl_x64.bat` script
2. call the `package.bat` script from the code directory
   1. `$ cd 4cc/code`
   2. `./bin/package.bat`
3. You can also use the `build.bat` script if you want just build the binaries, this does not copy the default config, fonts and bindings
   1. `$ cd 4cc/code`
   2. `$ ./bin/build.bat`

## Linux
> tested on Ubuntu 22.04:

1. Get required libraries (apt names):
    - `$ sudo apt install build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev`
2. Use the `package-linux.sh` script to build and package an application
   1. `$ cd 4cc/code`
   2. `$ ./bin/package-linux.sh`

3. You can also use the `build-linux.sh` script if you want just build the binaries, this does not copy the default config, fonts and bindings
   1. `$ cd 4cc/code`
   2. `$ ./bin/build-linux.sh`

## Mac (Untested)
1. The steps should be the same as linux but replace the `*-linux.sh` with their `*-mac.sh` equivalents.