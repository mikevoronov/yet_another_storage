#pragma once

namespace yas {
namespace macros {

#ifdef _MSC_VER
#define STRUCT_PACK(...) \
__pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#elif defined(__GNUC__)
#define STRUCT_PACK(...) \
__VA_ARGS__ __attribute__((packed))
#elif 
static_assert(false, "You used not VS and GCC compiler - please add STRUCT_PACK by supported way of your compiler");
#endif 

} // namespace macros
} // namespace yas
