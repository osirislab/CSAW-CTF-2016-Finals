equ SC2  0x4
equ OUT  0x200
; argument locations for convenience
equ ARG1  0x40
equ ARG2  0x41


MI ARG1, 0x100
MI ARG2, 0x12
JI div

; a / b, ARG1 = a, ARG2 = b
; on ret, ARG1 = a % b, ARG2 = floor(a / b)
; in C:
;
; num = 0
; while (a > b) {
;    a -= b
;    num++;
;}
;div = num
;mod = a
div:
    MI SC2, 0x0 ; num = 0
div__loop:
    JG ARG2, ARG1, div__end ; if (a > b) {
    SB ARG1, ARG2 ; a -= b
    INC SC2 ; num++
    JI div__loop ; }
div__end:
    MV ARG2, SC2 ; div = num
    ; mod = a (implicit, from loop)
    HF ; TODO: ret
