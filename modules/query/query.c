#define SKID_KDEBUG             // Enable DEBUG logging

#include <linux/init.h>
#include <linux/module.h>
#include "skid_kdebug.h"


#define DEVICE_NAME "Query LKM"  // Use this macro for logging
#define DEV_FILENAME "query_me"  // /dev/DEV_FILENAME


// NOTES:
//  Static because there can be only one?
static int query_init(void)
{
    SKID_KINFO(DEVICE_NAME, "LKM init");
    return 0;
}

static void query_exit(void)
{
    SKID_KINFO(DEVICE_NAME, "LKM exit");
    return;
}

// Tell the Linux kernel where to start executing your Linux module
// sudo insmod query.ko
module_init(query_init);
// Verify the module is loaded with...
// TO DO: DON'T DO NOW
// Modules typically live in /lib/modules
// sudo rmmod query.ko
module_exit(query_exit);

// DESCRIPTIVE INFORMATION ABOUT THIS MODULE
MODULE_AUTHOR("Joseph Harkleroad");
MODULE_DESCRIPTION("IOCTL Queries");
MODULE_LICENSE("GPL");
