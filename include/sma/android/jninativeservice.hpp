#pragma once

#include <jni.h>


extern "C" {
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_captureServicePointer(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_deleteServicePointer(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_runNativeAsyncTask(JNIEnv*, jobject);

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_startLinkThread(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_stopLinkThread(JNIEnv*, jobject);

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_handlePacket(JNIEnv*, jobject);
}


namespace sma
{
extern jobject android_service;
}