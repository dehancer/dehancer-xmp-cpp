# dehancer-xmp-cpp

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
