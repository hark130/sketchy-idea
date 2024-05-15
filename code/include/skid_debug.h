/*
 *	#define SKID_DEBUG
 *	...in order to enable DEBUG logging.
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
#include <stdio.h>   // fprintf()
#include <string.h>  // strerror()
#define DEBUG_ERROR_STR "<<<ERROR>>>"
#define DEBUG_INFO_STR "[INFO]"
#define DEBUG_WARNG_STR "¿¿¿WARNING???"
#define PRINT_ERRNO(errorNum) if (errorNum) { fprintf(stderr, "%s - %s - %s() - line %d - Returned errno [%d]: %s\n", DEBUG_ERROR_STR, __FILE__, __FUNCTION_NAME__, __LINE__, errorNum, strerror(errorNum)); };
#define PRINT_ERROR(msg) do { fprintf(stderr, "%s - %s - %s() - line %d - %s!\n", DEBUG_ERROR_STR, __FILE__, __FUNCTION_NAME__, __LINE__, #msg); } while (0);
#define PRINT_WARNG(msg) do { fprintf(stderr, "%s - %s - %s() - line %d - %s!\n", DEBUG_WARNG_STR, __FILE__, __FUNCTION_NAME__, __LINE__, #msg); } while (0);
#define FPRINTF_ERR(...) do { fprintf(stderr, __VA_ARGS__); } while (0);
#else
#define DEBUG_ERROR_STR ""
#define DEBUG_INFO_STR ""
#define DEBUG_WARNG_STR ""
#define PRINT_ERRNO(errorNum) ;;;
#define PRINT_ERROR(msg) ;;;
#define PRINT_WARNG(msg) ;;;
#define FPRINTF_ERR(...) ;;;
#endif  /* SKID_DEBUG */

#endif  /* __SKID_DEBUG_H__ */
