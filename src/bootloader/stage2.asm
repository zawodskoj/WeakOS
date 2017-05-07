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
    mov esp, 8000h
    jmp 08h:start32
    
bits 32 
start32:
    mov edi, 1000h ; ������� ��������� ��� paging
    mov ecx, 1000h ; 16��
    mov eax, 0
    rep stosd 
    
    mov [1000h], dword 87h ; ������ ������� �� ������ 2000h (P|RW|U)
    mov [1004h], dword 0c00087h ; ������ ������� �� ������ 2000h (P|RW|U)
    mov [1008h], dword 800087h ; ������ ������� �� ������ 2000h (P|RW|U)
    mov [1c00h], dword 400087h ; ������ ������� �� ������ 2000h (P|RW|U)
    
;   mov ecx, 100h ; identity paging
;   mov eax, 2000h
;   xor ebx, ebx
;   ident_paging:
;       mov edx, ebx
;       or edx, 7 ; (P|RW|U)
;       mov [eax], edx
;       add eax, 4
;       add ebx, 1000h
;       loop ident_paging
;   
;   mov ecx, 100h ; identity paging
;   mov eax, 4000h
;   mov ebx, 400000h
;   ident_paging2:
;       mov edx, ebx
;       or edx, 7 ; (P|RW|U)
;       mov [eax], edx
;       add eax, 4
;       add ebx, 1000h
;       loop ident_paging2
;       
;   mov ecx, 100h ; identity paging
;   mov eax, 5000h
;   mov ebx, 800000h
;   ident_paging3:
;       mov edx, ebx
;       or edx, 7 ; (P|RW|U)
;       mov [eax], edx
;       add eax, 4
;       add ebx, 1000h
;       loop ident_paging3
;       
;   mov ecx, 100h ; �������� 0c0000000h ��� payload
;   mov eax, 3000h
;   mov ebx, 0c00000h ; ������� �� ���������� ���������
;   payload_paging:
;       mov edx, ebx
;       or edx, 7 ; (P|RW|U)
;       mov [eax], edx
;       add eax, 4
;       add ebx, 1000h
;    loop payload_paging
        
    mov eax, cr4
    bts eax, 4
    mov cr4, eax
    mov eax, 1000h ; �������� paging
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    
	mov ecx, 68h
	xor eax, eax
	mov edi, 0b000h
	rep stosb
	
	mov [0b004h], dword 0x8000
	mov [0b008h], dword 10h
	mov ax, 28h
	ltr ax

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
    
	; mov esp, 1000000h
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

    dw 67h, 0b000h
    db 0h, 10001001b, 00000000b, 0h ; TSS
	
	dd 0, 0

gdtEnd:
