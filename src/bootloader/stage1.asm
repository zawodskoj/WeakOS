org 7c00h
bits 16

struc DAP
    .size resb 1
    .zero resb 1
    .sectors resw 1
    .offset resw 1
    .segment resw 1
    .lbalow resd 1
    .lbahi resd 1
endstruc

DAP_OFFSET equ 500h
EXTENDED_READ equ 42h
BOOT_DRIVE equ 80h

jmp 0:start ; выпрямляем cs

introMsg db 'ZawodOS Bootloader v2.0', 0dh, 0ah
introMsg_len equ $-introMsg
diskError db 'Disk read error. System halted', 0dh, 0ah
diskError_len equ $-diskError

start:
    xor ax, ax ; выпрямляем сегментные регистры
    mov ds, ax
    mov ss, ax
    
    mov sp, 1000h ; ставим стек
    
    push introMsg_len
    push introMsg ; печатаем приветственное сообщение
    call printMsg

    mov si, DAP_OFFSET ; заполняем DAP
    mov [si + DAP.size], byte 16 ; размер DAP
    mov [si + DAP.zero], byte 0
    mov [si + DAP.sectors], word 62 ; 62 сектора между первым разделом и MBR
    mov [si + DAP.offset], word 8000h ; расположение stage2
    mov [si + DAP.segment], ax ; нулевой сегмент
    mov [si + DAP.lbalow], dword 1 ; второй сектор
    mov [si + DAP.lbahi], dword 0
   
    mov ah, EXTENDED_READ ; EXTENDED READ
    mov dl, BOOT_DRIVE ; загрузочный диск
    int 13h
    jc err
    
    mov ax, 8000h
    jmp ax ; прыгаем в stage2
    
err:
    push diskError ; печатаем сообщение об ошибке
    call printMsg
    
    cli ; умираем (возможно лучше использовать jmp $)
    hlt
    
printMsg:
    mov ah, 3
    mov bx, 7
    int 10h ; получаем позицию курсора
    
    mov ax, 1301h
    mov bp, sp
    mov cx, [bp + 4]
    mov bp, [bp + 2]
    int 10h ; пишем строку
    
    xor ax, ax
    ret
    