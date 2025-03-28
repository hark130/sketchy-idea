/*
 *    #define SKID_KDEBUG
 *    ...in order to enable KERNEL DEBUG logging.
 */

#include <linux/printk.h>                       // printk()

#ifndef __SKID_KDEBUG_H__
#define __SKID_KDEBUG_H__

#ifdef SKID_KDEBUG
#define DEBUG_ERROR_STR "<<<ERROR>>>"
#define DEBUG_INFO_STR "[INFO]"
#define DEBUG_WARNG_STR "¿¿¿WARNING???"
#define SKID_KERROR(module, funcName, errMsg) do { printk(KERN_ERR "%s: %s %s - %s\n", DEBUG_ERROR_STR, module, #funcName, errMsg); } while (0);
#define SKID_KERRNO(module, funcName, errNum) do { printk(KERN_ERR "%s: %s %s() returned errno: %d!\n", DEBUG_ERROR_STR, module, #funcName, errNum); } while (0);
#define SKID_KWARNG(module, funcName, warnMsg) do { printk(KERN_WARNING "%s: %s %s() - %s!\n", DEBUG_WARNG_STR, module, #funcName, warnMsg); } while (0);
#define SKID_KFINFO(module, funcName, msg) do { printk(KERN_INFO "%s: %s %s - %s\n", DEBUG_INFO_STR, module, #funcName, msg); } while (0);
#define SKID_KINFO(module, msg) do { printk(KERN_INFO "%s: %s %s\n", DEBUG_INFO_STR, module, msg); } while (0);
#else
#define DEBUG_ERROR_STR ""
#define DEBUG_INFO_STR ""
#define DEBUG_WARNG_STR ""
#define SKID_KERROR(module, funcName, errMsg) ;;;
#define SKID_KERRNO(module, funcName, errNum) ;;;
#define SKID_KWARNG(module, funcName, warnMsg) ;;;
#define SKID_KFINFO(module, funcName, msg) ;;;
#define SKID_KINFO(module, msg) ;;;
#endif  /* SKID_KDEBUG */

#endif  /* __SKID_KDEBUG_H__ */
