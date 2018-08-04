#pragma once
#pragma warning(disable : 4996)

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define DBGS(X) OutputDebugStringA(X)
//#define DBGI(X) {char dbg[16];sprintf(dbg,"%ld",X);OutputDebugStringA(dbg);}
//#define DBGW(X) OutputDebugStringW(X)

//#undef NOTE
//#define NOTE(X) OutputDebugStringA(X)

#undef ASSERT
#define ASSERT(X) assert(X)
