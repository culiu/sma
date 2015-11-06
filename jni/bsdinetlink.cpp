#include <sma/android/bsdinetlink.hpp>

#include <sma/android/jnilinklayer.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <cassert>


namespace sma
{
BsdInetLink* link_instance = nullptr;

static int jni_throw(JNIEnv* env, char const* class_name, char const* msg)
{
  jclass ex_class = env->FindClass(class_name);
  if (!ex_class) {
    assert(0 && "Unable to find exception class to throw");
    return -1;
  }
  if (env->ThrowNew(ex_class, msg) != JNI_OK) {
    assert(0 && "Unable to throw exception");
    return -1;
  }
  return 1;
}

static void throw_runtime_exception(JNIEnv* env, char const* msg) {
  int thrown = jni_throw(env, "java/lang/RuntimeException", msg);
  assert(thrown);
}


std::uint16_t const BsdInetLink::BCAST_PORT = 9999;
std::string const BsdInetLink::BCAST_IP_ADDR = "255.255.255.255";


BsdInetLink::BsdInetLink(JNIEnv* env)
//  : log(Logger("BsdInetLink"))
{
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    throw_runtime_exception(env, "Error creating socket");

  sockaddr_in bindaddr;
  memset(reinterpret_cast<char*>(&bindaddr), 0, sizeof(bindaddr));
  bindaddr.sin_family = AF_INET;
  bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bindaddr.sin_port = htons(BCAST_PORT);

  if (bind(sock, reinterpret_cast<sockaddr*>(&bindaddr), sizeof(bindaddr)) < 0)
    throw_runtime_exception(env, "Error binding socket to local interface");

//  log.d("Bound socket to INADDR_ANY on port %v", BCAST_PORT);

  memset(reinterpret_cast<char*>(&bcastaddr), 0, sizeof(bcastaddr));
  bcastaddr.sin_family = AF_INET;
  bcastaddr.sin_port = htons(BCAST_PORT);
  assert(inet_pton(AF_INET, BCAST_IP_ADDR.c_str(), &bcastaddr.sin_addr) == 1);

  int so_broadcast = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast)))
    throw_runtime_exception(env, "Error setting SO_BROADCAST");

//  log.d("Using broadcast address %v", BCAST_IP_ADDR);

  link_instance = this;
}

BsdInetLink::~BsdInetLink()
{
  link_instance = nullptr;
  close();
}

void BsdInetLink::receive_packet()
{
  assert(sock != -1);

  char buf[8192];
  std::size_t size = ::recv(sock, buf, sizeof(buf), 0);
  push_packet(buf, size);
}

std::size_t BsdInetLink::read(void* dst, std::size_t size)
{
  auto packet = pop_packet();
  if (packet.first == nullptr)
    return 0;

  assert(size >= packet.second && "Destination buffer smaller than packet data");

  std::memcpy(dst, reinterpret_cast<void const*>(packet.first.get()), packet.second);
  return packet.second;
}

std::size_t BsdInetLink::write(void const* src, std::size_t size)
{
  assert(sock != -1);

  if (size) {
    auto wrote = ::sendto(sock, src, size, 0,
                          reinterpret_cast<sockaddr*>(&bcastaddr), sizeof(bcastaddr));
    if (wrote < 0) {
//      log.e("Error writing to socket");
      return 0;
    }
  }
  return size;
}

void BsdInetLink::close()
{
  if (sock != -1)
    ::close(sock);
}
}
