#include <sma/android/jnineighborhelper.hpp>
#include <sma/android/jninodecontainer.hpp>

#include <sma/neighborhelperimpl.hpp>

#include <vector>
#include <cassert>

#include <android/log.h>


JNIEXPORT jobject JNICALL Java_edu_asu_sma_NeighborHelper_all(JNIEnv* env, jobject thiz)
{
  assert(sma::neighbor_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  auto neighbors = sma::neighbor_helper->get();
  
  if (neighbors.size() == 0)
    __android_log_write(ANDROID_LOG_ERROR, "neighborhelper", "neighbor list empty");

  for (auto const& neighbor : neighbors) {
    std::string item = std::string("node id: ") 
	    + std::string(neighbor.id)
            + std::string("\t")
	    + std::string(sma::neighbor_helper->get_time_since_last_seen_ms(neighbor.id))
	    + std::string("\tms ago");

    env->CallBooleanMethod(list_obj,
                            env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
                            env->NewStringUTF(item.c_str()));
  }

			    

  env->DeleteLocalRef(list_class);


  return list_obj;
}
