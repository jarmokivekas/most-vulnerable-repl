#define SHELLCODE_C_FILE_CONTENTS "\
void call_this(void); \n\
int expression(void){           \n\
    __asm__(\"int $3\");        \n\
    __asm__(\"nop; \");         \n\
    call_this();                \n\
    __asm__(\"nop; \");         \n\
    int retval = %s;            \n\
    __asm__(\"nop; \");         \n\
    return retval;              \n\
}"
