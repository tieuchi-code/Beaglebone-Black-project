//this file is to create character device and matched to device tree source
//this ffile use ioctl and control led by get cmd and data from app in user space

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/platform_device.h>

#define GPIO_NUM 28

#define MAX_LEDS  4
struct gpio_desc *leds[MAX_LEDS];

typedef struct {
    int index;   
    int state;   
} led_cmd_t;
led_cmd_t cmd_data;
#define HELO_MAGIC  'H'
#define IOCTL_SET   _IOW(HELO_MAGIC, 1, led_cmd_t)
#define IOCTL_GET   _IOR(HELO_MAGIC, 2, led_cmd_t)

#define HELLO_LED_DTB_PATH "/hello_leds"

struct class* hello_kernel_class;
struct device* test_device;
dev_t cdev;
struct cdev dev;
struct class* hello_class;
struct device* hello_device;
int led_count = 0;
DEFINE_MUTEX(test_lock);

ssize_t hello_read (struct file *fp, char __user *buf, size_t size, loff_t *offset)
{
    printk("[HELLO KERNEL DRIVER]: this file is read\n");
    return 0;
}

ssize_t hello_write (struct file *fp, const char __user *buf, size_t size, loff_t *offset)
{
    printk("[HELLO KERNEL DRIVER]: this file is written\n");
    return 0;
}

long hello_ioctl (struct file *fp, unsigned int cmd, unsigned long data)
{
    printk("[HELLO KERNEL DRIVER]: ioctl called, cmd %d, data 0x%8lx \n", cmd, data);
    if (copy_from_user(&cmd_data, (led_cmd_t __user *)data, sizeof(cmd_data)))
        return -EFAULT;

    if (cmd_data.index >= led_count || cmd_data.index < 0)
        return -EINVAL;

    switch (cmd) {
    case IOCTL_SET:
        gpiod_set_value_cansleep(leds[cmd_data.index], cmd_data.state);
        printk("[HELLO] LED[%d] -> %s\n", cmd_data.index, cmd_data.state ? "ON" : "OFF");
        break;

    case IOCTL_GET: {
        cmd_data.state = gpiod_get_value_cansleep(leds[cmd_data.index]);
        if (copy_to_user((led_cmd_t __user *)data, &cmd_data, sizeof(cmd_data)))
            return -EFAULT;
        break;
    }

    default:
        break;
    }
    return 0;
}

const struct file_operations hello_fop = {
    .owner = THIS_MODULE,
    .read = hello_read,
    .write = hello_write,
    .unlocked_ioctl = hello_ioctl,
};

ssize_t test_show(struct device *dev, struct device_attribute *attr, char *buf) 
{
    mutex_lock(&test_lock);
    sprintf(buf,"led %s\n",(cmd_data.state == 1)? "ON":"OFF");
    printk("[HELLO KERNEL DRIVER]: you just read /sys/class/FrDevTree/device0/test \n");
    mutex_unlock(&test_lock);
    return strlen(buf);
}

ssize_t test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) 
{
    mutex_lock(&test_lock);
    printk("[HELLO KERNEL DRIVER]: you just write [%s] to /sys/class/hello-class/hello-device/test \n", buf);
    if (strstr(buf, "on")) {
        cmd_data.state = 1;
    }
    else if (strstr(buf,"off"))
    {
        cmd_data.state = 1;
    }
    gpiod_set_value_cansleep(leds[cmd_data.index], cmd_data.state);
    mutex_unlock(&test_lock);
    return count;
}

DEVICE_ATTR_RW(test);

static int __init hello_init (void)
{
    struct device_node *np = of_find_node_by_path(HELLO_LED_DTB_PATH);
    if (!np) {
        printk("[HELLO KERNEL DRIVER]: Not found %s \n",HELLO_LED_DTB_PATH);
        return -1;
    }
    led_count = of_gpio_count(np);
    if (led_count <= 0) {
        printk("No GPIO found!\n");
        return -EINVAL;
    }

    if (led_count > MAX_LEDS)
        led_count = MAX_LEDS;

    for (int i = 0; i < led_count && i < MAX_LEDS; i++) {
    leds[i] = gpiod_get_from_of_node(np, "gpios", i,GPIOD_OUT_LOW, "tieuchi");
    if (IS_ERR(leds[i])) {
        printk("[HELLO] Get LED %d failed\n", i);
        leds[i] = NULL;
        continue;
    }
    gpiod_set_value_cansleep(leds[i], 1);  
    }

    printk("[HELLO KERNEL DRIVER]: module loaded\n");

    int rets = alloc_chrdev_region(&cdev, 0, 1, "hello-driver");
    if(rets < 0) {
        printk("[HELLO KERNEL DRIVER]: alloc char dev failed \n");
        return -1;
    }

    cdev_init(&dev, &hello_fop);
    rets = cdev_add(&dev, cdev, 1);
    if (rets < 0) {
        printk("[HELLO KERNEL DRIVER]: char dev add failed \n");
        return -1;
    }
    printk("[HELLO KERNEL DRIVER]: char dev loaded\n");

    hello_class  = class_create(THIS_MODULE, "hello_class");
    hello_device = device_create(hello_class, NULL, cdev, NULL, "hello_character_device");
    int ret = device_create_file(hello_device, &dev_attr_test);
    if (ret != 0) {
        printk("[HELLO KERNEL DRIVER]: create test file failed \n");
        device_destroy(hello_class, cdev);
        class_destroy(hello_class);
    }
    return 0;
}
void __exit hello_exit(void)
{
    printk("[HELLO KERNEL]: module exited\n");
    device_remove_file(hello_device, &dev_attr_test);
    device_destroy(hello_class, cdev);
    class_destroy(hello_class);
    for (int i = 0; i < led_count; i++)
    if (leds[i]) gpiod_put(leds[i]);

}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tieu Chi");
MODULE_DESCRIPTION("Hello kernel demo");

module_init(hello_init);
module_exit(hello_exit);


/*/ {
	model = "TI AM335x BeagleBone Black";
	compatible = "ti,am335x-bone-black", "ti,am335x-bone", "ti,am33xx";

	hello_leds {
		compatible = "hello-character-device";
		gpios = <&gpio1 28 GPIO_ACTIVE_HIGH>,   //LED1
		        <&gpio1 16 GPIO_ACTIVE_HIGH>,   //LED2
		        <&gpio1 17 GPIO_ACTIVE_HIGH>;   //LED3
	};
};
*/


