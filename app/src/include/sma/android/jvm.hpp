#pragma once

#include <jni.h>


extern JavaVM* g_jvm;

void get_env(JNIEnv*& env_out);