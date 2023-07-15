/**
 * @file test_find_abi_changes.cpp
 * @copyright Copyright (c) 2022 UCSD PLSysSec. This project is released under
 * @brief Identify if sandbox changes the ABIs
 */

// #include "test_include.hpp"
// #include "rlbox_sandbox_plugin_base.hpp"
// #include "rlbox_wrapper_traits.hpp"

// class sandbox_same_abi
//     : public rlbox::rlbox_sandbox_plugin_base<sandbox_same_abi> {
//  public:
//   template <typename T>
//   using tainted = tainted_relocatable<T, sandbox_same_abi>;

//   template <typename T>
//   using tainted_volatile = tainted_volatile_standard<T, sandbox_same_abi>;
// };

// class sandbox_different_int
//     : public rlbox::rlbox_sandbox_plugin_base<sandbox_different_int> {
//  public:
//   template <typename T>
//   using tainted = tainted_relocatable<T, sandbox_different_int>;

//   template <typename T>
//   using tainted_volatile = tainted_volatile_standard<T,
//   sandbox_different_int>;

//   using sbx_int = uint16_t;
// };

// class sandbox_intptr : public
// rlbox::rlbox_sandbox_plugin_base<sandbox_intptr> {
//  public:
//   template <typename T>
//   using tainted = tainted_relocatable<T, sandbox_intptr>;

//   template <typename T>
//   using tainted_volatile = tainted_volatile_standard<T, sandbox_intptr>;

//   using sbx_pointer = uintptr_t;
// };

// TEST_CASE("Test when ABI changes require integer promotion APIs",
//           "[abi conversion]") {
//   constexpr bool r1 =
//       rlbox::detail::rlbox_base_types_unchanged_v<sandbox_same_abi>;
//   REQUIRE(r1 == true);

//   constexpr bool r2 =
//       rlbox::detail::rlbox_base_types_unchanged_v<sandbox_different_int>;
//   REQUIRE(r2 == false);

//   constexpr bool r3 =
//       rlbox::detail::rlbox_base_types_unchanged_v<sandbox_intptr>;
//   REQUIRE(r3 == true);
// }
