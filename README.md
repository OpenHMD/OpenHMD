# OpenHMD
This project aims to provide a Free and Open Source API and drivers for immersive technology, such as head mounted displays with built in head tracking.

## License
OpenHMD is released under the permissive Boost Software License (see LICENSE for more information), to make sure it can be linked and distributed with both free and non-free software. While it doesn't require contribution from the users, it is still very appreciated.

## Supported Devices
  * Oculus Rift DK1 and DK2 (rotation only)

## Supported Platforms
  * Linux
  * Windows
  * OS X

## Requirements
  * GNU Autotools (if you're building from the git repository)
  * HIDAPI
    * http://www.signal11.us/oss/hidapi/
    * https://github.com/signal11/hidapi/

## Language Bindings
  * Perl bindings by CandyAngel
    * https://bitbucket.org/CandyAngel/perl-openhmd
  * Python bindings by Lubosz Sarnecki
    * https://github.com/lubosz/python-rift
  
## Other FOSS HMD Drivers
  * libvr - http://hg.sitedethib.com/libvr

## Compiling and Installing
    ./autogen.sh # (if you're building from the git repository)
    ./configure [--enable-openglexample]
    make
    sudo make install

### Configuring udev on Linux
To avoid having to run your applications as root to access USB devices you have to add a udev rule (this will be included in .deb packages, etc).

as root, run:

    echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2833", MODE="0666", GROUP="plugdev"' > /etc/udev/rules.d/83-hmd.rules
    udevadm control --reload-rules

After this you have to unplug your Rift and plug it back in. You should now be able to access the Oculus Rift as a normal user.


### Cross compiling for windows using mingw
    export PREFIX=/usr/i686-w64-mingw32/ (or whatever your mingw path is)
    PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig ./configure --build=`gcc -dumpmachine` --host=i686-w64-mingw32 --prefix=$PREFIX
    make
the library will end up in the .lib directory, you can use microsoft's lib.exe to make a .lib file for it

### Static linking on windows
If you're linking statically with OpenHMD using windows/mingw you have to make sure the macro OHMD_STATIC is set before including openhmd.h. In GCC this can be done by adding the compiler flag -DOHMD_STATIC, and with msvc it can be done using /DOHMD_STATIC.

Note that this is *only* if you're linking statically! If you're using the DLL then you *must not* define OHMD_STATIC. (If you're not sure then you're probably linking dynamically and won't have to worry about this).

## Pre-built packages
Will be available soon.

## Using OpenHMD
See the examples/ subdirectory for usage examples. The OpenGL example is not built by default, to build it use the --enable-openglexample option for the configure script. It requires SDL, glew and OpenGL.

An API reference can be generated using doxygen and is also available here: http://openhmd.net/doxygen/0.1.0/openhmd_8h.html


