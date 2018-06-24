#pragma once

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#ifdef CAE_DEBUG
#define DEBUG_ONLY(src) src
#else
#define DEBUG_ONLY(src)
#endif

