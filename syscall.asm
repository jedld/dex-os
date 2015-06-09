[bits 32]

section .text

global syscall
syscall
   call 0xD8:0
ret
