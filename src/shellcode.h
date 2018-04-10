#define SHELLCODE_C_FILE_CONTENTS "\
void callback(void); \n\
int expression(void){           \n\
    __asm__(\"int $3\");        \n\
    __asm__(\"nop; \");         \n\
    callback();                \n\
    __asm__(\"nop; \");         \n\
    int retval = %s;            \n\
    __asm__(\"nop; \");         \n\
    return retval;              \n\
}"
