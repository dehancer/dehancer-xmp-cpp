# dehancer-xmp-cpp

## Build

Requires CMake 4.3+, C++17, and installed dependencies. Use Ninja Multi-Config,
Xcode, or Visual Studio. Dependencies supplied by a parent target are reused.

Dependencies: `dehancer_common_cpp` and Exiv2 (with XMP support).

```sh
# export PKG_CONFIG_PATH=/opt/dehancer-dependencies/lib/pkgconfig:$PKG_CONFIG_PATH
cmake -S . -B build \
  -G "Ninja Multi-Config" \
  -DCMAKE_PREFIX_PATH="/opt/dehancer-dependencies;$HOME/local-dehancer" \
  -DBUILD_TESTING=OFF \
  -DCREATE_PKG_CONFIG=OFF
cmake --build build --config Release --parallel $(nproc)
cmake --install build --config Release --prefix $HOME/local-dehancer
```

## Usage

```cmake
find_package(dehancer_xmp_cpp CONFIG REQUIRED)
target_link_libraries(app PRIVATE dehancer_xmp_cpp::dehancer_xmp_cpp)
```

The same target is available with `add_subdirectory(path/to/dehancer-xmp-cpp)`
or FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(dehancer_xmp_cpp
    GIT_REPOSITORY https://github.com/dehancer/dehancer-xmp-cpp.git
    GIT_TAG v7)
FetchContent_MakeAvailable(dehancer_xmp_cpp)
```
