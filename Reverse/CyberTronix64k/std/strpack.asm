import constants

public strpack
public strunpack

; returns the pointer to the packed data
; dst will be filled to ceil((1/2)(len))
; src will be read to len
;	void strpack(uint16_t* dst, uint16_t* src, size_t len) {
;   size_t dst_len = len / 2;
;		uint16_t* dst_end = dst + dst_len;
;   
;		while (dst != dst_end) {
;     *dst = *src << 8;
;			src++;
;     *dst |= *src & 255;
;     src++;
;     dst++;
;		}
;   if (len % 2 != 0) {
;			*dst = *src << 8;
;   }
; }
