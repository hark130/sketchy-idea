/*
 *	#define SKID_DEBUG
 *	...in order to enable DEBUGGING output.
 */

#ifndef __SKID_DEBUG_H__
#define __SKID_DEBUG_H__

#ifndef __FUNCTION_NAME__
    #ifdef WIN32  // Windows
        #define __FUNCTION_NAME__ __FUNCTION__  
    #else         // *nix
        #define __FUNCTION_NAME__ __func__ 
    #endif
#endif  /* __FUNCTION_NAME__ */

#ifdef SKID_DEBUG
#include <string.h>  // strerror()
#define PRINT_ERRNO(errorNum) if (errorNum) { fprintf(stderr, "<<<ERROR>>> - %s - %s() - %d - Returned errno: %s\n", __FILE__, __FUNCTION_NAME__, __LINE__, strerror(errorNum)); };
#define PRINT_ERROR(msg) do { fprintf(stderr, "<<<ERROR>>> - %s - %s() - %d - %s!\n", __FILE__, __FUNCTION_NAME__, __LINE__, #msg); } while (0);
#define PRINT_WARNG(msg) do { fprintf(stderr, "¿¿¿WARNING??? - %s - %s() - %d - %s!\n", __FILE__, __FUNCTION_NAME__, __LINE__, #msg); } while (0);
#define FPRINTF_ERR(...) do { fprintf(stderr, __VA_ARGS__); } while (0);
#else
#define PRINT_ERRNO(errorNum) ;;;
#define PRINT_ERROR(msg) ;;;
#define PRINT_WARNG(msg) ;;;
#define FPRINTF_ERR(...) ;;;
#endif  /* SKID_DEBUG */

#endif  /* __SKID_DEBUG_H__ */
