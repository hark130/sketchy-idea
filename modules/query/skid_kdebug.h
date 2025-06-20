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
#define SKID_KERROR(module, errMsg) do { printk(KERN_ERR "%s: %s %s - %s\n", DEBUG_ERROR_STR, module, __func__, errMsg); } while (0);
#define SKID_KERRNO(module, errNum) do { printk(KERN_ERR "%s: %s %s() returned errno: %d!\n", DEBUG_ERROR_STR, module, __func__, errNum); } while (0);
#define SKID_KWARNG(module, warnMsg) do { printk(KERN_WARNING "%s: %s %s() - %s!\n", DEBUG_WARNG_STR, module, __func__, warnMsg); } while (0);
#define SKID_KFINFO(module, msg) do { printk(KERN_INFO "%s: %s %s - %s\n", DEBUG_INFO_STR, module, __func__, msg); } while (0);
#define SKID_KINFO(module, msg) do { printk(KERN_INFO "%s: %s %s\n", DEBUG_INFO_STR, module, msg); } while (0);
#else
#define DEBUG_ERROR_STR ""
#define DEBUG_INFO_STR ""
#define DEBUG_WARNG_STR ""
#define SKID_KERROR(module, errMsg) ;;;
#define SKID_KERRNO(module, errNum) ;;;
#define SKID_KWARNG(module, warnMsg) ;;;
#define SKID_KFINFO(module, msg) ;;;
#define SKID_KINFO(module, msg) ;;;
#endif  /* SKID_KDEBUG */

#endif  /* __SKID_KDEBUG_H__ */
