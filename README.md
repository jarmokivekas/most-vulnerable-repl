

- wrap snippet in function call
- compile funtion
- supply machine code into buffer
- `call` into buffer, aka use a function pointer in C



Evaluation kit DEMO9S12XEP100


- use a vector interrupt table -like structure
- use an array of function pointers as liker table


- AVR is harvard architechture, which complicates things. Really the architechture should be Von Neumann, where the data store and program data store use the same bus, and the data is not execution-protected, in order to allow for self-modifying execution of code.



# Introduction to proof of concept

This project did not quite end up where it was originally planned, but it went in interesting places none the less. The basic idea was to build something akin to a conventional serial console for a microcontroller, but instead of having the controller run an interpreter for the serial commands, send it machine code instead. One would still write commands into a console, but using C syntax, and the commands would be cross-compiled on the computer before sending the machine code to the controller.

On the controller side, the machine code is read into a buffer and then executed as any other machine code in memory. Once executed, the controller would resume its normal execution flow and/or wait for more commands.

For throughout this write-up the word *"shellcode"* may be used to describe any blob of data that can be executed as valid machine code if it is placed in an appropriate location in memory. The word is typically used to describe malicious code used in exploitation of vulnerable software.


The original idea was to implement this on a microcontroller but, due to circumstance, there were no controller boards readily at hand, so a proof-of-concept was implemented on a Linux laptop running Ubuntu.




# The Proof of Concept

In order to have a clear goal to work toward, the prototype was named *Most vulnerable calculator*. A working prototype should work as a REPL that can be used for doing math using C syntax. Something like this:

```
$ ./repl.out
input expression > 2 * 4
8
```

----

The first task is to figure out how to execute shellcode in a controlled manner for a data buffer. Below is the function used for testing:

```c
int eval_this(void) {
    int i = 0x0A;
    return i;
}
```


This function was compiled with `gcc -c eval_this.c`, and then the machine code was manually extracted in hexadecimal representation form the output of `objdump -d`. A test program is listed below. The `char` array int the test program is cast to a function pointer, which is then called like a normal function. On success, the program should print the returned integer (decimal `10`, hex `0x0A`) and exit.


```c
#include <stdio.h>

char exec_this[] = {
    0x55,0x48,0x89,0xe5,0xc7,0x45,0xfc,0x0a,
    0x00,0x00,0x00,0x8b,0x45,0xfc,0x5d,0xc3
};

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


Since the shellcode is stored in memory as *"data"*, it is placed on the stack. This area of memory is protected by an NX bit (no execution bit). Therefore, the program is shut down with a segmentation fault because of a memory violation:


```
Program received signal SIGSEGV, Segmentation fault.
0x0000555555755010 in exec_this ()
```

An examination of the address in gdb shows the segmentation fault happens at the first instruction of `exec_this` (the char array with the shellcode):

```
$ gdb a.out
(gdb) disassemble 0x0000555555755010
Dump of assembler code for function exec_this:
=> 0x0000555555755010 <+0>:    push   %rbp
   0x0000555555755011 <+1>:    mov    %rsp,%rbp
   0x0000555555755014 <+4>:    movl   $0xa,-0x4(%rbp)
   0x000055555575501b <+11>:   mov    -0x4(%rbp),%eax
   0x000055555575501e <+14>:   pop    %rbp
   0x000055555575501f <+15>:   retq
```

The first instructions, `push %rbp` and `mov %rsp, %rbp`, are the function prologue, added automatically by the compiler. It makes sure that entering into and returning from functions works. For the function prologue to cause a segmentation fault things need to be pretty badly wrong. This is was a good sign that there is some execution protection mechanism causing the segmentation fault.

For program to work as intended, it is required to turn off execution prevention. The `char` array where she shellcode is stored is located on the stack. In GCC, stack execution prevention can be turned off by supplying `-z execstack` on the command line. The `-g` if for including debug symbols, which makes using gdb more convenient in some cases:

```bash
gcc -z execstack -g eval_this.c
```

Stack execution preventions `(-z execstack)` must not be confused with stack protector canaries that can be turned off using `-fno-stack-protector`. The canaries protect continuing execution after a buffer overflow has occurred, but in the case of this prototype, there are is no stack overflow being exploited. A stack overflow would typically be the mechanism that is used to put the shellcode into memory in the first place, in this test the shellcode is put on the stack essentially at compile-time.

# Conclusion

The basic prototype works. The next steps are to figure out a convenient way to do the transformation from a console input string such as `12 + 3  * 4` into a blob of shellcode that can be called as a C function. This shouldn't be too hard. There are specially made tools for this, for example in Radare2. Taking into account the already hacky nature of the project, it's likely the shellcode compiler will consist of GCC and some bash glue.

# Tools

- `gcc` for compilation
- `gdb` for debugging
- `man`, because I can't not read the manual
-  `cdecl`, which is a tool for converting C syntax for declarations and casts into English, and vice-versa.
It's a great tool when you have more complex data structures or have to deal with function pointers or dereferences where the syntax is less-than-intuitive.


Some examples of things that `cdecl` can do:
```
cdecl> explain (int(*)(void ))exec_this
cast exec_this into pointer to function (void) returning int

cdecl> declare foobar as array 10 of pointer to void
void *foobar[10]
```
Cdecl can also be used in a browser at cdecl.org.













--------------


# debugging tricks

```
__asm__("nop;");
__asm__("nop;");
__asm__("nop;");
```

```
(dgb) where
#0  0x0000000000000000 in ?? ()
#1  0x00007fffffffd731 in ?? ()
#2  0x0000555555757a80 in ?? ()
#3  0x000000c800000017 in ?? ()
#4  0x00007fffffffdf30 in ?? ()
#5  0x0000555555554b07 in main () at src/repl.c:80   // this is the call to the casted function pointer aka exec_shellcode():
```


(gdb) x/20i 0x0000555555554b05
   0x555555554b05 <main+99>:	mov    %eax,-0xc1c(%rbp)
   0x555555554b0b <main+105>:	nop
   0x555555554b0c <main+106>:	nop
   0x555555554b0d <main+107>:	mov    -0xc1c(%rbp),%eax
   0x555555554b13 <main+113>:	mov    %eax,%esi
   0x555555554b15 <main+115>:	lea    0x1ad(%rip),%rdi        # 0x555555554cc9
   0x555555554b1c <main+122>:	mov    $0x0,%eax
   0x555555554b21 <main+127>:	callq  0x555555554780 <printf@plt>
   0x555555554b26 <main+132>:	jmp    0x555555554abd <main+27>



```
(gdb) disassemble main
Dump of assembler code for function main:
[...snip...]
   0x0000555555554af6 <+84>:	nop
   0x0000555555554af7 <+85>:	mov    -0xc18(%rbp),%rdx
   0x0000555555554afe <+92>:	mov    $0x0,%eax
   0x0000555555554b03 <+97>:	callq  *%rdx
   0x0000555555554b05 <+99>:	mov    %eax,-0xc1c(%rbp)
   0x0000555555554b0b <+105>:	nop
...
```

The above assembler is the following lines of C:

```
__asm__("nop;");
int retval = exec_shellcode();
__asm__("nop;");
```

The `nop` instructions are there only to make it easy to spot. They make it very obvious were the relevant assembly intructions are in the disassembly output. The complier has no reason to insert `nop` instructions.

`callq *%rdx` means a function call to the address stored in the `rdx` register, the function pointer to `exec_shellcode`.

```
(gdb) info registers
...
rdx            0x7fffffffd720	140737488344864
...
```

(dgb) x/20i 0x7fffffffd720
(gdb) x/20i 0x7fffffffd720
   0x7fffffffd720:	push   %rbp
   0x7fffffffd721:	mov    %rsp,%rbp
   0x7fffffffd724:	sub    $0x10,%rsp
   0x7fffffffd728:	int3   
=> 0x7fffffffd729:	nop
   0x7fffffffd72a:	mov    $0x2,%edi
   0x7fffffffd72f:	nop
   0x7fffffffd730:	movl   $0x2,-0x4(%rbp)
   0x7fffffffd737:	nop
   0x7fffffffd738:	mov    $0x2,%edi
   0x7fffffffd73d:	mov    -0x4(%rbp),%eax
   0x7fffffffd740:	leaveq
   0x7fffffffd741:	retq   









```
nm -g repl.out | egrep '^[0-9a-e]{8,} T' |awk '{printf "PROVIDE(%s = 0x%s);\n",$3,$1}'
```
