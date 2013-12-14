; hello-os
 
        ORG     0x7c00
 
        JMP SHORT entry
        DB      0x90
        DB      "HELLOIPL"      ; Boot sector name (8 bytes)
        DW      512             ; Size of sector (Must be 512)
        DB      1               ; Size of cluster (Must be 1)
        DW      1               ; FAT start
        DB      2               ; FAT count
        DW      224             ; Size of root dir
        DW      2880            ; Size of this media (Must be 2880)
        DB      0xf0            ; Media type (Must be 0xf0)
        DW      9               ; FAT length
        DW      18              ; Number of sector in a track
        DW      2               ; Number of head
        DD      0               ; No partition
        DD      2880            ; Drive size
        DB      0,0,0x29        ; Magic
        DD      0xffffffff      ; Volume serial number
        DB      "HELLO-OS   "   ; Disk name (11 bytes)
        DB      "FAT12   "      ; Format name (8 bytes)
        TIMES 18 DB 0
 
entry:
        MOV     AX,0
        MOV     SS, AX
        MOV     SP, 0x7c00
        MOV     DS, AX
        MOV     ES, AX
 
        MOV     SI, msg
putloop:
        MOV     AL, [SI]
        ADD     SI, 1
        CMP     AL, 0
        JE SHORT fin
        MOV     AH, 0x0e        ; Function: put 1 char
        MOV     BX, 15          ; Color code
        INT     0x10
        JMP SHORT putloop
fin:
        HLT
        JMP SHORT fin
 
msg:
        DB      0x0a, 0x0a
        DB      "hello, world"
        DB      0x0a
        DB      0
 
        TIMES 0x01fe-($-$$) DB 0   ; Fill 0x00 until 510 byte
        DB      0x55, 0xaa      ; Marker
