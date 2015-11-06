#pragma once

#include <utility>
#include <type_traits>

namespace sma
{
/*! Example usage:
*
*    using MessageTypes = TypeSieve<int>
*      ::map<0, RequestMessage>
*      ::map<1, ResponseMessage>
*      ::map<2, PingMessage>;
*
*    struct Echo {
*      template <typename T>
*      void operator()(std::string m) {
*        auto tc = MessageTypes::typecode<T>();
*        cout << "message type " << tc << ": " << m;
*      }
*    };
*
*    NamedTypes::apply(2, Echo(), "hello!");
*
*    // output:
*    //
*    // message type 2: hello!
*/
template <typename TypecodeT>
struct TypeSieve {
private:
  // clang-format off
  /***************************************************************************/
  template <TypecodeT tc_, typename T_, typename next_>
  struct Node_ {
  private:
    using cur_ = Node_<tc_, T_, next_>;

  public:
    using typecode_type = TypecodeT;

    /*! \brief Expand this sieve by adding a mapping between the given typecode
     *          and the type T. The defined type contains a reference to this
     *          type and transitively all of its ancestor types, thus building
     *          a chain of (typecode, T) mappings that comprise the sieve.
     *
     * Head::next : undefined
     * Head::map  : A
     *
     * A::next : Head
     * A::map  : B
     *
     * B::next : A
     * B::map  : C
     *
     * . . .
     */
    template <TypecodeT typecode, typename T>
    using map = Node_<typecode, T, cur_>;

    /* \brief Get the typecode value associated with the type of U.
     *
     * Example usage:
     *
     *    PingMessage msg;
     *    auto tc = MessageTypes::typecode(msg);
     *    assert(tc == 2);
     */
    template <typename U>
    static constexpr TypecodeT typecode(U&& u)
    { return typecode<U>(); }

    /* \brief Get the typecode value associated with the type U.
     *
     * Example usage:
     *
     *    auto tc = MessageTypes::template typecode<PingMessage>();
     *    assert(tc == 2);
     *
     * A : Node[tc : Typecode, T]
     * A::typecode[t] : Void --> Typecode
     *
     * A::typecode[t] :: Void => A::next::typecode[t]
     * A::typecode[T] :: Void => tc
     */
    // Have to use SFINAE instead of specialization because C++ doesn't allow
    // specializing a member of an unspecialized template.
    template <typename U>
    static constexpr auto typecode()
      -> typename std::enable_if<std::is_same<U, T_>::value, TypecodeT>::type
    { return tc_; }

    // Recursive case U != T_: search next node for match
    template <typename U>
    static constexpr auto typecode()
      -> typename std::enable_if<not std::is_same<U, T_>::value, TypecodeT>::type
    { return next_::template typecode<U>(); }


    template <typename Receiver, typename... Args>
    static bool apply(TypecodeT& typecode, Receiver&& r, Args&&... args)
    {
      if (typecode == tc_) {
        r.template operator()<T_>(std::forward<Args>(args)...);
        return true;
      } else
        return next_::apply(typecode,
                            std::forward<Receiver>(r),
                            std::forward<Args>(args)...);
    }
  };    // Node_
  /***************************************************************************/
  template <TypecodeT tc_, typename T_, typename next_>
  friend struct Node_;

  using Head_ = TypeSieve<TypecodeT>;

  /*******************************
   * Head implementation        */
  template <typename... Args>
  static bool apply(Args&&...)
  { return false; }

  template <typename TT>
  static constexpr TypecodeT typecode();
  /******************************/
  // clang-format on

public:
  using typecode_type = TypecodeT;

  template <TypecodeT tc, typename T>
  using map = Node_<tc, T, Head_>;
};    // TypeSieve
// clang-format on
}
