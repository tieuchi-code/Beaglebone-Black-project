#include <stdio.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HELO_MAGIC  'H'
#define IOCTL_SET   _IOW(HELO_MAGIC, 1, int)
#define IOCTL_GET   _IOR(HELO_MAGIC, 2, int)

int main()
{
        printf("Start Program: Character \n");
        int res = 0;
        int gpio_fp = open("/dev/hello_character_device", O_RDWR);
        if (gpio_fp == -1)
        {
                printf("Open gpiochip0 failed");
                return -1;
        }


        int led_state = 1;
        res = ioctl(gpio_fp, IOCTL_GET, &led_state);
        printf("Led_state address: %p\n", &led_state);
        if (res < 0)
        {
                printf("GPIO_V2_GET_LINE_IOCTL falied \n");
                goto X;
        }


        //sleep(10);
X:      close(gpio_fp);
        printf("Demo Character Finish / led_state = %d\n",led_state);
        return res;
}