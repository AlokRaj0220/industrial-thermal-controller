#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "thermal_controller_ioctl.h"

#define DEVICE_NAME "thermal_controller"
#define CLASS_NAME "thermal_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antigravity");
MODULE_DESCRIPTION("A skeleton character driver for Industrial Thermal Controller");
MODULE_VERSION("0.1");

static int major_number;
static struct class *thermal_class = NULL;
static struct device *thermal_device = NULL;
static struct cdev thermal_cdev;

// Simulated registers/state
static struct thermal_state sim_state;

static int dev_open(struct inode *inodep, struct file *filep)
{
    pr_info("thermal_controller: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    pr_info("thermal_controller: Device successfully closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    int bytes_read = 0;
    char status_str[128];
    int msg_len;

    msg_len = snprintf(status_str, sizeof(status_str),
                       "Temp: %d, Fan: %d, Target: %d, Alarm: %d\n",
                       sim_state.temperature, sim_state.fan_speed,
                       sim_state.target_temp, sim_state.alarm_active);

    if (*offset >= msg_len)
        return 0; // EOF

    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (copy_to_user(buffer, status_str + *offset, len)) {
        return -EFAULT;
    }

    *offset += len;
    bytes_read = len;

    pr_info("thermal_controller: Read %d bytes from device\n", bytes_read);
    return bytes_read;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    char kbuf[128] = {0};
    int write_len = len < (sizeof(kbuf) - 1) ? len : (sizeof(kbuf) - 1);

    if (copy_from_user(kbuf, buffer, write_len)) {
        return -EFAULT;
    }

    // For simplicity, just log the string written
    pr_info("thermal_controller: Received %zu bytes from user: %s\n", len, kbuf);

    return len;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    __s32 temp_val;

    switch (cmd) {
    case THERMAL_GET_STATE:
        if (copy_to_user((struct thermal_state __user *)arg, &sim_state, sizeof(struct thermal_state)))
            return -EFAULT;
        pr_info("thermal_controller: THERMAL_GET_STATE called\n");
        break;

    case THERMAL_SET_TARGET_TEMP:
        if (copy_from_user(&temp_val, (__s32 __user *)arg, sizeof(__s32)))
            return -EFAULT;
        sim_state.target_temp = temp_val;
        pr_info("thermal_controller: THERMAL_SET_TARGET_TEMP set to %d\n", temp_val);
        break;

    case THERMAL_SET_FAN_SPEED:
        if (copy_from_user(&temp_val, (__s32 __user *)arg, sizeof(__s32)))
            return -EFAULT;
        sim_state.fan_speed = temp_val;
        pr_info("thermal_controller: THERMAL_SET_FAN_SPEED set to %d\n", temp_val);
        break;

    case THERMAL_RESET:
        sim_state.temperature = 25000;
        sim_state.fan_speed = 1000;
        sim_state.target_temp = 30000;
        sim_state.alarm_active = 0;
        pr_info("thermal_controller: THERMAL_RESET called\n");
        break;

    default:
        pr_err("thermal_controller: Invalid IOCTL command\n");
        return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .release = dev_release,
};

static int __init thermal_init(void)
{
    int ret;
    dev_t dev_num;

    pr_info("thermal_controller: Initializing the thermal controller module\n");

    // Initialize simulated state
    sim_state.temperature = 25000; // 25.0 C
    sim_state.fan_speed = 1000;    // 1000 RPM
    sim_state.target_temp = 30000; // 30.0 C
    sim_state.alarm_active = 0;

    // Allocate a major number dynamically
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("thermal_controller: Failed to allocate major number\n");
        return ret;
    }
    major_number = MAJOR(dev_num);
    pr_info("thermal_controller: Registered correctly with major number %d\n", major_number);

    // Register the device class
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    thermal_class = class_create(CLASS_NAME);
    #else
    thermal_class = class_create(THIS_MODULE, CLASS_NAME);
    #endif
    if (IS_ERR(thermal_class)) {
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        pr_err("thermal_controller: Failed to register device class\n");
        return PTR_ERR(thermal_class);
    }
    pr_info("thermal_controller: Device class registered correctly\n");

    // Initialize the cdev structure and add it to the kernel
    cdev_init(&thermal_cdev, &fops);
    ret = cdev_add(&thermal_cdev, MKDEV(major_number, 0), 1);
    if (ret < 0) {
        class_destroy(thermal_class);
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        pr_err("thermal_controller: Failed to add cdev\n");
        return ret;
    }
    pr_info("thermal_controller: cdev added correctly\n");

    // Register the device driver
    thermal_device = device_create(thermal_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(thermal_device)) {
        cdev_del(&thermal_cdev);
        class_destroy(thermal_class);
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        pr_err("thermal_controller: Failed to create the device\n");
        return PTR_ERR(thermal_device);
    }
    pr_info("thermal_controller: Device created correctly\n");

    return 0;
}

static void __exit thermal_exit(void)
{
    cdev_del(&thermal_cdev);
    device_destroy(thermal_class, MKDEV(major_number, 0));
    class_destroy(thermal_class);
    unregister_chrdev_region(MKDEV(major_number, 0), 1);
    pr_info("thermal_controller: Goodbye from the LKM!\n");
}

module_init(thermal_init);
module_exit(thermal_exit);
