#pragma once

namespace rlbox {
// Tainted data returned by the sandbox. T is the type of the data,  TSbx is
// the type of the sandbox plugin that represents the underlying sandbox
// implementation. This type supports the movement of the sandbox heap after
// creation. If the tainted data is a pointer type, the value is stored as a
// relative offset from the heap base. The offset is added to the heap base
// when the pointer is accessed.
template<typename T, typename TSbx>
class tainted_relocatable
{};
}
