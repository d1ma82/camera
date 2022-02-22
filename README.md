# NDK camera

Simple NDK camera2 application with GUI on kotlin and  
camera, openGL rendering and filtering (blur, thermal vision) in C++

* API Level 24+

## How to

### Build require

- [Cmake](https://cmake.org/), [Ninja](https://ninja-build.org/) libs.
- To specify [ABI](https://developer.android.com/ndk/guides/abis) use CmakeLists.txt file in \camera\app\src\main\jni.
- By defaul x86-64 selected
#### Environment variables:
- Add To Path Cmake and Ninja
- Add ANDROID_NDK location
- Add ANDROID_SDK_ROOT location
- Add GLM path location
In cmd window:
```
git clone  https://github.com/d1ma82/camera.git
cd .\camera\app\src\main\jni
cmake -B ./build -G "Ninja"
ninja -C ./build 
```

With native lib compiled you can open project in Android studio

## Questions
dimazverincevchanel@gmail.com
