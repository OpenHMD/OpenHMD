# OpenHMD
OpenHMD aims to provide a free and open source API as well as drivers for immersive technology, such as Head-mounted displays with built-in head tracking. <br>
OpenHMD supports Linux, Windows, macOS, Android and FreeBSD.

OpenHMD is released under the permissive [Boost Software License](https://www.boost.org/LICENSE_1_0.txt), to make sure it can be used and distributed with both free and non-free software. <br>
Even though it doesn't require contribution any sort of from the users, it is still very appreciated.

For a full list of supported devices, please check [our wiki.](https://github.com/OpenHMD/OpenHMD/wiki/Support-List)

## Requirements
You can either use [Ninja](https://ninja-build.org) in conjunction with [Meson](https://mesonbuild.com/) or [CMake](https://cmake.org). <br>
You'll also need a copy of [hidapi](https://github.com/signal11/hidapi/).

## Language bindings
- [Go bindings](https://github.com/Apfel/OpenHMD-GO) by [Marko (Apfel)](https://github.com/Apfel)
- [Java bindings](https://github.com/OpenHMD/OpenHMD-Java) by [Joey Ferwerda](https://github.com/TheOnlyJoey) and [Koen Mertens](https://github.com/KCMertens)
- [.NET bindings](https://github.com/jurrien-fakkeldij/OpenHMD.NET) by [Jurrien Fakkeldij](https://github.com/jurrien-fakkeldij)
- [Perl bindings](https://github.com/CandyAngel/perl-openhmd) by [CandyAngel](https://github.com/CandyAngel)
- [Python bindings](https://github.com/lubosz/python-rift) by [Lubosz Sarnecki](https://github.com/lubosz)
- [Rust bindings](https://github.com/TheHellBox/openhmd-rs) by [TheHellBox](https://github.com/TheHellBox)
  
## Other FOSS HMD Drivers
- libvr[http://hg.sitedethib.com/libvr]

## Compiling and Installing
### Meson

With Meson, you can enable and disable drivers to compile OpenHMD with.

Currently, the available drivers are:
- `rift`
- `deepon`
- `psvr`
- `vive`
- `nolo`
- `wmr`
- `xgvr`
- `vrtek`
- `external`
- `android`

These can be enabled or disabled by adding `-Ddrivers=...` with a comma separated list after the meson command, or using `meson configure ./build -Ddrivers=...` for an already existing build folder. <br>
Replace the `...` with the previously mentioned list of drivers that you'd like to enable. <br>
By default all drivers except `android` are enabled.

```sh
meson build [-Dexamples=simple,opengl] [-Ddrivers=rift,deepon,psvr,vive,nolo,wmr,xgvr,vrtek,external,android]
ninja -C build
sudo ninja -C build install
```

### CMake:

With CMake, you can enable and disable drivers to compile OpenHMD with.

Currently, the available drivers are:
- `OPENHMD_DRIVER_OCULUS_RIFT`
- `OPENHMD_DRIVER_DEEPOON`
- `OPENHMD_DRIVER_PSVR`
- `OPENHMD_DRIVER_HTC_VIVE`
- `OPENHMD_DRIVER_NOLO`
- `OPENHMD_DRIVER_WMR`
- `OPENHMD_DRIVER_XGVR`
- `OPENHMD_DRIVER_VRTEK`
- `OPENHMD_DRIVER_EXTERNAL`
- `OPENHMD_DRIVER_ANDROID.`

These can be enabled or disabled adding `-DDRIVER_OF_CHOICE=ON` *(obviously replacing `DRIVER_OF_CHOICE` with, well, your driver of choice™)* after the `cmake` command, or by using `cmake-gui`.

```sh
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Windows
CMake has a lot of generators available for IDEs and build systems. <br>
The easiest way to find one that fits your system is by checking the supported generators for you CMake version online.

Example using Visual Studio 2012:
```sh
cmake . -G "Visual Studio 12 2013 Win64"
```

and Visual Studio 2019:
```sh
cmake -G "Visual Studio 16 2019" -A x64 -Thost=x64
```

Refer to the CMake instructions above for selecting drivers. <br>
After CMake is done, you'll find a recently generated Solution for compiling OpenHMD.

However, you can also use Meson and Ninja to build on Windows. <br>
To do this, execute the following in one of the developer command lines shipped with Visual Studio:

```sh
meson --backend=ninja build -Dexamples=simple
ninja -C build
```

Once again, refer to the Meson instructions above for setting the build up.

#### Cross compiling for windows using MinGW

For cross compiling via MinGW, toolchain-based files tend to be the best solution. Therefore, please check the CMake documentation on how to do this. <br>
A starting point might be the [CMake wiki](https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/cross_compiling/Mingw).

#### Static linking on windows
If you're linking against OpenHMD statically on Windows *(no matter if MinGW was used or not)*, you'll have to make sure that `OHMD_STATIC` is set before including `openhmd.h`. For GCC, this can be done by adding the compiler flag `-DOHMD_STATIC`, and with MSVC it can be done using `/DOHMD_STATIC`.

## Pre-built packages
A list of pre-built backages can be found on [our site](http://www.openhmd.net/index.php/download/).

## Utilizing OpenHMD
See the `examples` subdirectory for usage examples. The OpenGL-based example is not built by default, thus you'll need to refer to the instructions for the build system you'll use. It requires [SDL2](https://libsdl.org/), [glew](http://glew.sourceforge.net/basic.html) and a library implementing the API of [OpenGL](https://www.opengl.org/). <br>
The package implementing the OpenGL API may vary from OS to OS, for example, on Debian/Ubuntu, it's called `libgl1-mesa-dev`.

An API reference can be generated using doxygen and is also available [here]http://openhmd.net/doxygen/0.1.0/openhmd_8h.html.