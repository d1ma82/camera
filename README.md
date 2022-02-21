# NDK camera

Simple NDK camera API

* API Level 24+

## How to

### Build require

- Cmake, Ninja and glm libs.
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
