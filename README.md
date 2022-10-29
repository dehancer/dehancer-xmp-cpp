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


iOS 
=======
    cd external/exiv2 && mkdir build-debug-ios && cd build-debug-ios
    export PKG_CONFIG_PATH=/opt/homebrew/opt/zlib/lib/pkgconfig:~/Develop/local/ios-debug/dehancer/lib/pkgconfig

    cmake -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=../../lib/ios.toolchain.cmake \
    -DPLATFORM=OS64COMBINED \
    -DENABLE_BITCODE=ON \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=~/Develop/local/ios-debug/dehancer \
    -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DEXIV2_BUILD_SAMPLES=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF \
    -DEXIV2_ENABLE_XMP=ON -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF ..
    
    cmake --build . --config Debug && cmake --install . --config Debug 

    cd external/exiv2 && mkdir build-release-ios && cd build-release-ios
    export PKG_CONFIG_PATH=/opt/homebrew/opt/zlib/lib/pkgconfig:~/Develop/local/ios-release/dehancer/lib/pkgconfig

    cmake -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=../../lib/ios.toolchain.cmake \
    -DPLATFORM=OS64COMBINED \
    -DENABLE_BITCODE=ON \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=~/Develop/local/ios-release/dehancer \
    -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DEXIV2_BUILD_SAMPLES=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF \
    -DEXIV2_ENABLE_XMP=ON -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF ..

    cmake --build . --config Release && cmake --install . --config Release 

    cd ../../../ && mkdir build-ios && cd build-ios

    cmake -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=../lib/ios.toolchain.cmake \
    -DPLATFORM=OS64COMBINED \
    -DENABLE_BITCODE=ON \
    -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=~/Develop/local/ios/dehancer ..

    cmake --build . --config Debug && cmake --install . --config Debug

    
    Add to XCode Project  
    
    Build Settiongs tab:
    Header Search Paths: ~/Develop/local/ios/dehancer/include/**
    Library Search Paths: ~/Develop/local/ios/dehancer/lib/**     

    Framework, Libraries section in XCode:
    libdehancer_xmp_cpp.a
    libdehancer_common_cpp.a
    libexiv2-xmp.a
    libexiv2.a
    libbase64cpp.a
    libed25519cpp.a
    libexpat.tbd
    libiconv.tbd
    libz.tbd

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