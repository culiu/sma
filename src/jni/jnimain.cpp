#include <jni.h>

#include <sma/android/jvm.hpp>

#include <cassert>


JavaVM* g_jvm;


extern "C"
{
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
  g_jvm = jvm;

  return JNI_VERSION_1_6;
}
}

void get_env(JNIEnv*& env_out)
{
  g_jvm->AttachCurrentThreadAsDaemon(&env_out, 0);
  assert(env_out != nullptr && "No JNIEnv* after attaching current thread");
}