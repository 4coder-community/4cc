# 4Coder Community

Welcome to the 4coder community repository.

# Building

## Windows
1. Setup the MSVC toolchain in your environment, this can be done with the `code/custom/bin/setup_cl_x64.bat` script
2. call the `package.bat` script from the code directory (this builds a distribution in the `distributions` directory with all the non-binary dependencies)
   1. `$ cd 4cc\code`
   2. `.\bin\package.bat`
3. You can also use the `build.bat` script if you want just build the binaries (this produces the build artifacts in the `build` directory, this doesn't produce a functional distribution)
   1. `$ cd 4cc\code`
   2. `$ .\bin\build.bat`

## Linux
> tested on Ubuntu 22.04:

1. Get required libraries (apt names):
    - `$ sudo apt install build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev`
2. Use the `package-linux.sh` script from the code directory (this builds a distribution in the `distributions` directory with all the non-binary dependencies)
   1. `$ cd 4cc/code`
   2. `$ ./bin/package-linux.sh`

3. You can also use the `build-linux.sh` script if you want just build the binaries, (this produces the build artifacts in the `build` directory, this doesn't produce a functional distribution)
   1. `$ cd 4cc/code`
   2. `$ ./bin/build-linux.sh`

## Mac 
> tested on macOS Catalina 10.15.7, MacBook Pro 2013

1. Get required libraries using macports or brew:
  1. `$ sudo port install coreutils`
  2. macports names the `realpath` command `grealpath`, so make a symbolic link in order to use build-mac.sh: 
     `$ sudo ln -s /opt/local/bin/grealpath /opt/local/bin/realpath`

2. Use the `package-mac.sh` script from the code directory (this builds a distribution in the `distributions` directory with all the non-binary dependencies)
   1. `$ cd 4cc/code`
   2. `$ ./bin/package-mac.sh`

3. You can also use the `build-mac.sh` script if you want just build the binaries, (this produces the build artifacts in the `build` directory, this doesn't produce a functional distribution)
   1. `$ cd 4cc/code`
   2. `$ ./bin/build-mac.sh`