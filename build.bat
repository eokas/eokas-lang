
rd /s /q cmake-build-debug
mkdir cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
cd ..

rd /s /q cmake-build-release
mkdir cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..

rd /s /q cmake-install
cmake --install cmake-build-debug --prefix ./cmake-install --config Debug
cmake --install cmake-build-release --prefix ./cmake-install --config Release

pause
