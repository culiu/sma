#include <sma/android/jnineighborhelper.hpp>
#include <sma/android/jninodecontainer.hpp>

#include <sma/neighborhelperimpl.hpp>

#include <vector>
#include <cassert>


JNIEXPORT jobject JNICALL Java_edu_asu_sma_NeighborHelper_all(JNIEnv* env, jobject thiz)
{
  assert(sma::neighbor_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  auto neighbors = sma::neighbor_helper->get();
  for (auto const& neighbor : neighbors)
    env->CallBooleanMethod(list_obj,
                            env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
                            env->NewStringUTF(std::string(neighbor.id).c_str()));

  env->DeleteLocalRef(list_class);

  return list_obj;
}
