#include <sma/android/jnicontenthelper.hpp>
#include <sma/android/jninodecontainer.hpp>

#include <sma/ccn/contenthelperimpl.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <vector>
#include <cstring>


JNIEXPORT void JNICALL Java_edu_asu_sma_ContentHelper_create(JNIEnv* env,
                                                             jobject thiz,
                                                             jobjectArray jtypes,
                                                             jstring jname,
                                                             jbyteArray jdata,
                                                             jlong jsize)
{
  int types_count = env->GetArrayLength(jtypes);
  std::vector<sma::ContentType> types;

  for (std::size_t i = 0; i < types_count; ++i) {
    jstring j_stype = (jstring)env->GetObjectArrayElement(jtypes, i);
    char const* c_stype = env->GetStringUTFChars(j_stype, NULL);
    types.emplace_back(std::string(c_stype));
    env->ReleaseStringUTFChars(j_stype, c_stype);
  }

  char const* c_name = env->GetStringUTFChars(jname, NULL);
  std::string name(c_name);
  env->ReleaseStringUTFChars(jname, c_name);

  std::size_t data_size(jsize);
  std::uint8_t* data = new std::uint8_t[data_size];
  jbyte* c_data = env->GetByteArrayElements(jdata, NULL);
  std::memcpy(data, c_data, data_size);
  env->ReleaseByteArrayElements(jdata, c_data, JNI_ABORT);

  sma::content_helper->create_new(std::move(types), sma::ContentName(name), data, data_size);

  delete[] data;
}

JNIEXPORT void JNICALL Java_edu_asu_sma_ContentHelper_fetch(JNIEnv* env,
		                                                    jobject thiz,
								    jstring jname)
{
  char const * c_name = env->GetStringUTFChars(jname, NULL);
  std::string name(c_name);
  env->ReleaseStringUTFChars(jname, c_name);
  sma::content_helper->fetch_content(std::move(name));
  env->DeleteLocalRef(jname);
}

JNIEXPORT jobject JNICALL Java_edu_asu_sma_ContentHelper_local(JNIEnv* env, jobject thiz)
{
  assert(sma::content_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  auto metas = sma::content_helper->local_metadata();

  for (auto const & meta : metas) {
    env->CallBooleanMethod(list_obj,
		           env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
			   env->NewStringUTF(std::string(meta).c_str()));
  }

  env->DeleteLocalRef(list_class);
  
  
  return list_obj;
}

//void content_complete_notify(JNIEnv* env, 
//		             jclass cls,
//			     std::string content_name)
//{
//  jclass activity_class = env->FindClass("java/edu/asu/sma/client/NavActivity");
//  jmethodID method = env->GetStaticMethodID(activity_class,
//		  "onComplete", "(Ljava/lang/String;)Z");
//  env->CallStaticBooleanMethod(activity_class, method, 
//		  env->NewStringUTF(content_name.c_str()));
//  env->DeleteLocalRef(activity_class);
//}

JNIEXPORT jobject JNICALL Java_edu_asu_sma_ContentHelper_remote(JNIEnv* env, jobject thiz)
{
  assert(sma::content_helper != nullptr);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  
  auto metas = sma::content_helper->remote_metadata();
  for (auto const & meta : metas) {
    std::stringstream oss;
    oss << std::string(meta) << "\t" << 100 * sma::content_helper->download_progress(meta) << "%";
 
    env->CallBooleanMethod(list_obj,
		            env->GetMethodID(list_class, "add", "(Ljava/lang/Object;)Z"),
			    env->NewStringUTF(oss.str().c_str()));
  }

  env->DeleteLocalRef(list_class);
  
  return list_obj;
}
