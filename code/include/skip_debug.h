/*
 *	#define SKIP_DEBUG
 *	...in order to enable DEBUGGING output.
 */

#ifndef __SKIP_DEBUG_H__
#define __SKIP_DEBUG_H__

#ifndef __FUNCTION_NAME__
    #ifdef WIN32  // Windows
        #define __FUNCTION_NAME__ __FUNCTION__  
    #else         // *nix
        #define __FUNCTION_NAME__ __func__ 
    #endif
#endif  /* __FUNCTION_NAME__ */

#ifdef SKIP_DEBUG
#include <string.h>  // strerror()
#define PRINT_ERRNO(errorNum) if (errorNum) { fprintf(stderr, "<<<ERROR>>> - %s - %s() returned errno:\t%s\n", __FILE__, __FUNCTION_NAME__, strerror(errorNum)); };
#define PRINT_ERROR(msg) do { fprintf(stderr, "<<<ERROR>>> - %s - %s() - %s!\n", __FILE__, __FUNCTION_NAME__, #msg); } while (0);
#define PRINT_WARNG(msg) do { fprintf(stderr, "¿¿¿WARNING??? - %s - %s() - %s!\n", __FILE__, __FUNCTION_NAME__, #msg); } while (0);
#else
#define PRINT_ERRNO(errorNum) ;;;
#define PRINT_ERROR(msg) ;;;
#define PRINT_WARNG(msg) ;;;
#endif  /* SKIP_DEBUG */

#endif  /* __SKIP_DEBUG_H__ */
