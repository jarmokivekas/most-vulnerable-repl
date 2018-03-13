/* printf, fgets, dprintf*/
#include <stdio.h>
/* system() */
#include <stdlib.h>


#define expression_buflen 1024
#define shellcode_buflen  2048


// char exec_this[] = {0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3};

char shellcode_buf[shellcode_buflen];
char expression_buf[expression_buflen];

/* stores in globa expression_buf */
void get_expresssion_snippet(void){
    printf("enter expression > ");
    fgets(expression_buf, expression_buflen, stdin);
    return;
}


void expr_to_shellcode(void){

    FILE *cfile = fopen("build/expression.c", "wb");
    fprintf(cfile, "int expression(void){int retval = %s; return retval;}", expression_buf);
    fclose(cfile);

    system("make shellcode > /dev/null");

    FILE *shellfile = fopen("build/expression.shellcode", "rb");
    // how long is the file?
    fseek(shellfile, 0, SEEK_END);
    long shellsize = ftell(shellfile);
    fseek(shellfile, 0, SEEK_SET);

    fread(shellcode_buf, shellsize, 1, shellfile);
    fclose(shellfile);
    return;
}


int main(void){
    while(1){

        get_expresssion_snippet();
        expr_to_shellcode();

        /* declare pointer to function(void) returning int */
        int (*exec_shellcode)();
        /* cast shellcode buf to function pointer */
        exec_shellcode = (int(*)())shellcode_buf;

        int retval = exec_shellcode();
        printf("%i\n", retval);
    }
    return 0;
}
