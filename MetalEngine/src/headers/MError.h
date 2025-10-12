// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine error system (Operating System specific)
// ------------------------------------------------------

#ifndef MERROR_H
#define MERROR_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>

#if defined(WIN32)
	#include <Windows.h>
#elif defined(__linux__)

#endif

	void FatalError(const char* title, const char* msg, ...);
	void ConsoleLog(const char* msg, ...);

#ifdef __cplusplus
}
#endif 

#endif