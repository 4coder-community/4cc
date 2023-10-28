/******************************************************************************
*	main.s
*	 by Alex Chadwick
*
*	A sample assembly code implementation of the ok05 operating system, that 
*	flashes out a pattern using the wait routine from ok04.
*
*	main.s contains the main operating system, and IVT code.
******************************************************************************/

/*
* .globl is a directive to our assembler, that tells it to export this symbol
* to the elf file. Convention dictates that the symbol _start is used for the 
* entry point, so this all has the net effect of setting the entry point here.
* Ultimately, this is useless as the elf itself is not used in the final 
* result, and so the entry point really doesn't matter, but it aids clarity,
* allows simulators to run the elf, and also stops us getting a linker warning
* about having no entry point. 
*/
.section .init
.globl _start
_start:

/*
* Branch to the actual main code.
*/
b main

/*
* This command tells the assembler to put this code with the rest.
*/
.section .text

/*
* main is what we shall call our main operating system method. It never 
* returns, and takes no parameters.
* C++ Signature: void main(void)
*/
main:

/*
* Set the stack point to 0x8000.
*/
mov sp,#0x8000

/*
* Use our new SetGpioFunction function to set the function of GPIO port 16 (OK 
* LED) to 001 (binary)
*/
mov r0,#16
mov r1,#1
bl SetGpioFunction

/* NEW
* Load in the pattern to flash and also store our position in the flash
* sequence.
*/
ptrn .req r4
ldr ptrn,=pattern
ldr ptrn,[ptrn]
seq .req r5
mov seq,#0

loop$:

/* NEW
* Use our new SetGpio function to set GPIO 16 base on the current bit in the 
* pattern causing the LED to turn on if the pattern contains 0, and off if it
* contains 1.
*/
mov r0,#16
mov r1,#1
lsl r1,seq
and r1,ptrn
bl SetGpio

/* NEW
* We wait for 0.25s using our wait method.
*/
ldr r0,=250000
bl Wait

/* NEW
* Loop over this process forevermore, incrementing the sequence counter.
* When it reaches 32, its bit pattern becomes 100000, and so anding it with 
* 11111 causes it to return to 0, but has no effect on all patterns less than
* 32.
*/
add seq,#1
and seq,#0b11111
b loop$

/* NEW
* In the data section of the kernel image, store the pattern we wish to flash
* on the LED.
*/
.section .data
.align 2
pattern:
.int 0b11111111101010100010001000101010
