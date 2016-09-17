import constants

public memcpy

; (uint16_t*, size_t) memcpy(uint16_t* dst, uint16_t* src, size_t len) {
;   uint16_t* dst_end = dst + len;
;   while (dst != dst_end) {
;     *dst = *src;
;     dst++;
;     src++;
;   }
;   return dst;
; }

equ __a_memcpy_dst s00
equ __a_memcpy_src s01
equ __a_memcpy_len s02

equ __r_memcpy_ptr s00
equ __r_memcpy_len s01

equ __c_memcpy_1 s03

equ __v_memcpy_dst_end s04
equ __v_memcpy_tmp s05

memcpy:
	mi __c_memcpy_1, 1

	; uint16_t* dst_end = dst + len;
	mv __v_memcpy_dst_end, __a_memcpy_dst
	ad __v_memcpy_dst_end, __a_memcpy_len
	; while (dst != dst_end) {
	__l_memcpy_loop_test:
	jq __a_memcpy_dst, __v_memcpy_dst_end, __l_memcpy_loop_end
		__l_memcpy_loop_start:
			; *dst = *src;
			md __v_memcpy_tmp, __a_memcpy_src
			ld __a_memcpy_dst, __v_memcpy_tmp
			; dst++;
			ad __a_memcpy_dst, __c_memcpy_1
			; src++;
			ad __a_memcpy_src, __c_memcpy_1
	; }
		ji __l_memcpy_loop_test
		__l_memcpy_loop_end:
	; return (dst - len, len);
	sb __r_memcpy_ptr, __a_memcpy_len ; __r_memcpy_len currently has dst_end in it
	mv __r_memcpy_len, __a_memcpy_len
	ret

