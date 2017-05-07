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
# пока не надо

# заливаем загрузчик
sudo dd if=bin/stage1 of=/dev/loop1 bs=446 count=1 conv=notrunc > /dev/null
sudo dd if=bin/stage2 of=/dev/loop1 bs=512 seek=1 count=2047 conv=notrunc > /dev/null

# заливаем ядро и payload
sudo dd if=bin/kernel of=/dev/loop1p1 bs=4k conv=notrunc > /dev/null
# dd if=bin/payload of=/dev/loop1p2 bs=4k conv=notrunc

# размонтируем
sync
sudo losetup -d /dev/loop1
