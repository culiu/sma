#pragma once

#include <jni.h>
#include <string>


extern "C"
{
JNIEXPORT void JNICALL Java_edu_asu_sma_ContentHelper_create(JNIEnv* env, 
                                                             jobject thiz, 
                                                             jobjectArray jtypes, 
                                                             jstring jname, 
                                                             jbyteArray jdata, 
                                                             jlong jsize);

JNIEXPORT void JNICALL Java_edu_asu_sma_ContentHelper_fetch(JNIEnv* env,
		                                                    jobject thiz,
								    jstring jname);

//void content_complete_notify(JNIEnv* env, jclass cls, std::string content_name);

JNIEXPORT jobject JNICALL Java_edu_asu_sma_ContentHelper_local(JNIEnv* env, jobject thiz);

JNIEXPORT jobject JNICALL Java_edu_asu_sma_ContentHelper_remote(JNIEnv* env, jobject thiz);
}
