#include <sma/android/jniinteresthelper.hpp>

#include <sma/android/jninodecontainer.hpp>
#include <sma/ccn/interesthelperimpl.hpp>


JNIEXPORT jboolean JNICALL Java_edu_asu_sma_InterestHelper_create(JNIEnv* env, jobject thiz, jstring interest)
{
  char const* c_sinterest = env->GetStringUTFChars(interest, NULL);
  std::string sinterest(c_sinterest);
  env->ReleaseStringUTFChars(interest, c_sinterest);

  auto local = sma::interest_helper->local();
  for (auto const& i : local)
    if (std::string(i) == sinterest)
      return false;

  sma::interest_helper->create_local(sma::ContentType(sinterest));

  return true;
}

JNIEXPORT void JNICALL Java_edu_asu_sma_InterestHelper_delete(JNIEnv* env, jobject thiz, jstring interest)
{
  char const* c_sinterest = env->GetStringUTFChars(interest, NULL);
  std::string sinterest(c_sinterest);
  env->ReleaseStringUTFChars(interest, c_sinterest);

  sma::interest_helper->delete_local(sma::ContentType(sinterest));
}

JNIEXPORT jobject JNICALL Java_edu_asu_sma_InterestHelper_local(JNIEnv* env, jobject thiz)
{
  assert(sma::interest_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  auto interests = sma::interest_helper->local();
  for (auto const& interest : interests)
    env->CallBooleanMethod(list_obj,
                            env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
                            env->NewStringUTF(std::string(interest).c_str()));

  env->DeleteLocalRef(list_class);

  return list_obj;
}

JNIEXPORT jobject JNICALL Java_edu_asu_sma_InterestHelper_remote(JNIEnv* env, jobject thiz)
{
  assert(sma::interest_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  auto interests = sma::interest_helper->remote();
  for (auto const& interest : interests)
    env->CallBooleanMethod(list_obj,
                            env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
                            env->NewStringUTF(std::string(interest).c_str()));

  env->DeleteLocalRef(list_class);

  return list_obj;
}
