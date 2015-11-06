#include <sma/android/jninativeservice.hpp>

#include <sma/android/bsdinetlink.hpp>

#include <sma/android/jniasynchelper.hpp>
#include <sma/async.hpp>

#include <sma/android/jnilinklayer.hpp>

//#include <sma/io/log.hpp>

namespace sma
{
jobject android_service = nullptr;

//static Logger log("NativeService");
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_captureServicePointer(JNIEnv* env, jobject thiz)
{
  sma::android_service = env->NewGlobalRef(thiz);
//  sma::log.d("Captured NativeService*");
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_deleteServicePointer(JNIEnv* env, jobject thiz)
{
  sma::asynctaskqueue.clear();
  env->DeleteGlobalRef(sma::android_service);
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_runNativeAsyncTask(JNIEnv* env, jobject thiz)
{
  if (sma::asynctaskqueue.empty())
    return;

  auto it = sma::asynctaskqueue.begin();

  it->second();
  sma::asynctaskqueue.erase(it);
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_startLinkThread(JNIEnv*, jobject)
{
  sma::start_link_read_thread();
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_stopLinkThread(JNIEnv*, jobject)
{
  sma::stop_link_read_thread();
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_handlePacket(JNIEnv*, jobject)
{
  sma::link_instance->readable(true);
}
