// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine error system (Operating System specific)
// ------------------------------------------------------

#include "headers/MError.h"

void FatalError(const char* title, const char* msg, ...)
{
	char buffer[1024];

	va_list args;										// <-- Declare a variable to hold the variable arguments*/
	va_start(args, msg);								// <-- Initialize 'args' to point to the first argument after 'msg'
	vsnprintf(buffer, sizeof(buffer), msg, args);		// <-- Write formatted string into buffer
	va_end(args);										// <-- Clean up the variable argument list

	MessageBox(NULL, msg, title, MB_OK | MB_ICONERROR);	// <-- Show the error message box
}

void ConsoleLog(const char* msg, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, msg);
	vsnprintf(buffer, sizeof(buffer), msg, args);
	va_end(args);

	printf(msg);
}