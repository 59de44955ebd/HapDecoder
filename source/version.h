#pragma once								

#define _VER_BUILDNUM 0							
#define _VER_BUILDNUM_STRING "0"					

#define _VER_MAJORVERSION 0				    
#define _VER_MAJORVERSION_STRING "0"

#define _VER_MINORVERSION 1
#define _VER_MINORVERSION_STRING "1"     
														
#define _VER_BUGFIXVERSION 0
#define _VER_BUGFIXVERSION_STRING "0"

#if _VER_BUGFIXVERSION > 0
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING "." _VER_MINORVERSION_STRING "." _VER_BUGFIXVERSION_STRING 
#else
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING "." _VER_MINORVERSION_STRING 
#endif

// following are used in version info for the windows resource 
#define _VER_ORIGINALFILENAME	"HapDecoder.ax"
#define _VER_COMPANY			"Valentin Schmidt"
#define _VER_FILEDESCRIPTION	"HapDecoder DirectShow Filter"
#define _VER_PRODUCTNAME		"HapDecoder"
#define _VER_INTERNALNAME		"HapDecoder.ax"
#define _VER_COPYRIGHT "(c) 2018"
