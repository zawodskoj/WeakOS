#!/bin/bash
#

# подчищаем за предыдущей сборкой
rm -rf bin obj
mkdir bin obj obj/kernel obj/tools bin/tools obj/user obj/abi obj/liballoc

# собираем bootloader
yasm src/bootloader/stage1.asm -I src/bootloader/ -o bin/stage1
yasm src/bootloader/stage2.asm -I src/bootloader/ -o bin/stage2

# собираем abi
cd obj/abi
clang++ ../../src/abi/cpp/*.cpp --std=c++14 -m32 -mno-sse -c -I ../../src/include\
	-fno-exceptions -fno-rtti
cd ../..

# собираем ядро
cd obj/kernel
clang++ ../../src/*.cpp --std=c++14 -m32 -mno-sse -c -I ../../src/include/ -g\
	-fno-exceptions -fno-rtti # -nostdinc++
cd ../..
ld -m elf_i386 -o bin/kernel obj/abi/*.o obj/kernel/*.o -e k_main -T src/kernel.ld # -lgcc
# strip bin/kernel

# собираем liballoc
cd obj/liballoc
clang++ ../../src/liballoc/*.cpp --std=c++14 -m32 -mno-sse -c -I ../../src/include/\
	-fno-exceptions -fno-rtti # -nostdinc++
cd ../..
# strip bin/kernel

# собираем стдлибу

# cd obj/user
# clang ../../src/user/*.c --std=c11 -m32 -mno-sse -c -I ../../src/include/
# cd ../..

# собираем тулзы
# for toolName in `find src/tools/* -type d -printf "%f\n"`; do
#   mkdir obj/tools/${toolName}
#   cd obj/tools/${toolName}
#   clang ../../../src/tools/${toolName}/*.c --std=c11 -m32 -mno-sse -c -I ../../../src/include/
#   cd ../../..
#   ld -m elf_i386 -o bin/tools/${toolName} obj/user/*.o obj/tools/${toolName}/*.o -e main -T src/tools/tool.ld
# done
