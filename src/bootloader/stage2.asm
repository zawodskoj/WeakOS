org 8000h
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

start:
    xor ax, ax ; ���������� ���������� ��������
    mov es, ax
    mov ds, ax
    mov ss, ax
    
    mov sp, 1000h ; ������ ����

    mov si, DAP_OFFSET ; ��������� DAP
    mov [si + DAP.size], byte 16 ; ������ DAP
    mov [si + DAP.zero], byte 0
    mov [si + DAP.sectors], word 80 ; 64 �� payload
    mov [si + DAP.offset], word 0h ; ������������ stage2
    mov [si + DAP.segment], word 1000h
    mov [si + DAP.lbalow], dword 800h ; �������� �� �������
    mov [si + DAP.lbahi], dword 0
   
    mov ah, EXTENDED_READ ; EXTENDED READ
    mov dl, BOOT_DRIVE ; ����������� ����
    int 13h
    jc err
    
    mov si, DAP_OFFSET ; ��������� DAP
    mov [si + DAP.size], byte 16 ; ������ DAP
    mov [si + DAP.zero], byte 0
    mov [si + DAP.sectors], word 80 ; 64 �� payload
    mov [si + DAP.offset], word 0h ; ������������ stage2
    mov [si + DAP.segment], word 2000h
    mov [si + DAP.lbalow], dword 1000h ; �������� �� �������
    mov [si + DAP.lbahi], dword 0
   
    mov ah, EXTENDED_READ ; EXTENDED READ
    mov dl, BOOT_DRIVE ; ����������� ����
    int 13h
    jc err
    
    mov eax, 13h ; �������� ���������� 13h (��� ������, ������� �� VESA)
    ; int 10h
    
    mov edi, 0a020h ; �������� ����� ������
    xor ebx, ebx
    mov edx, 534d4150h
    mov [0a000h], dword 0
    mem_map:
        mov eax, 0e820h
        mov ecx, 24
        int 15h
        jc mem_map_end
        add [0a000h], dword 1
        test ebx, ebx
        jz mem_map_end
        add edi, 20h
        jmp mem_map
                
    mem_map_end:
    
    ; ��������� � PM
    cli ; �������� ����������
    in al, 70h
    or al, 80h
    out 70h, al
    lgdt [gdtr] ; ������ gdt
    mov eax, cr0
    or al, 1
    mov cr0, eax ; ��������� � PM
    
    mov ax, 10h ; ����������� ��������
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov esp, 8000h
    jmp 08h:start32
    
bits 32 
start32:
    mov edi, 1000h ; ������� ��������� ��� paging
    mov ecx, 1000h ; 16��
    mov eax, 0
    rep stosd 
    
    mov [1000h], dword 87h ; �������� paging, �� �� � �����
    mov [1004h], dword 0c00087h ; ������ ������� �����
    mov [1008h], dword 1000087h
    mov [1c00h], dword 400087h  ; ���� ������� ����
    mov [1ffch], dword 800087h ; ���� ������� ����
    
    mov eax, cr4
    bts eax, 4
    mov cr4, eax
    mov eax, 1000h ; �������� paging
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    
    mov esi, 10000h ; ��������� elf
    movzx ecx, word [esi + 44] ; ���-�� ������
    mov esi, [esi + 28]
    add esi, 10000h
    load_elf: ; ������ ������ � ������
        mov eax, [esi]
        cmp eax, 1
        jne skip_section
        
        push ecx
        push esi
        mov ecx, [esi + 20]
        mov edi, [esi + 8]
        xor eax, eax
        rep stosb ; �������� �������
        mov ecx, [esi + 16]
        mov edi, [esi + 8]
        mov esi, [esi + 4]
        add esi, 10000h
        rep movsb ; �������� ������
        pop esi
        pop ecx
        
        skip_section:
        add esi, 32
        loop load_elf
    
    mov esp, 0fffffff0h
    jmp [10000h + 24] ; ������� � payload
    
err:    
    cli ; ������� (�������� ����� ������������ jmp $)
    hlt
    
gdtr:
    dw gdtEnd - gdt - 1
    dd gdt

gdt: ; flat-������
    dd 0, 0

    dw 0ffffh, 0h
    db 0h, 10011110b, 11001111b, 0h ; ������� ���� (ring0)

    dw 0ffffh, 0h
    db 0h, 10010010b, 11001111b, 0h ; ������� ������ (ring0)
    
    dw 0ffffh, 0h
    db 0h, 11111110b, 11001111b, 0h ; ������� ���� (ring3)

    dw 0ffffh, 0h
    db 0h, 11110010b, 11001111b, 0h ; ������� ������ (ring3)

gdtEnd:
