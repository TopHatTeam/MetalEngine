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

	/**
	* @brief Used for creating a warning message box on windows
	* @param title -> The title of the message box
	* @param msg -> The message you want displayed in the message box
	* @returns void
	*/
	void WarningMessage(const char* title, const char* msg, ...);

	/**
	* @brief Used for creating a error message box on windows
	* @param title -> The title of the message box
	* @param msg -> The message you want displayed 
	* @returns void
	*/
	void FatalError(const char* title, const char* msg, ...);

	/**
	* @brief Used for creating a message in the operating system terminal
	* @param msg -> The message you want displayed in the terminal
	* @returns void
	*/
	void ConsoleLog(const char* msg, ...);

#ifdef __cplusplus
}
#endif 

#endif