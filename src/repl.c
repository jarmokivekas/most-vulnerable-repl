    /* printf, fgets, dprintf*/
#include <stdio.h>
/* system() */
#include <stdlib.h>


#define expression_buflen 1024
#define shellcode_buflen  2048


// char exec_this[] = {0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3};


/* stores in globa expression_buf */
void get_expresssion(char * expression_buf){
    printf("enter expression > ");
    fgets(expression_buf, expression_buflen, stdin);
    return;
}

void call_this(void){
    FILE *logfile = fopen("build/log.txt", "ab");
    fprintf(logfile, "you called it!");
    return;
}

unsigned long address_of_call_this(void){
    return (unsigned long) &call_this;
}

void write_linker_table(FILE *cfile){

    fprintf(cfile, "void (*call_this_from_linker)(void) = (void (*)(void ))%#lx;\n", address_of_call_this());
    // fprintf(cfile, "void (*call_this_from_linker)(void) = (void (*)(void ))%#lx;\n", (unsigned long) &call_this);
    return;
}

void expression_to_shellcode(char *expression_buf, char *shellcode_buf){

    FILE *cfile = fopen("build/expression.c", "wb");

    write_linker_table(cfile);
    fprintf(cfile,

    "#include <signal.h>\n\
    int expression(void){\n\
        __asm__(\"nop; \");\n\
        raise(SIGINT);\n\
        __asm__(\"nop; \");\n\
        int retval = %s;\n\
        __asm__(\"nop; \");\n\
        raise(SIGINT);\n\
        return retval;\n}", expression_buf);
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
    __asm__("nop;");
    char shellcode_buf[shellcode_buflen];
    char expression_buf[expression_buflen];

    while(1){

        get_expresssion(expression_buf);

        expression_to_shellcode(expression_buf, shellcode_buf);
        __asm__("nop;");
        __asm__("nop;");
        /* declare pointer to function(void) returning int */
        int (*exec_shellcode)();
        /* cast shellcode buf to function pointer */
        exec_shellcode = (int(*)())shellcode_buf;
        __asm__("nop;");
        __asm__("nop;");
        int retval = exec_shellcode();
        __asm__("nop;");
        __asm__("nop;");
        printf("%i\n", retval);
    }
    return 0;
}
