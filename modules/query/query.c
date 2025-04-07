#define SKID_KDEBUG             // Enable DEBUG logging

#include <linux/errno.h>    // errno macros
#include <linux/fs.h>       // file_operations struct
#include <linux/init.h>     // __exit, __init
#include <linux/module.h>   // module_exit(), module_init()
// #include <sys/lock.h>       // ???
// #include <sys/sema.h>       // sema_destroy(), sema_init()
#include "skid_kdebug.h"


/**************************************************************************************************/
/********************************************* MACROS *********************************************/
/**************************************************************************************************/

#define ENOERR ((int)0)          // Success value for errno
#define DEVICE_NAME "Query LKM"  // Use this macro for logging
#define DEV_FILENAME "query_me"  // /dev/DEV_FILENAME
#define CDEV_BUFF_SIZE 511       // Character device buffer size

/**************************************************************************************************/
/******************************************** TYPEDEFS ********************************************/
/**************************************************************************************************/

typedef struct _myQueryDevice
{
    char log_buf[CDEV_BUFF_SIZE + 1];  // Log buffer
    size_t buf_len;                    // Current log length
    struct semaphore open_sem;         // No more than one user may open the device at a time
    struct semaphore busy_sem;         // One modifcation to the buffer at a time
    dev_t dev_num;                     // Will hold device number that kernel gives us
    int major_num;                     // Major number assigned to the device
    int minor_num;                     // Minor number assigned to the device
} myQueryDevice, *myQueryDevice_ptr;

/**************************************************************************************************/
/************************************* FUNCTION DECLARATIONS **************************************/
/**************************************************************************************************/

/*
 *  Description:
 *      TO DO: DON'T DO NOW...
 *
 *      The __exit macro causes the omission of the function when the module is built
 *      into the kernel.
 *
 *  Args:
 *      errnum: [Out] Storage location for errno values encountered.
 *      opt_arg: Specifies the data type of fcntl()'s optional third argument.
 *          Use FcntlOptArg_t.Void for no argument.
 *      fd: Open file descriptor to pass to fcntl().
 *      cmd: The fnctl() operation to execute.  See: fcntl(2).
 *      ... Some fcntl() cmds take a third argument.  See: fcntl(2).
 *
 *  Returns:
 *      For a successful call, the return value depends on the operation.  See: fcntl(2).
 *      On error, -1 is returned, and errnum is set appropriately.
 */
static void __exit query_exit(void);

/*
 *  Description:
 *      TO DO: DON'T DO NOW...
 *
 *      The __init macro causes the init function to be discarded and its memory freed
 *      once the init function finishes for built-in drivers, but not loadable modules.
 *
 */
static int __init query_init(void);

int device_close(struct inode *inode, struct file *filp);

int device_open(struct inode *inode, struct file *filp);

ssize_t device_read(struct file *filp, char *buf_store_data, size_t buf_count, loff_t *cur_offset);

ssize_t device_write(struct file *filp, const char *buf_src_data, size_t buf_count,
                     loff_t *cur_offset);

/**************************************************************************************************/
/******************************************** GLOBALS *********************************************/
/**************************************************************************************************/

// Device file_operations
struct file_operations fops =
{
    .owner = THIS_MODULE,     // Prevent unloading of this module when operations are in use
    .open = device_open,      // Points to the method to call when opening the device
    .release = device_close,  // Points to the method to call when closing the device
    .write = device_write,    // Points to the method to call when writing to the device
    .read = device_read       // Points to the method to call when reading from the device
};

myQueryDevice my_query_device;  // My Query Device

/**************************************************************************************************/
/************************************** FUNCTION DEFINITIONS **************************************/
/**************************************************************************************************/


static void __exit query_exit(void)
{
    // EXIT
    SKID_KINFO(DEVICE_NAME, "LKM exit");
    // Teardown Character Device
    // 1. Unregister the character device
    SKID_KINFO(DEVICE_NAME, "Unregistering the character device");

    // DONE
    return;
}


static int __init query_init(void)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Function call results

    // INIT
    SKID_KINFO(DEVICE_NAME, "LKM init");
    // Setup Character Device
    // 1. Prepare the internal struct
    SKID_KINFO(DEVICE_NAME, "Preparing the internal struct");
    memset(my_query_device.log_buf, 0x0, (CDEV_BUFF_SIZE + 1) * sizeof(char));
    // Initialize the semaphores
    sema_init(&my_query_device.open_sem, 1);
    sema_init(&my_query_device.busy_sem, 1);

    // 2. Register the character device
    // dynamically chooses the major device number
    SKID_KINFO(DEVICE_NAME, "Registering the character device");


// 65 #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
// 66 cls = class_create(DEVICE_NAME);
// 67 #else
// 68 cls = class_create(THIS_MODULE, DEVICE_NAME);
// 69 #endif
// 70 device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
 

    // DONE
    return result;
}


int device_close(struct inode *inode, struct file *filp)
{
    // LOCAL VARIABLES
    int result = ENOERR;

    // DONE
    return result;
}


int device_open(struct inode *inode, struct file *filp)
{
    // LOCAL VARIABLES
    int result = ENOERR;

    // DONE
    return result;
}


ssize_t device_read(struct file *filp, char *buf_store_data, size_t buf_count, loff_t *cur_offset)
{
    // LOCAL VARIABLES
    ssize_t result = 0;

    // DONE
    return result;
}


ssize_t device_write(struct file *filp, const char *buf_src_data, size_t buf_count,
                     loff_t *cur_offset)
{
    // LOCAL VARIABLES
    ssize_t result = 0;

    // DONE
    return result;
}


// Tell the Linux kernel where to start executing your Linux module
// sudo insmod query.ko
module_init(query_init);
// Verify the module is loaded with...
// TO DO: DON'T DO NOW
// Modules typically live in /lib/modules
// sudo rmmod query.ko
module_exit(query_exit);

// LKM Descriptive Information
MODULE_AUTHOR("Joseph Harkleroad");
MODULE_DESCRIPTION("IOCTL Queries");
MODULE_LICENSE("GPL");
