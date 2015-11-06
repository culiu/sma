#pragma once

#include <jni.h>


extern "C"
{
JNIEXPORT jboolean JNICALL Java_edu_asu_sma_InterestHelper_create(JNIEnv* env, jobject thiz, jstring interest);
JNIEXPORT void JNICALL Java_edu_asu_sma_InterestHelper_delete(JNIEnv* env, jobject thiz, jstring interest);

JNIEXPORT jobject JNICALL Java_edu_asu_sma_InterestHelper_local(JNIEnv* env, jobject thiz);
JNIEXPORT jobject JNICALL Java_edu_asu_sma_InterestHelper_remote(JNIEnv* env, jobject thiz);
}