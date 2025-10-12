// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine MacOS support 
// ------------------------------------------------------

#import "headers/macos/MMac.h"

extern "C" void FatalErrorApple(const char* msg, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, msg);
	vsnprintf(buffer, sizeof(buffer), msg, args);
	va_end(args);

	@autoreleasepool
	{
		NSAlert* alert = [[NSAlert alloc] init];
		[alert setMessageText:@"ERROR"];
		[alert setInformativeText:[NSString stringWithUTF8String:buffer]];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
	}
}