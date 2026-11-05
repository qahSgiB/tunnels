# tunnels
*tunnels* is an application to compute and view protein tunnels.
I developed it as my [bachelor thesis](https://is.muni.cz/th/jh9ye/).

Application is programmed using C++.
For the graphics part of the application WebGPU is used.

## Running

### Prerequisities
You need CMake in order to build the application.

### Building
There is a CMake configuration and presets provided. There are two configure and build presetes:
 - `debug-x64` for debug builds
 - `release-x64` for release builds

There is one target - `tunnels` which builds the application binary.

For example, to build the application using debug preset run the following commands:
```
cmake --preset debug-x64
cmake --build --preset debug-x64 --target tunnels
```

(To use release preset just replace "debug-x64" in the above commands with "release-x64".)

### Running
After building the application, binary named `tunnels` (or `tunnels.exe` on Windows) will be in folder `build/<preset name>`.

For example on unix with the debug preset you can run the app using a following command: `./build/debug-x64/tunnels`.

## Libraries
List of libraries used in this project:
 - **Dawn** located in `libraries/dawn` - older version from their [Google Git](https://dawn.googlesource.com/dawn) is used. This copy of Dawn contains only neccessary include files and prebuilt shared library for Linux and Windows.
 - **[GLFW](https://www.glfw.org/)** located in `libraries/glfw` - older version from their [GitHub](https://github.com/glfw/glfw) is used, unnecessary files were stripped off
 - **GLM** located in `libraries/glm` - older version from their [GitHub](https://github.com/g-truc/glm) is used, unnecessary files were stripped off
 - **[HEALPix](https://healpix.sourceforge.io/)** located in `libraries/healpix` - version 3.83 from their [SourceForge](https://sourceforge.net/projects/healpix/) is used, only C++ library is used, unnecessary files were stripped off
 - **Dear ImGui** located in `libraries/imgui` - older version from their [GitHub](https://github.com/ocornut/imgui) is used, unnecessary files were stripped off

All of these libraries have individual licenses (see `LICENSE` file).

## emscripten
Originally the application also supported compilation to an web application using emscripten.
But since then it stopped working and I haven't updated it.
(New version version of emscripten is currently not compatible with the application.
Old version of emscripten compatible with the application is not compatible with current WebGPU standard implemented in browser.)
