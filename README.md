# `orbital`

This is a project using SDL2 and modern OpenGL that
simulates the orbit of a satellite around a planet.
My primary goal was to get a feel of how orbital
maneuvers work in a realistic setting, using Earth
and a satellite in LEO. This is a simplified model
considering only Newtonian gravity between two bodies (so
it is not a comprehensive simulation of what actually
happens).

# Controls

Arrow keys control thrust relative to the axis of the
satellite (which is modelled as a fully fueled Falcon 9
upper stage). Press the 'R' key to reset the satellite back
to its initial position and velocity.

# Demo

![orbital.png](https://i.postimg.cc/zDdpn2wd/orbital.png)

Linked [here](https://streamable.com/83lmfk) is a demo of
the program in action. It shows 2 Hohmann transfers, the
first one to a higher orbit and the second back down to a
lower orbit.

# Build

Requires CMake and a C99 compiler to build. Requires 
LibSDL2 to be installed on your system (`sudo apt-get install libsdl2-dev` 
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

Utilizes:

  * [SDL2](https://www.libsdl.org/)
  * [cglm](https://github.com/recp/cglm)
