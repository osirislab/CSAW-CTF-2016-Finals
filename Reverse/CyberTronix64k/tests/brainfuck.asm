ji start

; --- System constants ---
equ OUT 0x200
equ IN  0x201
equ SC2 0x4

; --- BF Constants ---
equ CURR_Cmd 0xF0 ; ptr to currently executing cmd
equ DATA_POINTER 0xF1 ; ptr to current data

start:
    mi CURR_Cmd, PROGRAM ; set our current command to PROGRAM
    mi DATA_POINTER, DATA_REGION ; Data pointer to beginning of data region

    dec CURR_Cmd ; back one so inc on first interp loop brings us to beginning
    ji interp

interp:
    inc CURR_Cmd
    ;Debug: print current command
    ;  mi OUT, 0x43
    ;  mi OUT, 0x4d
    ;  mi OUT, 0x44
    ;  mi OUT, 0x3a
    ;  mi OUT, 0x20
    ;  md OUT, CURR_Cmd
	;  mi OUT, 0x0a
    md SC2, CURR_Cmd
    mi SC, 0x3e ; >
    jq SC2, SC, inc_ptr
    mi SC, 0x3c ; <
    jq SC2, SC, dec_ptr
    mi SC, 0x2b ; +
    jq SC2, SC, inc_val
    mi SC, 0x2d ; -
    jq SC2, SC, dec_val
    mi SC, 0x2e ; .
    jq SC2, SC, put_val
    mi SC, 0x2c ; ,
    jq SC2, SC, get_val
    mi SC, 0x5b ; [
    jq SC2, SC, beg_loop
    mi SC, 0x5d ; ]
    jq SC2, SC, end_loop
    mi SC, 0x0 ; NUL terminate str
    jq SC2, SC, exit
    hf ; no matching instruction, fail

inc_ptr:
    inc DATA_POINTER
    ji interp

dec_ptr:
    dec DATA_POINTER
    ji interp

inc_val:
    md SC2, DATA_POINTER
    inc SC2
    ; mi OUT, 0x2d
    ; MV OUT, SC
    ; mi OUT, 0x2d
    st SC2, DATA_POINTER
    ji interp
dec_val:
    md SC2, DATA_POINTER
    dec SC2
    st SC2, DATA_POINTER
    ji interp

put_val:
    md OUT, DATA_POINTER
    ji interp

get_val:
    ld DATA_POINTER, IN
    ji interp
    
beg_loop:
    md SC2, DATA_POINTER
    mi SC, 0x0
    jq SC2, SC, beg_loop__no_enter_loop
    ji beg_loop__enter_loop
beg_loop__no_enter_loop: ; we don't need to start looping -- data is 0
    ; eat until ']'
    md SC2, CURR_Cmd
    mi SC, 0x5d ; ']'
    jq SC, SC2, beg_loop__after
    inc CURR_Cmd
    ji beg_loop__no_enter_loop
beg_loop__after:
    ji interp
beg_loop__enter_loop:
    ; Push cmd ptr and enter loop
    PUSH CURR_Cmd
    ji interp

end_loop:
    ; if *DP != 0, pop CURR_Cmd, else pop SC
    md SC2, DATA_POINTER
    mi SC, 0x0
    jq SC2, SC, end_loop__no_loop
    ; fallthru to end_loop__do_loop
end_loop__do_loop:
    pop CURR_Cmd
    dec CURR_Cmd ; back one
    ji interp
end_loop__no_loop:
    pop SC
    ji interp

exit:
    mi OUT, 0x42
    mi OUT, 0x79
    mi OUT, 0x65
    mi OUT, 0x21
    mi OUT, 0x0a
    hf


; --- BF Program to run ---
PROGRAM:
data "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++\
			++++++++++.>.+++.------.--------.>+.>." 0x0

; currently 256 words
DATA_REGION:
data rep 256 0x0
