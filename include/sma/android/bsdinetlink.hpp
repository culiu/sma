#pragma once

#include <jni.h>
#include <sma/android/jninativeservice.hpp>

#include <sma/link.hpp>
//#include <sma/io/log.hpp>

#include <netinet/in.h>

#include <jni.h>
#include <string>
#include <cstdlib>
#include <cstdint>


namespace sma
{
class BsdInetLink : public Link
{
public:
  friend JNIEXPORT void JNICALL ::Java_edu_asu_sma_client_NativeService_handlePacket(JNIEnv*, jobject);

  BsdInetLink(JNIEnv* env);
  BsdInetLink(BsdInetLink const&) = delete;
  BsdInetLink& operator=(BsdInetLink const&) = delete;

  ~BsdInetLink();

  void receive_packet();

  std::size_t read(void* dst, std::size_t size) override;
  std::size_t write(void const* src, std::size_t size) override;

  void close() override;

private:
  static std::uint16_t const BCAST_PORT;
  static std::string const BCAST_IP_ADDR;

//  Logger log;

  int sock = -1;
  int sendsock = -1;
  sockaddr_in bcastaddr;
};

extern BsdInetLink* link_instance;
}
