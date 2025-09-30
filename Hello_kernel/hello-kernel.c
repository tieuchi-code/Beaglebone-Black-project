#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/string.h>

#define GPIO_NUM 28

struct class* hello_kernel_class;
struct device* test_device;

DEFINE_MUTEX(test_lock);

typedef enum {
    LED_OFF, LED_ON
} led_state_t;

led_state_t led_state = LED_OFF;

ssize_t test_show(struct device *dev, struct device_attribute *attr, char *buf) 
{
    mutex_lock(&test_lock);
    sprintf(buf,"led %s\n",(led_state == LED_ON)? "ON":"OFF");
    printk("[HELLO KERNEL DRIVER]: you just read /sys/class/hello-kernel/device0/test \n");
    mutex_unlock(&test_lock);
    return strlen(buf);
}

ssize_t test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) 
{
    mutex_lock(&test_lock);
    printk("[HELLO KERNEL DRIVER]: you just write [%s] to /sys/class/hello-kernel/device0/test \n", buf);
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

static int __init hello_init(void)
{
    int rets = gpio_request(28, "mygppio");
    gpio_direction_output(28,0);
    gpio_set_value(GPIO_NUM,0);
    if (!rets) {
    printk("[HELLO KERNEL DRIVER]: gpio %d requested successfully\n", GPIO_NUM);
    }
    else if (rets) {
        printk("*[HELLO KERNEL DRIVER]: can not request gpio1 - 28 ");
        return -1;
    }

    printk("[HELLO KERNEL DRIVER]: module loaded\n");
    hello_kernel_class = class_create (THIS_MODULE,"hello-kernel");
    if (IS_ERR(hello_kernel_class)) {
        printk("[HELLO KERNEL DRIVER]: create hello-kernel class failed\n");
        return -1;
    }

    test_device = device_create(hello_kernel_class,NULL, MKDEV(0,0), NULL, "device0");
    if (IS_ERR(test_device)) {
        printk("[HELLO KERNEL DRIVER]: create device0 failed \n");
        class_destroy(hello_kernel_class);
        return -1;

    }

    int ret = device_create_file(test_device, &dev_attr_test);
    if (ret != 0) {
        printk("[HELLO KERNEL DRIVER]: create test file failed \n");
        device_destroy(hello_kernel_class, MKDEV(0,0));
        class_destroy(hello_kernel_class);
    }
    return 0;

}

static void __exit hello_exit(void)
{
    printk("[HELLO KERNEL]: module exited\n");
    gpio_free(GPIO_NUM);
    device_remove_file(test_device, &dev_attr_test);
    device_destroy(hello_kernel_class, MKDEV(0,0));
    class_destroy(hello_kernel_class);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tieu Chi");
MODULE_DESCRIPTION("Hello kernel demo");
module_init(hello_init);
module_exit(hello_exit);