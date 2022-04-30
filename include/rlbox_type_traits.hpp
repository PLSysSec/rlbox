/**
 * @file rlbox_type_traits.hpp
 * @brief This header contains misc type traits useful for rlbox
 */
#pragma once

namespace rlbox::detail {

template<typename TSourcePtr, typename TDestPtr>
using preserve_ptr_cv_ref_t = int;

}