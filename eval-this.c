#include <stdio.h>

int eval_this(void) {
    int i = 0x0A;
    return i;
}

char exec_this[] = {0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3};


int main(void){

    int (*exec)();
    exec = (int(*)())exec_this;
    int retval = exec();
    printf("%i\n", retval);
    return 0;
}
