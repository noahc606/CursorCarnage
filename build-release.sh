# Release
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++
cmake --build build-release
