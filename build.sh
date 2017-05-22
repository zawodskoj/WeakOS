#!/bin/bash
#

CFLAGS="-m32 -mno-sse -g"
CXXFLAGS="${CFLAGS} -std=c++14 -c -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter"

set -eu

# подчищаем за предыдущей сборкой
rm -rf bin obj
mkdir bin obj obj/kernel obj/tools bin/tools obj/libc_userspace obj/abi obj/liballoc obj/lib bin/lib

# собираем bootloader
yasm src/bootloader/stage1.asm -I src/bootloader/ -o bin/stage1
yasm src/bootloader/stage2.asm -I src/bootloader/ -o bin/stage2

# собираем abi
cd obj/abi
clang++ ${CXXFLAGS} ../../src/abi/cpp/*.cpp ../../src/abi/cpp/*.s -I ../../src/include
cd ../..
ld -m elf_i386 -o bin/abi.o -r obj/abi/*.o

# собираем библиотеки
for libname in `find src/lib/* -type d -printf "%f\n"`; do
    mkdir bin/lib/${libname} obj/lib/${libname}
    cd obj/lib/${libname}
    clang++ ${CXXFLAGS} ../../../src/lib/${libname}/*.cpp -I ../../../src/include/
    cd ../../..
    ld -m elf_i386 -o bin/lib/${libname}.o -r obj/lib/${libname}/*.o
done

# собираем ядро
cd obj/kernel
clang++ ${CXXFLAGS} ../../src/*.cpp -I ../../src/include/ -nostdinc++\
    -I /home/zawodskoj/Projects/libc++/build/include/c++/v1
cd ../..
#ld -m elf_i386 -o bin/kernel \
#    obj/abi/crti.o\
#    `clang -m32 -print-file-name=crtbegin.o`\
#    obj/kernel/*.o\
#    obj/abi/cxa.o obj/abi/new.o obj/abi/integer.o\
#    bin/lib/*.o\
#    `clang -m32 -print-file-name=crtend.o`\
#    obj/abi/crtn.o\
#    -T src/kernel.ld \
#    -nostdlib -L/usr/lib/gcc/i686-linux-gnu/5 -lgcc
ld -m elf_i386 -o bin/kernel \
    obj/kernel/*.o\
    obj/abi/crti.o\
    `clang -m32 -print-file-name=crtbegin.o`\
    obj/abi/cxa.o obj/abi/new.o obj/abi/integer.o\
    bin/lib/*.o\
    `clang -m32 -print-file-name=crtend.o`\
    obj/abi/crtn.o\
    -rpath /home/zawodskoj/Projects/libc++/build/lib/ \
    -L/usr/lib/gcc/i686-linux-gnu/5 \
    -L/usr/lib32 \
    -L /home/zawodskoj/Projects/libc++/build/lib/ \
    -Bstatic \
    -lc++ -lc++abi -lm -lc -lgcc \
    -T src/kernel.ld \
    -nostdlib \
    -e main

# strip bin/kernel

# собираем стдлибу

cd obj/libc_userspace
clang++ ${CXXFLAGS} ../../src/libc_userspace/*.cpp -I ../../src/libc_userspace/include
cd ../..

#собираем тулзы
for toolName in `find src/tools/* -type d -printf "%f\n"`; do
    mkdir obj/tools/${toolName}
    cd obj/tools/${toolName}
    clang ../../../src/tools/${toolName}/*.c --std=c11 -m32 -mno-sse -c -I ../../../src/include/
    cd ../../..
#    ld -m elf_i386 -o bin/tools/${toolName} obj/tools/${toolName}/*.o -e main -T src/tools/tool.ld
    ld -m elf_i386 -o bin/tools/${toolName} obj/libc_userspace/*.o obj/tools/${toolName}/*.o -e main -T src/tools/tool.ld
#    ld -m elf_i386 -o bin/tools/${toolName} obj/user/*.o obj/tools/${toolName}/*.o -e main -T src/tools/tool.ld
done
