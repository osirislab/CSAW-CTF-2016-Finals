ji start

import std.constants
import std.getline
import std.println
import std.printhex
import std.base64
import std.memeq

import user
import cksum
import swizzle


start:
	call get_input
	hf

get_input:	
	mi s00, password_entry_s
	mi s01, password_entry_s_len
	call print
	mi s00, input_string
	mi s01, 256
	call getline
	st s01, input_string_len_p
	mi sc0, 78 ; max length of encoded data
	jq sc0, s01, good_len
err_bad_len:
	mi s00, bad_len_s
	mi s01, bad_len_s_len
	call println
	hf
good_len:
	mi s00, password
	mi s01, input_string
	mi s02, 26
    call base64dec
    ; unswizzle
    mi s00, user
    call swizzle
    call validate_cksum
    ; check valid idxes
    mi sc0, 0b10000
    jl sc0, user__index, err_bad_index
good_index:
	; jump to index entry in jumptable
    mi s00, jump_table
    ad s00, user__index
    md ip, s00
	ret
err_bad_index:
    mi s00, bad_idx_s
    mi s01, bad_idx_s_len
    call println
    hf
    

; returns if user cksum and password cksum match
validate_cksum:
    mi s00, user
    mi s01, 25
    call cksum
    jq s00, password__cksum, good_cksum
    mi s00, bad_cksum_s
    mi s01, bad_cksum_s_len
    call println
    hf
good_cksum:
    ret

password_entry_s:
data "PASSWORD: "

equ password_entry_s_len 10

bad_len_s:
data "BAD LENGTH, TERMINATING"

equ bad_len_s_len 23

bad_cksum_s:
data "BAD CKSUM, TERMINATING"

equ bad_cksum_s_len 22

bad_idx_s:
data "BAD INDEX, TERMINATING"

equ bad_idx_s_len 22

input_string:
data rep 256 0

input_string_len:
data 0

; pointer for stores
input_string_len_p:
data input_string_len

; jump table, length 15
jump_table:
entry_0:  data jt0
entry_1:  data jt1
entry_2:  data jt2
entry_3:  data jt3
entry_4:  data jt4
entry_5:  data jt5
entry_6:  data jt6
entry_7:  data jt7
entry_8:  data jt8
entry_9:  data jt9
entry_10: data jt10
entry_11: data jt11
entry_12: data jt12
entry_13: data jt13
entry_14: data jt14

; a struct password (importantly, directly after jumptable)
password:
password__cksum: data 0
; password.data
user:
user__name: data rep 16 0
user__secret: data rep 8 0
user__index: data 0

; -- Jump targets
jt_fail:
    mi s00, fail_string
    mi s01, fail_len
    call println
    hf

jt0:
    mi s00, target1_string
    mi s01, target1_len
    call println
    hf
jt1:
    mi s00, target1_string
    mi s01, target1_len
    call println
    hf
jt2:
    mi s00, target2_string
    mi s01, target2_len
    call println
    hf
jt3:
    mi s00, user__name
    mi s01, target3_user_len
    mi s02, target3_user
    mi s03, target3_user_len
    call memeq
    jq s00, r02, jt_fail

    mi s00, user__secret
    mi s01, target3_pw_len
    mi s02, target3_pw
    mi s03, target3_pw_len
    call memeq
    jq s00, r02, jt_fail

    mi s00, target3_string
    mi s01, target3_len
    call println
    hf

jt4:
    mi s00, target4_string
    mi s01, target4_len
    call println
    hf
jt5:
    mi s00, target5_string
    mi s01, target5_len
    call println
    hf
jt6:
    mi s00, target6_string
    mi s01, target6_len
    call println
    hf
jt7:
    mi s00, target7_string
    mi s01, target7_len
    call println
    hf
jt8:
    mi s00, target8_string
    mi s01, target8_len
    call println
    hf
jt9:
    mi s00, target9_string
    mi s01, target9_len
    call println
    hf
jt10:
    mi s00, target10_string
    mi s01, target10_len
    call println
    hf
jt11:
    mi s00, target11_string
    mi s01, target11_len
    call println
    hf
jt12:
    mi s00, user__name
    mi s01, target12_user_len
    mi s02, target12_user
    mi s03, target12_user_len
    call memeq
    jq s00, r02, jt_fail

    mi s00, user__secret
    mi s01, target12_pw_len
    mi s02, target12_pw
    mi s03, target12_pw_len
    call memeq
    jq s00, r02, jt_fail

    mi s00, flag1
    mi s01, flag1_len
    call println
    hf
jt13:
    mi s00, target13_string
    mi s01, target13_len
    call println
    hf
jt14:
    mi s00, target14_string
    mi s01, target14_len
    call println
    hf




import flags

fail_string: data "That username and password are not recognized."
equ fail_len, 46

target0_string: data "Welcome to the CYBERTRONIX64K. After 40 years in my closet, I hope it was worth the wait."
equ target0_len, 89

target1_string: data "A C program is like a fast dance on a newly waxed dance floor by people carrying razors."
equ target1_len, 88
target2_string: data "FORTRAN is not a flower but a weed - it is hardy, occasionally blooms, and grows in every computer."
equ target2_len, 99

target3_string: data "We hope you enjoy this problem!"
equ target3_len, 31
target3_user: data "ubsan and Hyper"
equ target3_user_len, 15
target3_pw: data "hopulike"
equ target3_pw_len, 8

target4_string: data "C++: an octopus made by nailing extra legs onto a dog."
equ target4_len, 54

target5_string: data "Perl - The only language that looks the same before and after RSA encryption."
equ target5_len, 77
target6_string: data "I had a problem, so I thought I'd use Rust. Now I have &'a &'b mut Problem that I cannot move out of a borrowed context."
equ target6_len, 120
target7_string: data "Go is not a good language. It's not bad; it's just not good."
equ target7_len, 60
target8_string: data "If Java had true garbage collection, most programs would delete themselves upon execution."
equ target8_len, 90
target9_string: data "PHP is built to keep chugging along at all costs. When faced with either doing something nonsensical or aborting with an error, it will do something nonsensical."
equ target9_len, 161 
target10_string: data "Lisp is not an acceptable LISP. Not for any value of Lisp."
equ target10_len, 58
target11_string: data "JavaScript is built on some very good ideas and a few very bad ones."
equ target11_len, 68

target12_user: data "President Skroob"
equ target12_user_len, 16
target12_pw: data "12345"
equ target12_pw_len, 5

target13_string: data "The use of COBOL cripples the mind; its teaching should therefore be regarded as a criminal offense."
equ target13_len, 100
target14_string: data "Python's a drop-in replacement for BASIC in the sense that Optimus Prime is a drop-in replacement for a truck."
equ target14_len, 110
