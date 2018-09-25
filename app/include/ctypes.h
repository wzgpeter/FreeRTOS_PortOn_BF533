
#ifndef __CTYPES_H
#define __CTYPES_H

// #define DEVELOPMENT_AMP

//#ifdef __ECC__

#define U8_TO_S8(a) (((S8) a << 24) >> 24)

typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;
typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned int	UINT32;
typedef unsigned char	Uint8;
typedef unsigned short	Uint16;
typedef unsigned int	Uint32;

typedef signed char		S8;
typedef signed short  	S16;
typedef signed int		S32;
typedef signed char		INT8;
typedef signed short  	INT16;
typedef signed int		INT32;
typedef signed char		Int8;
typedef signed short  	Int16;
typedef signed int		Int32;


#ifndef dword
typedef unsigned int  dword;
#endif

#ifndef word
typedef unsigned short  word;
#endif

#ifndef byte
typedef unsigned char   byte;
#endif
typedef bool BOOL;


#define PRIVATE static
#define PUBLIC  

#define OK				(0)
#define FAIL			(1)

// Note: MOST NetServices do define the following macros in the same way.
// 		 so we added the #ifdefs in order to avoid compiler warnings.

#ifndef TRUE
#define TRUE			(1)
#endif

#ifndef FALSE
#define FALSE			(0)
#endif

#ifndef ON
#define ON				(1)
#endif

#ifndef OFF						
#define OFF				(0)
#endif

#define SSI_ENABLED

//#endif

#endif
