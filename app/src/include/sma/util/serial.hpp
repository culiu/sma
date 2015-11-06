#pragma once

#include <sma/util/reader.hpp>

#define CAT(A, B) A##B
#define SELECT(NAME, NUM) CAT(NAME##_, NUM)

#define GET_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, COUNT, ...) COUNT
#define VA_SIZE(...) GET_COUNT(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define VA_SELECT(NAME, ...) SELECT(NAME, VA_SIZE(__VA_ARGS__))(__VA_ARGS__)



#define INIT_FIELD_AS(field, type) field(SMA_DES_READER_R_.template get<type>())

#define INIT_FIELD(field) INIT_FIELD_AS(field, decltype(field))
#define INIT_FIELDS(...) VA_SELECT(INIT_FIELDS_IMPL, __VA_ARGS__)

#define INIT_FIELDS_IMPL_1(a) INIT_FIELD(a)
#define INIT_FIELDS_IMPL_2(a, b) INIT_FIELD(a), INIT_FIELD(b)
#define INIT_FIELDS_IMPL_3(a, b, c) INIT_FIELDS_IMPL_2(a, b), INIT_FIELD(c)
#define INIT_FIELDS_IMPL_4(a, b, c, d)                                         \
  INIT_FIELDS_IMPL_3(a, b, c), INIT_FIELD(d)
#define INIT_FIELDS_IMPL_5(a, b, c, d, e)                                      \
  INIT_FIELDS_IMPL_4(a, b, c, d), INIT_FIELD(e)
#define INIT_FIELDS_IMPL_6(a, b, c, d, e, f)                                   \
  INIT_FIELDS_IMPL_5(a, b, c, d, e), INIT_FIELD(f)
#define INIT_FIELDS_IMPL_7(a, b, c, d, e, f, g)                                \
  INIT_FIELDS_IMPL_6(a, b, c, d, e, f), INIT_FIELD(g)
#define INIT_FIELDS_IMPL_8(a, b, c, d, e, f, g, h)                             \
  INIT_FIELDS_IMPL_7(a, b, c, d, e, f, g), INIT_FIELD(h)
#define INIT_FIELDS_IMPL_9(a, b, c, d, e, f, g, h, i)                          \
  INIT_FIELDS_IMPL_8(a, b, c, d, e, f, g, h), INIT_FIELD(i)


#define PUT_FIELD(field) PUT_FIELDS_IMPL_1(field)
#define PUT_FIELDS(...) VA_SELECT(PUT_FIELDS_IMPL, __VA_ARGS__)

#define PUT_FIELDS_IMPL_1(a) SMA_SER_WRITER_W_ << a
#define PUT_FIELDS_IMPL_2(a, b) PUT_FIELDS_IMPL_1(a) << b
#define PUT_FIELDS_IMPL_3(a, b, c) PUT_FIELDS_IMPL_2(a, b) << c
#define PUT_FIELDS_IMPL_4(a, b, c, d) PUT_FIELDS_IMPL_3(a, b, c) << d
#define PUT_FIELDS_IMPL_5(a, b, c, d, e) PUT_FIELDS_IMPL_4(a, b, c, d) << e
#define PUT_FIELDS_IMPL_6(a, b, c, d, e, f)                                    \
  PUT_FIELDS_IMPL_5(a, b, c, d, e) << f
#define PUT_FIELDS_IMPL_7(a, b, c, d, e, f, g)                                 \
  PUT_FIELDS_IMPL_6(a, b, c, d, e, f) << g
#define PUT_FIELDS_IMPL_8(a, b, c, d, e, f, g, h)                              \
  PUT_FIELDS_IMPL_7(a, b, c, d, e, f, g) << h
#define PUT_FIELDS_IMPL_9(a, b, c, d, e, f, g, h, i)                           \
  PUT_FIELDS_IMPL_8(a, b, c, d, e, f, g, h) << i



#define GET_FIELD(field) SMA_DES_READER_R_ >> field

#define PUT_BYTES(field, size) SMA_SER_WRITER_W_.write(field, size)
#define GET_BYTES(field, size) SMA_DES_READER_R_.read(field, size)


#define DESERIALIZING_CTOR(CLASSNAME)                                          \
  template <typename... SMA_DES_READER_T_>                                     \
  CLASSNAME(Reader<SMA_DES_READER_T_...>& SMA_DES_READER_R_)


#define SERIALIZER()                                                           \
  template <typename SMA_SER_WRITER_T_>                                        \
  void write_fields(SMA_SER_WRITER_T_& SMA_SER_WRITER_W_) const


#define TRIVIALLY_SERIALIZABLE(CLASSNAME, ...)                                 \
  DESERIALIZING_CTOR(CLASSNAME)                                                \
    : INIT_FIELDS(__VA_ARGS__)                                                 \
  {                                                                            \
  }                                                                            \
  SERIALIZER() { PUT_FIELDS(__VA_ARGS__); }

#define EMPTY_SERIALIZABLE(CLASSNAME)                                          \
  DESERIALIZING_CTOR(CLASSNAME) {}                                             \
  SERIALIZER() {}
