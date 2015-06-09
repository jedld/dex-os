@echo off
REM Set the location of your DJGPP dev environment here
SET DJGPP_LOC=C:\gcc
SET NASM_LOC=C:\gcc\bin

REM clean
del kernel32.bin
del *.o

REM set this to the location of djgpp.env
set DJGPP=%DJGPP_LOC%\djgpp.env

%DJGPP_LOC%\bin\gcc -w -nostdlib -fno-builtin -ffreestanding -c -o kernel32.o kernel32.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o scheduler.o process\scheduler.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o fat.o filesystem\fat12.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o iso9660.o filesystem\iso9660.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o devfs.o filesystem\devfs.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o iomgr.o iomgr\iosched.c

%DJGPP_LOC%\bin\gcc -w  -nostdlib -fno-builtin -ffreestanding -c -o devmgr_error.o devmgr\devmgr_error.c
%NASM_LOC%\nasmw -f coff -o startup.o startup\startup.asm
%NASM_LOC%\nasmw -f coff -o asmlib.o startup\asmlib.asm
%NASM_LOC%\nasmw -f coff -o irqwrap.o irqwrap.asm
%DJGPP_LOC%\bin\ld -T lscript.txt -Map mapfile.txt
REM -Ttext 0x100000 --oformat binary -o kernel32.bin startup.o irqwrap.o asmlib.o kernel32.o scheduler.o
IF EXIST kernel32.bin. (
gzip kernel32.bin
) ELSE (
@echo compile failed.
)
pause