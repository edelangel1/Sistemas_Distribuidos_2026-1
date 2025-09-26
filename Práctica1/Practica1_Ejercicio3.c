#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int pid = 0;
    pid = fork();

    /*if (pid == 0) {
        printf("Código a ejecutar en el proceso hijo\n");
        exit(0);
    } else {
        printf("Código a ejecutar en el proceso padre\n");
        exit(0);
    } */

    switch (pid)
    {
    case 0:
        printf("Código a ejecutar en el proceso hijo\n");
        exit(0);
        break;
    default:
        printf("Código a ejecutar en el proceso padre\n");
        exit(0);
    }
}
