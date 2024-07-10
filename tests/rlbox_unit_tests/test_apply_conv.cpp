/**
 * @file test_apply_conv.cpp
 * @copyright Copyright (c) 2023 Shravan Narayan. This project is released under
 * the MIT License. You can obtain a copy of the License at
 * https://raw.githubusercontent.com/PLSysSec/rlbox/master/LICENSE
 * @brief Check that conversions applied to compound types works correctly on
 * the base types
 */

#include <type_traits>

#include "rlbox_type_traits.hpp"
#include "test_include.hpp"

#define REQUIRE_SAME(...) REQUIRE(std::is_same_v<__VA_ARGS__>)

template <typename T>
class Wrap {};

template <typename T>
using apply = rlbox::detail::apply_conv_to_base_types<T, Wrap>;

template <typename T>
using applyPtr =
    rlbox::detail::apply_conv_to_base_types<T, Wrap,
                                            true /* TCallTConvOnPointers */>;

TEST_CASE("Test basic cases", "[base type conversion]") {
  REQUIRE_SAME(apply<int>, Wrap<int>);
  REQUIRE_SAME(apply<int*>, Wrap<int>*);
  REQUIRE_SAME(applyPtr<int*>, Wrap<int*>);
  REQUIRE_SAME(apply<int[]>, Wrap<int>[]);
  REQUIRE_SAME(apply<int[3]>, Wrap<int>[3]);
  REQUIRE_SAME(apply<int(float)>, Wrap<int>(Wrap<float>));
  REQUIRE_SAME(apply<int (*)(float)>, Wrap<int>(*)(Wrap<float>));
  REQUIRE_SAME(apply<std::array<int, 3>>, std::array<Wrap<int>, 3>);
}

TEST_CASE("Test const cases", "[base type conversion]") {
  REQUIRE_SAME(apply<const int>, const Wrap<int>);

  REQUIRE_SAME(apply<const int*>, const Wrap<int>*);
  REQUIRE_SAME(apply<int* const>, Wrap<int>* const);
  REQUIRE_SAME(apply<const int* const>, const Wrap<int>* const);

  REQUIRE_SAME(applyPtr<const int*>, Wrap<const int*>);
  REQUIRE_SAME(applyPtr<int* const>, Wrap<int*> const);
  REQUIRE_SAME(applyPtr<const int* const>, Wrap<const int*> const);

  REQUIRE_SAME(apply<const int[]>, const Wrap<int>[]);
  REQUIRE_SAME(apply<const int[3]>, const Wrap<int>[3]);

  REQUIRE_SAME(apply<std::array<const int, 3>>, std::array<const Wrap<int>, 3>);
  REQUIRE_SAME(apply<const std::array<int, 3>>, const std::array<Wrap<int>, 3>);
  REQUIRE_SAME(apply<const std::array<const int, 3>>,
               const std::array<const Wrap<int>, 3>);
}

TEST_CASE("Test volatile cases", "[base type conversion]") {
  REQUIRE_SAME(apply<volatile int>, volatile Wrap<int>);

  REQUIRE_SAME(apply<volatile int*>, volatile Wrap<int>*);
  REQUIRE_SAME(apply<int* volatile>, Wrap<int>* volatile);
  REQUIRE_SAME(apply<volatile int* volatile>, volatile Wrap<int>* volatile);

  REQUIRE_SAME(applyPtr<volatile int*>, Wrap<volatile int*>);
  REQUIRE_SAME(applyPtr<int* volatile>, Wrap<int*> volatile);
  REQUIRE_SAME(applyPtr<volatile int* volatile>, Wrap<volatile int*> volatile);

  REQUIRE_SAME(apply<volatile int[]>, volatile Wrap<int>[]);
  REQUIRE_SAME(apply<volatile int[3]>, volatile Wrap<int>[3]);

  REQUIRE_SAME(apply<std::array<volatile int, 3>>,
               std::array<volatile Wrap<int>, 3>);
  REQUIRE_SAME(apply<volatile std::array<int, 3>>,
               volatile std::array<Wrap<int>, 3>);
  REQUIRE_SAME(apply<volatile std::array<volatile int, 3>>,
               volatile std::array<volatile Wrap<int>, 3>);
}

TEST_CASE("Test ref cases", "[base type conversion]") {
  REQUIRE_SAME(apply<int&>, Wrap<int>&);
  REQUIRE_SAME(apply<int*&>, Wrap<int>*&);
  REQUIRE_SAME(applyPtr<int*&>, Wrap<int*>&);

  //   REQUIRE_SAME(apply<std::array<int&, 3>>, std::array<Wrap<int>&, 3>);
  REQUIRE_SAME(apply<std::array<int, 3>&>, std::array<Wrap<int>, 3>&);
  //   REQUIRE_SAME(apply<std::array<int&, 3>&>, std::array<Wrap<int>&, 3>&);
}

TEST_CASE("Test refref cases", "[base type conversion]") {
  REQUIRE_SAME(apply<int&&>, Wrap<int>&&);
  REQUIRE_SAME(apply<int*&&>, Wrap<int> * &&);
  REQUIRE_SAME(applyPtr<int*&&>, Wrap<int*>&&);

  //   REQUIRE_SAME(apply<std::array<int&&, 3>>, std::array<Wrap<int>&&, 3>);
  REQUIRE_SAME(apply<std::array<int, 3>&&>, std::array<Wrap<int>, 3>&&);
  //   REQUIRE_SAME(apply<std::array<int&&, 3>&&>, std::array<Wrap<int>&&,
  //   3>&&);
}
