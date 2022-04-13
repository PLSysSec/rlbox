#pragma once

namespace rlbox {
// Tainted data returned by the sandbox. T is the type of the data,  TSbx is
// the type of the sandbox plugin that represents the underlying sandbox
// implementation. This type assumes
// - the sandbox heap cannot be moved once created (fixed)
// - the sandbox heap is aligned to its size (aligned)
//      e.g., 4gb heap is aligned to 4gb
// Due to these assumptions, if the tainted data is a pointer type, the
// value is stored as a global pointer.
template<typename T, typename TSbx>
class tainted_fixed_aligned
{};
}
