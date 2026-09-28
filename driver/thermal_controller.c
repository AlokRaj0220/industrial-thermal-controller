#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>

#include "thermal_controller_ioctl.h"

#define DEVICE_NAME "thermal_controller"
#define CLASS_NAME "thermal_class"
#define MAX_CHANNELS 4

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

// New Hardware simulation registers
static uint16_t adc_millivolts[MAX_CHANNELS];
static uint16_t pwm_duty[MAX_CHANNELS];
static uint32_t tach_pulses[MAX_CHANNELS];
static uint32_t sim_zone_load[MAX_CHANNELS];

// Kernel timer for physics simulation
static struct timer_list sim_timer;
static spinlock_t sim_lock;
#define TIMER_INTERVAL_MS 500

// Initialize default register values
static void init_sim_registers(void)
{
    int i;
    for (i = 0; i < MAX_CHANNELS; ++i) {
        adc_millivolts[i] = 3284; // roughly 25C based on NTC math (cooling applied -> ADC increases)
        pwm_duty[i] = 0;
        tach_pulses[i] = 0;
        sim_zone_load[i] = 0;
    }
}

// Timer callback simulating physics updates
static void sim_timer_callback(struct timer_list *t)
{
    int i;
    unsigned long flags;

    spin_lock_irqsave(&sim_lock, flags);

    for (i = 0; i < MAX_CHANNELS; ++i) {
        // Tach pulses: assuming 3000 RPM at 100% (255) duty
        // 3000 RPM = 50 rev/sec. Over 500ms = 25 revs. 
        // Assume 2 pulses per rev -> max 50 pulses per 500ms.
        uint32_t pulses = (pwm_duty[i] * 50) / 255;
        tach_pulses[i] += pulses;

        // Thermal physics (integer-only approximation)
        int net_drift = 0;

        // Simulated load increases temperature -> ADC decreases
        if (sim_zone_load[i] > 0) {
            net_drift -= sim_zone_load[i];
        }

        // PWM fan cooling decreases temperature -> ADC increases
        if (pwm_duty[i] > 0) {
            net_drift += (pwm_duty[i] / 25);
        }

        // No load + no fan should remain at or gradually return toward ambient 3284 mV
        if (net_drift == 0 && adc_millivolts[i] < 3284) {
            net_drift = 1;
        }

        adc_millivolts[i] += net_drift;

        // Ensure bounds are maintained
        if (adc_millivolts[i] > 3284) {
            adc_millivolts[i] = 3284;
        }
        if (adc_millivolts[i] < 3000) {
            adc_millivolts[i] = 3000;
        }
    }

    // Sync legacy state loosely to channel 0 for backward compatibility
    sim_state.fan_speed = (pwm_duty[0] * 3000) / 255;
    
    // Legacy temperature: map 3284 mV -> 25000 mC, 3000 mV -> ~85000 mC roughly 
    // Just a placeholder linear conversion so values change
    sim_state.temperature = 25000 + (3284 - adc_millivolts[0]) * 200; 

    spin_unlock_irqrestore(&sim_lock, flags);

    // Reschedule timer
    mod_timer(&sim_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
}

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
    unsigned long flags;

    spin_lock_irqsave(&sim_lock, flags);
    msg_len = snprintf(status_str, sizeof(status_str),
                       "Temp: %d, Fan: %d, Target: %d, Alarm: %d\n",
                       sim_state.temperature, sim_state.fan_speed,
                       sim_state.target_temp, sim_state.alarm_active);
    spin_unlock_irqrestore(&sim_lock, flags);

    if (*offset >= msg_len)
        return 0; // EOF

    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (copy_to_user(buffer, status_str + *offset, len)) {
        return -EFAULT;
    }

    *offset += len;
    bytes_read = len;

    return bytes_read;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    char kbuf[128] = {0};
    int write_len = len < (sizeof(kbuf) - 1) ? len : (sizeof(kbuf) - 1);

    if (copy_from_user(kbuf, buffer, write_len)) {
        return -EFAULT;
    }

    pr_info("thermal_controller: Received %zu bytes from user: %s\n", len, kbuf);

    return len;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    __s32 temp_val;
    struct thermal_channel_data ch_data;
    unsigned long flags;

    switch (cmd) {
    // --- Legacy IOCTLs ---
    case THERMAL_GET_STATE:
        spin_lock_irqsave(&sim_lock, flags);
        if (copy_to_user((struct thermal_state __user *)arg, &sim_state, sizeof(struct thermal_state))) {
            spin_unlock_irqrestore(&sim_lock, flags);
            return -EFAULT;
        }
        spin_unlock_irqrestore(&sim_lock, flags);
        break;

    case THERMAL_SET_TARGET_TEMP:
        if (copy_from_user(&temp_val, (__s32 __user *)arg, sizeof(__s32)))
            return -EFAULT;
        spin_lock_irqsave(&sim_lock, flags);
        sim_state.target_temp = temp_val;
        spin_unlock_irqrestore(&sim_lock, flags);
        break;

    case THERMAL_SET_FAN_SPEED:
        if (copy_from_user(&temp_val, (__s32 __user *)arg, sizeof(__s32)))
            return -EFAULT;
        spin_lock_irqsave(&sim_lock, flags);
        sim_state.fan_speed = temp_val;
        spin_unlock_irqrestore(&sim_lock, flags);
        break;

    case THERMAL_RESET:
        spin_lock_irqsave(&sim_lock, flags);
        sim_state.temperature = 25000;
        sim_state.fan_speed = 1000;
        sim_state.target_temp = 30000;
        sim_state.alarm_active = 0;
        init_sim_registers();
        spin_unlock_irqrestore(&sim_lock, flags);
        break;

    // --- New HAL IOCTLs ---
    case THERMAL_GET_ADC_MV:
        if (copy_from_user(&ch_data, (struct thermal_channel_data __user *)arg, sizeof(struct thermal_channel_data)))
            return -EFAULT;
        
        if (ch_data.channel < MAX_CHANNELS) {
            spin_lock_irqsave(&sim_lock, flags);
            ch_data.value = adc_millivolts[ch_data.channel];
            spin_unlock_irqrestore(&sim_lock, flags);
            if (copy_to_user((struct thermal_channel_data __user *)arg, &ch_data, sizeof(struct thermal_channel_data)))
                return -EFAULT;
        } else {
            return -EINVAL;
        }
        break;

    case THERMAL_SET_PWM_DUTY:
        if (copy_from_user(&ch_data, (struct thermal_channel_data __user *)arg, sizeof(struct thermal_channel_data)))
            return -EFAULT;
        
        if (ch_data.channel < MAX_CHANNELS) {
            spin_lock_irqsave(&sim_lock, flags);
            pwm_duty[ch_data.channel] = ch_data.value > 255 ? 255 : ch_data.value;
            spin_unlock_irqrestore(&sim_lock, flags);
        } else {
            return -EINVAL;
        }
        break;

    case THERMAL_GET_TACH_PULSES:
        if (copy_from_user(&ch_data, (struct thermal_channel_data __user *)arg, sizeof(struct thermal_channel_data)))
            return -EFAULT;

        if (ch_data.channel < MAX_CHANNELS) {
            spin_lock_irqsave(&sim_lock, flags);
            ch_data.value = tach_pulses[ch_data.channel];
            tach_pulses[ch_data.channel] = 0; // Reset after reading
            spin_unlock_irqrestore(&sim_lock, flags);
            if (copy_to_user((struct thermal_channel_data __user *)arg, &ch_data, sizeof(struct thermal_channel_data)))
                return -EFAULT;
        } else {
            return -EINVAL;
        }
        break;

    case THERMAL_SET_SIM_LOAD:
        if (copy_from_user(&ch_data, (struct thermal_channel_data __user *)arg, sizeof(struct thermal_channel_data)))
            return -EFAULT;
        
        if (ch_data.channel < MAX_CHANNELS) {
            spin_lock_irqsave(&sim_lock, flags);
            sim_zone_load[ch_data.channel] = ch_data.value;
            spin_unlock_irqrestore(&sim_lock, flags);
        } else {
            return -EINVAL;
        }
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

    pr_info("thermal_controller: Initializing module\n");

    spin_lock_init(&sim_lock);
    
    // Initialize legacy state
    sim_state.temperature = 25000;
    sim_state.fan_speed = 1000;
    sim_state.target_temp = 30000;
    sim_state.alarm_active = 0;

    init_sim_registers();

    // Setup and start timer
    timer_setup(&sim_timer, sim_timer_callback, 0);
    mod_timer(&sim_timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        del_timer_sync(&sim_timer);
        return ret;
    }
    major_number = MAJOR(dev_num);

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    thermal_class = class_create(CLASS_NAME);
    #else
    thermal_class = class_create(THIS_MODULE, CLASS_NAME);
    #endif
    
    if (IS_ERR(thermal_class)) {
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        del_timer_sync(&sim_timer);
        return PTR_ERR(thermal_class);
    }

    cdev_init(&thermal_cdev, &fops);
    ret = cdev_add(&thermal_cdev, MKDEV(major_number, 0), 1);
    if (ret < 0) {
        class_destroy(thermal_class);
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        del_timer_sync(&sim_timer);
        return ret;
    }

    thermal_device = device_create(thermal_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(thermal_device)) {
        cdev_del(&thermal_cdev);
        class_destroy(thermal_class);
        unregister_chrdev_region(MKDEV(major_number, 0), 1);
        del_timer_sync(&sim_timer);
        return PTR_ERR(thermal_device);
    }

    return 0;
}

static void __exit thermal_exit(void)
{
    del_timer_sync(&sim_timer); // Safely stop timer before destroying data
    cdev_del(&thermal_cdev);
    device_destroy(thermal_class, MKDEV(major_number, 0));
    class_destroy(thermal_class);
    unregister_chrdev_region(MKDEV(major_number, 0), 1);
    pr_info("thermal_controller: Goodbye from the LKM!\n");
}

module_init(thermal_init);
module_exit(thermal_exit);
