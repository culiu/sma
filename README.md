# sma
an information-centric device to device network on android

# Devices
- Devices should be rooted
- Flash the [cyanogenmod](http://www.cyanogenmod.org) firmware to support WiFi AdHoc mode.
- Nexus 5 and Nexus 7 are both supported. However, the latest version 12.1 does not support Nexus 5.
- A working version can be downloaded here [cyanogenmod 12.1] (http://culiuliu.net/downloads/cm-12.1-20151110-NIGHTLY-grouper.zip)

# structure
The project consists of two parts, i.e., (1) the sma core and (2) Android app.

- sma core is located in src/jni
- Android app is located in the "app" under the root folder.

# compile and run
1. cd src/jni
2. ndk-build
3. cp src/obj/local/armeabi-v7a/libsmanative.so app/src/main/jniLibs/armeabi-v7a/
4. launch Android Studio and open app
