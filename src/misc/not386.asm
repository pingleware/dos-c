;;; not386.asm
;;;
;;; This is 16-bit (real, V86 or protected mode) 80x86 code which tells
;;; you whether the processor you're running on supports 32-bit operations.
;;; ('386 or better).  Returns zero if it does, non-zero if it doesn't.
;;; Actually, the return code indicates what test proved that it
;;; doesn't.  If the return value is -2, the push sp test failed.
;;; If the return value is 16, the shift test failed.
;;;
;;; This test is completely of the operating system (if any) running
;;; on the processor.  It uses one word of stack space and preserves
;;; all registers except flags and ax.
;;;
;;; If the last "xor ah,ah" is changed to "mov ah,0", the zero flag is
;;; properly set on return from this function.


;; The following magic incantation is a bit mysterious to me...
_TEXT   segment byte public use16 'CODE'
assume  cs:_TEXT

_not386 proc
public  _not386
;;
;; This first test detects 80x86 for x < 2.  On the 8086 and '186,
;; "push sp" does "--sp; sp[0] = sp".  On all later processors, it does
;; "sp[-1] = sp; --sp".
;;
        push    sp
        pop     ax
        sub     ax,sp
        jne     return

;; This test is the key one.  It will probably detect 8086, V30 and 80186
;; as well as 80286, but I haven't had access to test it on any of those,
;; so it's protected by the well-knwon test above.
;;
;; This test uses the fact that the '386 and above have a barrel shifter
;; to do shifts, while the '286 does left shifts by releated adds.
;; That means that on the '286, the auxilliary carry gets a copy of
;; bit 4 of the shift output, while on the '386 and up, it's trashed
;; (as it happens, set to 1) independent of the result.
;;
;; We do two shifts, which should produce different auxilliary carries
;; on a '286 and XOR them to see if they are different.  Even on a
;; future processor that does something different with the aux carry
;; flag, it probably does something data-independent, so this will still
;; work.  Note that all flags except aux carry are defined for shl
;; output and will be the same for both cases.

        mov     al,4
        shl     al,1    ; Expected to produce ac = 0 on a '286
        lahf
        shl     al,1    ; Expected to produce ac = 1 on a '286
        mov     al,ah
        lahf
        xor     al,ah   ; Xor the flags together to detect the difference
        xor     ah,ah   ; Clear ah
return:
        ret

_not386 endp
_TEXT   ends


