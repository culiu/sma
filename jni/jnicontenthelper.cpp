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