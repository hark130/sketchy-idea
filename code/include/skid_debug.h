/*
 *    #define SKID_DEBUG
 *    ...in order to enable DEBUG logging.
 *
 *  USAGE:
 *      PRINT_ERRNO(errno);  // Print the error number and its description
 *      PRINT_ERROR(Something went wrong);  // Print an error message
 *      PRINT_WARNG(Is this right?);  // Print a warning message
 *      FPRINTF_ERR("Error inside %s\n", __FUNCTION_NAME__);  // Use printf-style format in messages
 *      // Utilize MODULE_*LOAD() for basic runtime diagnostics
 *      // Print out metadata at load/unload time (shared object or program start/end):
 *      MODULE_LOAD();  // Print the module name being loaded using the gcc constructor attribute
 *      MODULE_UNLOAD();  // Print the module name being unloaded using the gcc destructor attribute
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
#if (defined(__GNUC__) || defined(__clang__))
#define MODULE_LOAD() __attribute__((constructor)) static void print_skid_load(void) { fprintf(stderr, "%s Module loaded: %s\n", DEBUG_INFO_STR, __FILE__); }
#define MODULE_UNLOAD() __attribute__((destructor)) static void print_skid_unload(void) { fprintf(stderr, "%s Module unloaded: %s\n", DEBUG_INFO_STR, __FILE__); }
#else
#define MODULE_LOAD() ;;;  // gcc attributes are not available with this compiler
#define MODULE_UNLOAD() ;;;  // gcc attributes are not available with this compiler
#endif  /* MODULE_LOAD(), MODULE_UNLOAD() */
#else
#define DEBUG_ERROR_STR ""
#define DEBUG_INFO_STR ""
#define DEBUG_WARNG_STR ""
#define PRINT_ERRNO(errorNum) ;;;
#define PRINT_ERROR(msg) ;;;
#define PRINT_WARNG(msg) ;;;
#define FPRINTF_ERR(...) ;;;
#define MODULE_LOAD() ;;;
#define MODULE_UNLOAD() ;;;
#endif  /* SKID_DEBUG */

#endif  /* __SKID_DEBUG_H__ */
