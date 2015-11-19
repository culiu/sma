# sma
an information-centric device to device network on android

# structure
The project consists of two parts, i.e., (1) the sma core and (2) Android app.

- sma core is located in src/jni
- Android app is located in the "app" under the root folder.

# compile and run
1. cd src/jni
2. ndk-build
3. cp src/obj/local/armeabi-v7a/libsmanative.so app/src/main/jniLibs/armeabi-v7a/
4. launch Android Studio and open app
