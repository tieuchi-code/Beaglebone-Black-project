#include <stdio.h>

int main()
{
    printf("start program\n");
    FILE* fp;
    fp = fopen("/sys/class/hello-kernel/device0/test","w");
    if(fp == NULL)
    {
        printf("File open failed\n");
        return -1;
    }

    char state[10];   
    printf("Enter state (on/off): ");
    scanf("%9s", state);   

    fprintf(fp, "%s", state); 
    fclose(fp);

    return 0;
}
