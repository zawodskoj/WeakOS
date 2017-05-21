#!/bin/bash
#

# подчищаем за предыдущей сборкой
rm -rf hdd
mkdir hdd

# проверяем stage1
if [ $(wc -c < bin/stage1) -gt 446 ]; then
    echo stage1 is too big
    exit -1
fi

# создаем образ
# fallocate -l 512M hdd/minios.img
touch hdd/minios.img
truncate -s 512M hdd/minios.img

# размечаем
parted hdd/minios.img mklabel msdos > /dev/null
sync
parted hdd/minios.img mkpart primary 2048s 4095s > /dev/null # загрузочный раздел
sync
parted hdd/minios.img mkpart primary 4096s 100% > /dev/null # основной раздел
sync
parted hdd/minios.img set 1 boot on > /dev/null
sync

# монтируем
sudo losetup -P /dev/loop1 hdd/minios.img
sync

# форматируем
sudo mkfs.fat -F32 /dev/loop1p2

# заливаем загрузчик
sudo dd if=bin/stage1 of=/dev/loop1 bs=446 count=1 conv=notrunc > /dev/null
sudo dd if=bin/stage2 of=/dev/loop1 bs=512 seek=1 count=2047 conv=notrunc > /dev/null

# заливаем ядро
sudo dd if=bin/kernel of=/dev/loop1p1 bs=4k conv=notrunc > /dev/null

# временно: заливаем хуйню
mkdir hdd/shit
sudo mount /dev/loop1p2 hdd/shit
sudo cp bin/tools/shell hdd/shit/test.elf
sudo sh -c 'echo sample text > hdd/shit/test.txt'
sudo mkdir hdd/shit/testdir
sudo cp bin/tools/shell hdd/shit/testdir/test.elf
sudo sh -c 'echo sample text 2 > hdd/shit/testdir/test2.txt'
sudo sh -c 'echo sample text 3 > hdd/shit/testdir/other_really_big_file_name.txt'
sudo sh -c 'echo sample text 4 > hdd/shit/really_big_file_name.txt'
sudo umount /dev/loop1p2

# размонтируем
sync
sudo losetup -d /dev/loop1
