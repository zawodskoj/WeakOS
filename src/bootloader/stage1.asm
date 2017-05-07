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

jmp 0:start ; ���������� cs

introMsg db 'ZawodOS Bootloader v2.0', 0dh, 0ah
introMsg_len equ $-introMsg
diskError db 'Disk read error. System halted', 0dh, 0ah
diskError_len equ $-diskError

start:
    xor ax, ax ; ���������� ���������� ��������
    mov ds, ax
    mov ss, ax
    
    mov sp, 1000h ; ������ ����
    
    push introMsg_len
    push introMsg ; �������� �������������� ���������
    call printMsg

    mov si, DAP_OFFSET ; ��������� DAP
    mov [si + DAP.size], byte 16 ; ������ DAP
    mov [si + DAP.zero], byte 0
    mov [si + DAP.sectors], word 62 ; 62 ������� ����� ������ �������� � MBR
    mov [si + DAP.offset], word 8000h ; ������������ stage2
    mov [si + DAP.segment], ax ; ������� �������
    mov [si + DAP.lbalow], dword 1 ; ������ ������
    mov [si + DAP.lbahi], dword 0
   
    mov ah, EXTENDED_READ ; EXTENDED READ
    mov dl, BOOT_DRIVE ; ����������� ����
    int 13h
    jc err
    
    mov ax, 8000h
    jmp ax ; ������� � stage2
    
err:
    push diskError ; �������� ��������� �� ������
    call printMsg
    
    cli ; ������� (�������� ����� ������������ jmp $)
    hlt
    
printMsg:
    mov ah, 3
    mov bx, 7
    int 10h ; �������� ������� �������
    
    mov ax, 1301h
    mov bp, sp
    mov cx, [bp + 4]
    mov bp, [bp + 2]
    int 10h ; ����� ������
    
    xor ax, ax
    ret
    