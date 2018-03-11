

- wrap snippet in function call
- compile funtion
- supply machine code into buffer
- `call` into buffer, aka use a function pointer in C



Evaluation kit DEMO9S12XEP100


- use a vector interrupt table -like structure
- use an array of function pointers as liker table

- AVR is harvard architechture, which complicates things. Really the architechture should be Von Neumann, where the data store and program data store use the same bus, and the data is not execution-protected, in order to allow for self-modifying execution of code.




The basic idea is to be able to inject code written in C into the execution flow during runtime. Instead of using a serial terminal prompt that issues commands that are interpreted, and result in specific subroutines to be called, the device receives executable machine code.

For throughout this write-up the word *"shellcode"* will be used to describe any blob of data that can be executed as valid machine code it it is placed in an appropriate location in memory. The word is typically used to describe malicious code used in exploitation of vulnerable software.

In order to make it user-friendly, the commands can be written in C, after which the snippet is complied into a relocatable blob of executable machine code.

------

# Proto 1


There is a simple test function;
```c
int eval_this(void) {
    int i = 0x0A;
    return i;
}
```


The function was complied with `gcc -c eval_this.c`, and then the machine code was manually extracted in hexadecimal representation form the output of `objdump -d`. The test program is listed below. The `char` array is casted to a function pointer, which is then called like a normal function. On success the program should print the returned integer and exit.



```c
#include <stdio.h>

char exec_this[] = {0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3};

int main(void){

    /*declare exec as pointer to function(void) returning int*/
    int (*exec)(void);
    /*cast exec_this into pointer to function(void) returning int*/
    exec = (int(*)(void ))exec_this;
    int retval = exec();
    printf("%i\n", retval);
    return 0;
}
```


Since the machine code is stored as data, it is placed on the stack. This area of memory is protected by a NX bit (no execution bit). Therefore, the program is shut down with a segmentation fault:


```
Program received signal SIGSEGV, Segmentation fault.
0x0000555555755010 in exec_this ()
```

An examination of the address in gdb shows the segmentation fault happens at the first instruction of `exec_this` (the char array with the shellcode)

```
> gdb a.out
(gdb) disassemble 0x0000555555755010
Dump of assembler code for function exec_this:
=> 0x0000555555755010 <+0>:    push   %rbp
   0x0000555555755011 <+1>:    mov    %rsp,%rbp
   0x0000555555755014 <+4>:    movl   $0xa,-0x4(%rbp)
   0x000055555575501b <+11>:   mov    -0x4(%rbp),%eax
   0x000055555575501e <+14>:   pop    %rbp
   0x000055555575501f <+15>:   retq
```

The first instruction, `push %rbp` and `mov %rsp, %rbp` and are the function prologue, added automatically by the complier. For the function prologue to cause a segmentation fault things need to be pretty badly wrong. This is was a good sign that there is some execution protection mechanism causing the segmentation fault.

For program to work as intended, it is required to turn off execution prevention. The `char` array where she shellcode is stored is located on the stack. In GCC, stack execution prevention can be turned off by supplying `-z execstack` on the command line. The `-g` if for including debug symbols, which makes using gdb more convenient in some cases:

```bash
gcc -z execstack -g eval_this.c
```

Stack execution preventions `(-z execstack)` should must not be confused with stack protector canaries that can be turned off using `-fno-stack-protector`. Canaries protect from buffer overflows but in the case of this prototype, there are is no stack overflow being exploited. A stack overflow would typically be the mechanism that is used to put the shellcode into memory in the first place.

----------------------

--f-set-stack-execution on minGW

what is --converage

-z execstack

-----
