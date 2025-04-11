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

#define CDEV_BUFF_SIZE 511          // Character device buffer size
#define CLASS_NAME "Query Class"    // Query LKM class name
#define DEV_FILENAME "query_me"     // /dev/DEV_FILENAME
#define DEVICE_NAME "Query LKM"     // Use this macro for logging
#define ENOERR ((int)0)             // Success value for errno
#define HELLO_WORLD "Hello World!"  // Hello World!
#define NUM_MIN_NUMS 1              // Number of minor numbers to register/unregister

// IOCTLs
#define QUERY_IOCTL_HELLO_WORLD _IO('q', 1)  // Print "Hello World" to /dev/DEV_FILENAME

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
 *      Clear my_query_device.log_buf and reset the buf_len.
 */
void clear_buf(void);

/*
 *  Description:
 *      Remove the global cdev from the system by calling cdev_del() and clears the pointer.
 */
void delete_cdev(void);

/*
 *  Description:
 *      Destroys the global dev_class by calling class_destroy(), then clears the pointer.
 */
void destroy_class(void);

/*
 *  Description:
 *      Removes a device that was created with device_create() by calling device_destroy().
 */
void destroy_device(void);

/*
 *  Description:
 *      Releases the 'open' semaphore.
 *
 *  Returns:
 *      0 on success, non-zero on failure.
 */
int device_close(struct inode *inode, struct file *filp);

/*
 *  Description:
 *      Obtain the 'open' semaphore.
 *
 *  Returns:
 *      0 on success, -errno value on failure.
 */
int device_open(struct inode *inode, struct file *filp);

/*
 *  Description:
 *      Attempts to read whichever is lower between buf_count and the data-to-read into
 *      buf_store_data.
 *
 *  Returns:
 *      Number of bytes read on success, negative errno on failure.
 */
ssize_t device_read(struct file *filp, char *buf_store_data, size_t buf_count, loff_t *cur_offset);

/*
 *  Description:
 *      Does not allow writes to this read-only character device.
 *
 *  Returns:
 *      -EPERM.
 */
ssize_t device_write(struct file *filp, const char *buf_src_data, size_t buf_count,
                     loff_t *cur_offset);

/*
 *  Description:
 *      Handles IOCTLs on behalf of this LKM.
 *
 *  Returns:
 *      0 on success, -errno values on error.
 */
static long handle_ioctls(struct file *file, unsigned int cmd, unsigned long arg);

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

/*
 *  Description:
 *      Clears myQueryDevice.log_buf and writes msg so users may read it.
 *
 *  Returns:
 *      0 on success, -ENOMEM if the log_buf is not large enough, -errno on failure.
 */
int write_to_buf(void *msg, size_t num_bytes);

/**************************************************************************************************/
/******************************************** GLOBALS *********************************************/
/**************************************************************************************************/

// Device file_operations
struct file_operations fops =
{
    .owner = THIS_MODULE,            // Prevent unloading of this module when operations are in use
    .open = device_open,             // Points to the method to call when opening the device
    .release = device_close,         // Points to the method to call when closing the device
    .write = device_write,           // Points to the method to call when writing to the device
    .read = device_read,             // Points to the method to call when reading from the device
    .unlocked_ioctl = handle_ioctls  // Handles IOCTLs
};

myQueryDevice my_query_device;          // My Query Device

static struct class *dev_class = NULL;  // Character device class

struct cdev *query_cdev = NULL;         // Character device driver struct

/**************************************************************************************************/
/************************************** FUNCTION DEFINITIONS **************************************/
/**************************************************************************************************/


void clear_buf(void)
{
    my_query_device.log_buf[0] = 0x0;  // Truncate current contents
    my_query_device.buf_len = 0;  // Indicate the buffer is empty
}


void delete_cdev(void)
{
    if (NULL != query_cdev)
    {
        cdev_del(query_cdev);  // Remove the cdev from the system
        query_cdev = NULL;     // Clear the pointer
    }
}


void destroy_class(void)
{
    if (NULL != dev_class)
    {
        class_destroy(dev_class);  // Destroy the struct class structure
        dev_class = NULL;  // Clear the pointer
    }
}


void destroy_device(void)
{
    if (NULL != dev_class)
    {
        device_destroy(dev_class, my_query_device.dev_num);  // Remove the query device
    }
}


int device_close(struct inode *inode, struct file *filp)
{
    // LOCAL VARIABLES
    int result = ENOERR;

    // CLOSE IT
    up(&(my_query_device.open_sem));  // Release the open semaphore
    SKID_KINFO(DEVICE_NAME, "Released the 'open' semaphore");

    // DONE
    return result;
}


int device_open(struct inode *inode, struct file *filp)
{
    // LOCAL VARIABLES
    int result = down_interruptible(&(my_query_device.open_sem));  // Result of execution

    // DID IT OPEN?
    if (0 == result)
    {
        SKID_KINFO(DEVICE_NAME, "Successfully obtained the 'open' semaphore");
    }
    else
    {
        SKID_KERROR(DEVICE_NAME, "The call to down_interruptible() failed");
        SKID_KERRNO(DEVICE_NAME, -result);
    }

    // DONE
    return result;
}


ssize_t device_read(struct file *filp, char *buf_store_data, size_t buf_count, loff_t *cur_offset)
{
    // LOCAL VARIABLES
    ssize_t num_bytes = 0;         // Number of bytes read
    unsigned long not_copied = 0;  // Return value from copy_to_user (num bytes *not* copied)
    size_t num_to_read = 0;        // Number of bytes to read: smallest between bufCount and offset
    int i = 0;                     // Iterating variable

    // VALIDATION
    if (NULL == filp || NULL == buf_store_data || NULL == cur_offset)
    {
        SKID_KERROR(DEVICE_NAME, "Received a NULL pointer");
        num_bytes = -EINVAL;  // NULL pointer
    }
    else if (buf_count < 1)
    {
        SKID_KERROR(DEVICE_NAME, "Invalid destination buffer size");
        num_bytes = -EINVAL;  // NULL pointer
    }
    // READ IT
    else if (my_query_device.buf_len > 0)
    {
        SKID_KINFO(DEVICE_NAME, "Reading from device");
        num_to_read = buf_count < my_query_device.buf_len ? buf_count : my_query_device.buf_len;

        not_copied = copy_to_user(buf_store_data, my_query_device.log_buf, num_to_read);

        // Success
        if (0 == not_copied)
        {
            // Everything was read
            if (num_to_read == my_query_device.buf_len)
            {
                SKID_KINFO(DEVICE_NAME, "Total read executed");
                num_bytes = num_to_read;
                clear_buf();  // Clear the log_buf
            }
            // Partial read
            else
            {
                SKID_KINFO(DEVICE_NAME, "Partial read executed");
                // Save the return value
                num_bytes = num_to_read;
                // Move everything to the front
                while (num_to_read < my_query_device.buf_len)
                {
                    my_query_device.log_buf[i] = my_query_device.log_buf[num_to_read];
                    i++;
                    num_to_read++;
                }
                // Truncate it
                my_query_device.log_buf[num_to_read] = 0x0;
                // Update the buffer length
                my_query_device.buf_len = i;
            }
        }
        // Error condition
        else
        {
            SKID_KERROR(DEVICE_NAME, "copy_to_user() failed to copy all the bytes");
            num_bytes = num_to_read - not_copied;  // Return the number of bytes read
        }
    }
    else
    {
        SKID_KINFO(DEVICE_NAME, "Nothing to read");
        num_bytes = 0;  // Nothing was read
    }

    // DONE
    return num_bytes;
}


ssize_t device_write(struct file *filp, const char *buf_src_data, size_t buf_count,
                     loff_t *cur_offset)
{
    // LOCAL VARIABLES
    ssize_t result = -EPERM;  // Writes are not permitted

    // DONE
    return result;
}


static long handle_ioctls(struct file *file, unsigned int cmd, unsigned long arg)
{
    // LOCAL VARIABLES
    long result = ENOERR;  // Results of execution

    // HANDLE IT
    printk(KERN_INFO "%s: Received IOCTL %u\n", DEBUG_INFO_STR, cmd);  // DEBUGGING
    printk(KERN_INFO "%s: Expecting IOCTL %u\n", DEBUG_INFO_STR, QUERY_IOCTL_HELLO_WORLD);  // DEBUGGING
    switch (cmd)
    {
        case QUERY_IOCTL_HELLO_WORLD:
            result = write_to_buf(HELLO_WORLD, strlen(HELLO_WORLD) * sizeof(char));
            break;
        default:
            SKID_KERROR(DEVICE_NAME, "Unsupported IOCTL");
            result = -ENOTTY;
    }

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
    /*
     *  DEBUGGING
     */
    write_to_buf("Hello there?", strlen("Hello there?") * sizeof(char));
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
    destroy_class();  // init() #3
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
        dev_class = class_create(CLASS_NAME);
#else
        dev_class = class_create(THIS_MODULE, CLASS_NAME);
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


int write_to_buf(void *msg, size_t num_bytes)
{
    // LOCAL VARIABLES
    int result = ENOERR;  // Result of execution

    // INPUT VALIDATION
    if (NULL == msg)
    {
        result = -EINVAL;
    }
    else if (num_bytes > CDEV_BUFF_SIZE)
    {
        result = -ENOMEM;  // Not enough space
    }

    // WRITE IT
    // Get the 'busy' semaphore
    if (ENOERR == result)
    {
        result = down_interruptible(&(my_query_device.busy_sem));
        // Did we get it?
        if (0 == result)
        {
            SKID_KINFO(DEVICE_NAME, "Successfully obtained the 'busy' semaphore");
        }
        else
        {
            result = -result;
            SKID_KERROR(DEVICE_NAME, "The call to down_interruptible() failed");
            SKID_KERRNO(DEVICE_NAME, result);
        }
    }
    // Write to the log_buf
    if (ENOERR == result)
    {
        // Clear the log_buf first
        clear_buf();
        // Copy the data in
        memcpy(my_query_device.log_buf, msg, num_bytes);
        // Set the new buf_len
        my_query_device.buf_len = num_bytes;
    }
    up(&(my_query_device.busy_sem));  // Release the 'busy' semaphore
    SKID_KINFO(DEVICE_NAME, "Released the 'busy' semaphore");

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
