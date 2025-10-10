#include <stdio.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main()
{
        printf("Start Program: Led control\n");
        int res = 0;
        int gpio_fp = open("/dev/gpiochip0", O_RDWR);
        if (gpio_fp == -1)
        {
                printf("Open gpiochip0 failed");
                return -1;
        }
        struct gpio_v2_line_request gpio_line_req = 
        {
                .offsets = {28},
                .consumer = "LED demo",
                .config = {
                        .flags = GPIO_V2_LINE_FLAG_OUTPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_UP,
                        
                },
                .num_lines = 1,
        };
        
        res = ioctl(gpio_fp, GPIO_V2_GET_LINE_IOCTL, &gpio_line_req);

        if (res < 0)
        {
                printf("GPIO_V2_GET_LINE_IOCTL falied \n");
                goto X;
        }
        struct gpio_v2_line_values line_val =
        {
                .bits = 0b1,
                .mask = 1
        };

        for(int i = 0; i<10 ; i++)
        {
                line_val.bits = ~line_val.bits;
                res = ioctl(gpio_line_req.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &line_val);
                if(res < 0)
                {
                        printf("IOCTL GPIO_V2_LINE_SET_VALUES_IOCTL failed \n");   
                        goto Y;    
                }
        sleep(1);

        }

Y:      close(gpio_line_req.fd);
X:      close(gpio_fp);
        return res;
}