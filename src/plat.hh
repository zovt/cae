#pragma once

#if defined(unix) || defined(__unix__) || defined(__unix)
#define UNIX
#endif

#ifdef UNIX
#include "plat_unix.hh"
#endif
