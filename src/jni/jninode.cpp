#include <sma/android/jninode.hpp>

#include <sma/android/jninodecontainer.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <memory>
#include <string>
#include <cassert>


JNIEXPORT jstring JNICALL Java_edu_asu_sma_Node_id(JNIEnv* env, jobject thiz)
{
  assert(sma::node != nullptr);
  return env->NewStringUTF(std::string(sma::node->id).c_str());
}

JNIEXPORT void JNICALL Java_edu_asu_sma_Node_tick(JNIEnv* env, jobject thiz)
{
  assert(sma::node != nullptr);
}
