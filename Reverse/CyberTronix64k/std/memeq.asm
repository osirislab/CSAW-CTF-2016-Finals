import constants

public memeq

; bool memeq(uint16_t* buf1, size_t len1, uint16_t* buf2, size_t len2) {
;   if (len1 != len2) {
;     return false;
;   }
;   uint16_t* buf1_end = buf1 + len1;
;   while (buf1 != buf1_end) {
;     if (*buf1 != *buf2) {
;       return false;
;     }
;     buf1++;
;     buf2++;
;   }
;   return true;
; }

equ __a_memeq_buf1 s00
equ __a_memeq_len1 s01
equ __a_memeq_buf2 s02
equ __a_memeq_len2 s03

equ __r_memeq_ret s00

equ __c_memeq_1 s04

equ __v_memeq_buf1_end s05
equ __v_memeq_buf1_tmp s06
equ __v_memeq_buf2_tmp s07

memeq:
  ; if (len1 != len2) {
	jq __a_memeq_len1, __a_memeq_len2, __l_memeq_lengths_same
		; return false;
		mi __r_memeq_ret, 0
		ret
  ; }
	__l_memeq_lengths_same:

	mi __c_memeq_1, 1

  ; uint16_t* buf1_end = buf1 + len1;
	mv __v_memeq_buf1_end, __a_memeq_buf1
	ad __v_memeq_buf1_end, __a_memeq_len1

  ; while (buf1 != buf1_end) {
	__l_memeq_loop_test:
	jq __a_memeq_buf1, __v_memeq_buf1_end, __l_memeq_loop_end
		__l_memeq_loop_start:
			; if (*buf1 != *buf2) {
			md __v_memeq_buf1_tmp, __a_memeq_buf1
			md __v_memeq_buf2_tmp, __a_memeq_buf2
			jq __v_memeq_buf1_tmp, __v_memeq_buf2_tmp, __l_memeq_loop_continue
				; return false;
				mi __r_memeq_ret, 0
				ret
			; }
			__l_memeq_loop_continue:
			; buf1++;
			ad __a_memeq_buf1, __c_memeq_1
			; buf2++;
			ad __a_memeq_buf2, __c_memeq_1
	; }
		ji __l_memeq_loop_test
		__l_memeq_loop_end:
	; if you've gotten here, they compare equal through the entirety
  ; return true;
	mi __r_memeq_ret, 1
	ret
