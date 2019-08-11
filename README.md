# directory_watcher
Application for directory files monitoring

## Build
This project is CMake-based so you need to use CMake to build it.

### Requirements
* C++14-compatible compiler;
* Qt5 installation;
* Installed Qt toolchain that support chosen compiler;
* CMake 3.1.0 or higher.

### General notes
* Make sure that your Qt toolchain architecture corresponds to compiler architecture (for example, if you want to use MSVC in Win64 configuration, you need appropriate Qt toolchain, like `msvc2015_64` in case of MSVS2015)
* You need to pass the path of Qt CMake modules via `CMAKE_PREFIX_PATH`.
* In some cases you also need to pass some additional information to CMake (like in [THIS EXAMPLE](#qt_mingw_example)).
* In case of problems, see [this official Qt<->CMake manual](https://doc.qt.io/qt-5/cmake-manual.html)

### Microsoft Visual Studio 2015 Win64 build
Run from project root:
```bat
cd build
cmake -G"Visual Studio 14 2015 Win64" -DCMAKE_PREFIX_PATH="Path/To/Qt/version/msvc2015_64/lib/cmake/Qt5" ..
cmake --build . --config Release
```

<a name="qt_mingw_example"></a>
### Build using MinGW taken from Qt installation
Please make sure you have `Path/To/Qt/Tools/mingw*_*/bin` in your `PATH` environment variable.

Run from project root:
```bat
cd build
cmake -G"MinGW Makefiles" -DCMAKE_PREFIX_PATH="Path/To/Qt/version/mingw*_*/lib/cmake/Qt5" -DCMAKE_MAKE_PROGRAM="Path/To/Qt/Tools/mingw*_*/bin/mingw32-make.exe" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
