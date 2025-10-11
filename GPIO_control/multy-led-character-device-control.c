//this file is to control GPIO by using ioctl to driver in kernel space

#include <stdio.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HELO_MAGIC  'H'
typedef struct {
    int index;
    int state;
} led_cmd_t;

#define IOCTL_SET   _IOW(HELO_MAGIC, 1, led_cmd_t)
#define IOCTL_GET   _IOR(HELO_MAGIC, 2, led_cmd_t)

int main()
{
        

        printf("Start Program: Character \n");
        int res = 0;
        int gpio_fp = open("/dev/hello_character_device", O_RDWR);
        if (gpio_fp == -1)
        {
                printf("Open character device failed\n");
                return -1;
        }
        printf("Open character device OK\n");

        led_cmd_t cmd;
        printf("LED index (0-2): ");
        scanf("%d", &cmd.index);

        printf("LED state (1/0): ");
        scanf("%d", &cmd.state);
        
        res = ioctl(gpio_fp, IOCTL_SET, &cmd);
        printf("Led_state address: %p\n", &cmd);
        if (res < 0)
        {
                printf("GPIO_V2_GET_LINE_IOCTL falied \n");
                goto X;
        }
 
X:      close(gpio_fp);
        printf("Demo Character Finish / led_state = %d\n",cmd.state);
        return res;
}