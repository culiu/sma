#pragma once

#include <jni.h>


extern "C" {
JNIEXPORT jstring JNICALL Java_edu_asu_sma_Node_id(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_Node_tick(JNIEnv*, jobject);
}
