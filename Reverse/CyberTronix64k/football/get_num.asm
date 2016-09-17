equ OUT 0x200
equ IN  0x201
equ ARG1  0x40

JI get_digit_input

get_digit_input:
    MV ARG1, IN
    ; bounds checks
    MI SC, '0'
    JL ARG1, SC, get_digit_input__err
    MI SC, '9'
    JG ARG1, SC, get_digit_input__err
    SBI ARG1, '0'
    MV OUT, ARG1
    HF ; TODO: ret

get_digit_input__err:
    HF
