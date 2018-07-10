#pragma once
#ifdef CAE_DEBUG
#include <cstdio>
#include <iostream>

#ifndef DEBUG_NAMESPACE
#define DEBUG_NAMESPACE "UNKNOWN"
#endif

#define DEBUG_ONLY(...) __VA_ARGS__

#define dbg_print(...)\
fprintf(stderr, "[" DEBUG_NAMESPACE "] " "%s:%d :: ", __FILE__, __LINE__);\
fprintf(stderr, __VA_ARGS__)

#define dbg_println(...)\
dbg_print(__VA_ARGS__);\
fprintf(stderr, "\n")

#define dbg_printval(value)\
dbg_print("");\
std::cerr << #value << ": " << value << std::endl

#else
#define DEBUG_ONLY(...)
#define dbg_print(...)
#define dbg_println(...)
#define dbg_printval(value)

#endif
