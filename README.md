# dehancer-xmp-cpp

Requirements 
============

    git clone https://github.com/madler/zlib
    cd zlib 
    ./configure --static --archs="-arch x86_64" --prefix=/usr/local/x86_64
    make && sudo make install
    

Build M1
==========
    mkdir build-arm64 && cd build-arm64
    cmake -DPRINT_DEBUG=ON -DBUILD_TESTING=ON \
    -DCMAKE_OSX_ARCHITECTURES=arm64 -DDEHANCER_TARGET_ARCH=arm64-apple-macos11 

Build Intel
==========
    mkdir build-x86_64 && cd build-x86_64
    cmake -DPRINT_DEBUG=ON -DBUILD_TESTING=ON \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 -DDEHANCER_TARGET_ARCH=x86_64-apple-macos10.14 

Build Linux
==========
    mkdir build-x86_64 && cd build-x86_64
    cmake -DCMAKE_CXX_COMPILER=clang++ -DPRINT_DEBUG=ON -DBUILD_TESTING=ON 

Windows GCC
=======

    # mingw
    # Install https://www.msys2.org/
    # https://blog.jetbrains.com/clion/2020/12/setting-up-clang-on-windows/

    pacman -S mingw-w64-x86_64-toolchain
    pacman -S mingw-w64-x86_64-clang
    pacman -S mingw-w64-x86_64-cmake
    pacman -S libcurl
    pacman -S zlib-devel
    pacman -S libcurl-devel


Windows MVSC
=======
    # Requrements: 
    # Visual Studio, English Language Pack!
    # https://vcpkg.info/
    # GitBash

    cd C:
    git clone https://github.com/microsoft/vcpkg
    cd /c/vcpkg/
    ./bootstrap-vcpkg.sh
    /c/vcpkg/vcpkg integrate install
    /c/vcpkg/vcpkg install expat:x64-windows-static
    /c/vcpkg/vcpkg install libiconv:x64-windows-static 
    /c/vcpkg/vcpkg install zlib:x64-windows-static

    # cmake integration
    -G
    "Ninja"
    -DCMAKE_VERBOSE_MAKEFILE=ON
    -DBUILD_TESTING=ON
    -DPRINT_DEBUG=ON
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    -DVCPKG_TARGET_TRIPLET=x64-windows-static