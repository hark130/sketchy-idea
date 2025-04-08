#define SKID_KDEBUG             // Enable DEBUG logging

#include <linux/cdev.h>     // cdev_add(), cdev_alloc(), cdev_del(), cdev_init()
// #include <linux/device.h>   // device_create(), device_destroy()
// #include <linux/errno.h>    // errno macros
#include <linux/fs.h>       // file_operations struct, various macros, *_chrdev_region()
// #include <linux/init.h>     // __exit, __init
#include <linux/module.h>   // module_exit(), module_init(), MODULE_* macros
#include <linux/version.h>  // KERNEL_VERSION, LINUX_VERSION_CODE
// #include <sys/lock.h>       // ???
// #include <sys/sema.h>       // sema_destroy(), sema_init()
#include "skid_kdebug.h"


/**************************************************************************************************/
/********************************************* MACROS *********************************************/
/**************************************************************************************************/

#define CDEV_BUFF_SIZE 511       // Character device buffer size
#define DEV_FILENAME "query_me"  // /dev/DEV_FILENAME
#define DEVICE_NAME "Query LKM"  // Use this macro for logging
#define ENOERR ((int)0)          // Success value for errno
#define NUM_MIN_NUMS 1           // Number of minor numbers to register/unregister

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
 *      Remove the global cdev from the system by calling cdev_del() and clears the pointer.
 */
void delete_cdev(void);

/*
 *  Description:
 *      Destroys the global dev_class by calling destroy_device() and clears the pointer.
 */
void destroy_device(void);

int device_close(struct inode *inode, struct file *filp);

int device_open(struct inode *inode, struct file *filp);

ssize_t device_read(struct file *filp, char *buf_store_data, size_t buf_count, loff_t *cur_offset);

ssize_t device_write(struct file *filp, const char *buf_src_data, size_t buf_count,
                     loff_t *cur_offset);

/*
 *  Description:
 *      Intialize the myQueryDevice global.
 */
void init_mqd(void);

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

myQueryDevice my_query_device;          // My Query Device

static struct class *dev_class = NULL;  // Character device class

struct cdev *query_cdev = NULL;         // Character device driver struct

/**************************************************************************************************/
/************************************** FUNCTION DEFINITIONS **************************************/
/**************************************************************************************************/


void delete_cdev(void)
{
    if (NULL != query_cdev)
    {
        cdev_del(query_cdev);  // Remove the cdev from the system
        query_cdev = NULL;     // Clear the pointer
    }
}


void destroy_device(void)
{
    if (NULL != dev_class)
    {
        // Removes a device that was created with device_create
        device_destroy(dev_class, my_query_device.dev_num);
        dev_class = NULL;  // Clear the pointer
    }
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


void init_mqd(void)
{
    memset(my_query_device.log_buf, 0x0, (CDEV_BUFF_SIZE + 1) * sizeof(char));
    my_query_device.buf_len = 0;
    // Initialize the semaphores
    sema_init(&(my_query_device.open_sem), 1);
    sema_init(&(my_query_device.busy_sem), 1);
    my_query_device.dev_num = 0;
    my_query_device.major_num = 0;
    my_query_device.minor_num = 0;
}


static void __exit query_exit(void)
{
    // LOCAL VARIABLES
    unsigned int count = NUM_MIN_NUMS;  // Number of minor numbers to unregister

    // EXIT
    SKID_KINFO(DEVICE_NAME, "exit");
    // Teardown Character Device
    // 1. Destroy the character device
    SKID_KINFO(DEVICE_NAME, "Destroying the character device");  // init() #6
    destroy_device();
    // 2. Remove the cdev
    SKID_KINFO(DEVICE_NAME, "Removing the cdev from the system");
    delete_cdev();  // init() #4 & #5
    // 3. Destroy the device class
    SKID_KINFO(DEVICE_NAME, "Destroying device class");
    class_destroy(dev_class);  // init() #3
    // 4. Unregister the character device
    SKID_KINFO(DEVICE_NAME, "Unregistering the character device");
    unregister_chrdev_region(my_query_device.dev_num, count);  // init() #2

    // DONE
    return;
}


static int __init query_init(void)
{
    // LOCAL VARIABLES
    int result = ENOERR;                // Function call results
    unsigned int baseminor = 0;         // First of the requested range of minor numbers
    unsigned int count = NUM_MIN_NUMS;  // Number of minor numbers to register
    struct device *device = NULL;       // device_create() return value

    // INIT
    SKID_KINFO(DEVICE_NAME, "init");
    // Setup Character Device
    // 1. Prepare the internal struct
    SKID_KINFO(DEVICE_NAME, "Preparing the internal struct");
    init_mqd();  // Initialize the myQueryDevice struct
    // 2. Register the character device
    SKID_KINFO(DEVICE_NAME, "Registering the character device");
    // dynamically chooses the major device number
    result = alloc_chrdev_region(&(my_query_device.dev_num), baseminor, count, DEVICE_NAME);
    if (ENOERR != result)
    {
        SKID_KERROR(DEVICE_NAME, "The call to alloc_chrdev_region() failed");
        SKID_KERRNO(DEVICE_NAME, -result);
    }
    else
    {
        my_query_device.major_num = MAJOR(my_query_device.dev_num);
        my_query_device.minor_num = MINOR(my_query_device.dev_num);
        printk(KERN_INFO "%s received major %d minor %d\n", DEVICE_NAME, my_query_device.major_num,
               my_query_device.minor_num);
    }

    // 3. Create device class (before allocation of the array of devices)
    if (ENOERR == result)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        dev_class = class_create(DEVICE_NAME);
#else
        dev_class = class_create(THIS_MODULE, DEVICE_NAME);
#endif
        if (IS_ERR(dev_class))
        {
            SKID_KERROR(DEVICE_NAME, "The call to class_create() failed");
            result = PTR_ERR(dev_class);
        }
    }

    // 4. Allocate and initialize the character device structure
    if (ENOERR == result)
    {
        // Create our cdev structure
        query_cdev = cdev_alloc();

        // Initialize the cdev structure
        if (NULL == query_cdev)
        {
            SKID_KERROR(DEVICE_NAME,
                        "The call to cdev_alloc() failed to allocate the cdev structure");
            result = -1;
        }
        else
        {
            cdev_init(query_cdev, &fops);
        }
    }

    // 5. Add this character device to the system
    if (ENOERR == result)
    {
        result = cdev_add(query_cdev, my_query_device.dev_num, count);

        if (ENOERR != result)
        {
            SKID_KERROR(DEVICE_NAME, "Unable to add cdev to kernel");
            SKID_KERRNO(DEVICE_NAME, -result);
        }
        else
        {
            // The device is now "live" and can be called by the kernel
            SKID_KINFO(DEVICE_NAME, "Character device is now 'live'");
        }
    }

    // 6. Create a device and register it with sysfs
    if (ENOERR == result)
    {
        device = device_create(dev_class, NULL, my_query_device.dev_num, NULL, DEV_FILENAME);

        if (IS_ERR(device))
        {
            result = PTR_ERR(device);
            SKID_KERROR(DEVICE_NAME, "The call to device_create() failed");
            SKID_KERRNO(DEVICE_NAME, result);
            delete_cdev();
        }
    }

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
