//this file will be build-in to kernel, use module platform driver

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

#define HELO_MAGIC  'H'
#define IOCTL_SET   _IOW(HELO_MAGIC, 1, int)
#define IOCTL_GET   _IOR(HELO_MAGIC, 2, int)

struct class* hello_kernel_class;
struct device* test_device;
dev_t cdev;
struct cdev dev;

struct class* hello_class;
struct device* hello_device;

typedef enum {
    LED_OFF, LED_ON
} led_state_t;

DEFINE_MUTEX(test_lock);

led_state_t led_state = LED_OFF;

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
    switch (cmd)
    {
    case IOCTL_SET:
        led_state = (data == 0)?LED_OFF:LED_ON;
        printk("[HELLO KERNEL DRIVER]: Set led: %s \n", (led_state == LED_OFF)?"OFF":"ON");
        break;
    case IOCTL_GET:
        printk("[HELLO KERNEL DRIVER]: Get led: %s \n", (led_state == LED_OFF)?"OFF":"ON");
        int val = led_state;
        if (copy_to_user((int __user *)data, &val, sizeof(val)))
        return -EFAULT;
        break;
    
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
    sprintf(buf,"led %s\n",(led_state == LED_ON)? "ON":"OFF");
    printk("[HELLO KERNEL DRIVER]: you just read /sys/class/FrDevTree/device0/test \n");
    mutex_unlock(&test_lock);
    return strlen(buf);
}

ssize_t test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) 
{
    mutex_lock(&test_lock);
    printk("[HELLO KERNEL DRIVER]: you just write [%s] to /sys/class/FrDevTree/device0/test \n", buf);
    if (strstr(buf, "on")) {
        led_state = LED_ON;
    }
    else if (strstr(buf,"off"))
    {
        led_state = LED_OFF;
    }
    gpio_set_value(GPIO_NUM,led_state);
    mutex_unlock(&test_lock);
    return count;
}

DEVICE_ATTR_RW(test);

//#define HELLO_LED_DTB_PATH "/hello_led"
struct gpio_desc *g;

int hello_driver_init(struct platform_device * pd)
{
    //struct device_node *np = of_find_node_by_path(HELLO_LED_DTB_PATH);
#if 0
    struct device_node *np = pd->dev.of_node;
    if (!np) {
        printk("[HELLO KERNEL DRIVER]: Not found  \n");
        return -1;
    }

    g = gpiod_get_from_of_node(np,"gpios",0, GPIOD_OUT_LOW, NULL);
    of_node_put(np);
    if (IS_ERR(g)) {
        printk("[HELLO KERNEL DRIVER]: gpiod_get_from_node failed: \n");
        return -1;
    }

    gpiod_set_value_cansleep(g,1);
#endif

    printk("[HELLO KERNEL DRIVER]: module loaded\n");

    int ret = alloc_chrdev_region(&cdev, 0, 1, "hello-driver");
    if(ret < 0) {
        printk("[HELLO KERNEL DRIVER]: alloc char dev failed \n");
        return -1;
    }

    cdev_init(&dev, &hello_fop);
    ret = cdev_add(&dev, cdev, 1);
    if (ret < 0) {
        printk("[HELLO KERNEL DRIVER]:char dev add failed \n");
        return -1;
    }
    printk("[HELLO KERNEL DRIVER]:char dev loaded\n");

    hello_class = class_create(THIS_MODULE, "hello_class");
    hello_device = device_create(hello_class, NULL, cdev, NULL, "hello_character_device");

    return 0;

}
int hello_drvier_exit(struct platform_device * pd)
{
    printk("[HELLO KERNEL]: module exited\n");
    gpio_free(GPIO_NUM);
    device_remove_file(test_device, &dev_attr_test);
    device_destroy(hello_kernel_class, MKDEV(0,0));
    class_destroy(hello_kernel_class);
    return 0;
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tieu Chi");
MODULE_DESCRIPTION("Hello kernel demo");
struct of_device_id hello_led_of_driver[] = {
    {.compatible = "hello-character-device"},
    {}
};
static struct platform_driver hello_led_driver = {
    .probe = hello_driver_init,
    .remove = hello_drvier_exit,
    .driver = {
        .name = "hello-kernel",
        .of_match_table = hello_led_of_driver,
    },

};

module_platform_driver(hello_led_driver);
// module_init(hello_init);
// module_exit(hello_exit);