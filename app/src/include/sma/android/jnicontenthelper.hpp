#pragma once

#include <jni.h>


extern "C"
{
JNIEXPORT void JNICALL Java_edu_asu_sma_ContentHelper_create(JNIEnv* env, 
                                                             jobject thiz, 
                                                             jobjectArray jtypes, 
                                                             jstring jname, 
                                                             jbyteArray jdata, 
                                                             jlong jsize);
}