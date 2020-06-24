# `orbital`

# Build

Requires CMake and a C compiler to build. Requires LibSDL2
to be installed on your system (`sudo apt-get install libsdl2-dev` 
on Debian) and OpenGL (`sudo apt-get install libgl1-mesa-dev`
or native GPU driver) to run.

``` shell
git clone https://github.com/AgentTroll/orbital.git
cd orbital
mkdir build && cd build
cmake .. && make
./orbital
```

# Credits

Built with [CLion](https://www.jetbrains.com/clion/)

Utilizes [LibSDL2](https://www.libsdl.org/)
