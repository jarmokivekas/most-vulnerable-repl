
#include "repl.h"

// char exec_this[] = {0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3};


/* stores in globa expression_buf */
void get_expresssion(char * expression_buf){
    printf("enter expression > ");
    fgets(expression_buf, expression_buflen, stdin);
    return;
}

void callback_2(uint8_t val){
    printf("-------- callback with value %i------- \n", val);
    return;
}

void call_this(void){
    // FILE *logfile = fopen("./build/log.txt", "a");
    printf("##############callback executed!");
    return;
}

unsigned long address_of_call_this(void){
    return (unsigned long) &call_this;
}

void write_linker_table(FILE *linker_file){
    fprintf(linker_file, "{\n" );          //begin json object
    fprintf(linker_file, "%s\n", "functions: [\n" );
    fprintf(linker_file, "    \"%s\"    :  \"%#lx\",\n"    , "call_this", (unsigned long) &call_this);
    fprintf(linker_file, "    \"%s\"    :  \"%#lx\",\n"    , "call_this", (unsigned long) &call_this);
    fprintf(linker_file, "%s\n", "]" );
    // fprintf(cfile, "void (*call_this_from_linker)(void) = (void (*)(void ))%#lx;\n", (unsigned long) &call_this);
    return;
}

void expression_to_shellcode(char *expression_buf, char *shellcode_buf){

    // FILE *linker_file = fopen("build/linker-table.json", "wb");
    // write_linker_table(linker_file);
    // fclose(linker_file);

    FILE *shellcode_source = fopen("build/expression.c", "wb");
    fprintf(shellcode_source, SHELLCODE_C_FILE_CONTENTS, expression_buf);
    fclose(shellcode_source);

    // compile the shellcode source into machine code
    system("make shellcode > /dev/null");

    FILE *shellfile = fopen("build/expression.shellcode", "rb");
    // how long is the file?
    fseek(shellfile, 0, SEEK_END);
    long shellsize = ftell(shellfile);
    fseek(shellfile, 0, SEEK_SET);
    // read binary shellcode into buffer
    fread(shellcode_buf, shellsize, 1, shellfile);
    fclose(shellfile);
    return;
}


int main(void){
    char expression_buf[expression_buflen];

    while(1){
        get_expresssion(expression_buf);

        expression_to_shellcode(expression_buf, shellcode_buf);
        /* declare pointer to function(void) returning int */
        int (*exec_shellcode)();
        /* cast shellcode buf to function pointer */
        exec_shellcode = (int(*)())shellcode_buf;
        __asm__("int $3");
        int retval = exec_shellcode();
        printf("%i\n", retval);
    }
    return 0;
}
